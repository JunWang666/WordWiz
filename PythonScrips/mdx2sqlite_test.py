import sqlite3
import sys
from pathlib import Path

from readmdict import MDX  # 这个应该没问题
try:
    from readmdict.readmdict import MdxError # 尝试从子模块导入 MdxError
except ImportError:
    # 如果上面的导入失败，说明 MdxError 可能名字变了或者被移除了
    # 作为临时方案，你可以用一个通用的异常，但最好是找到正确的错误类型
    print("警告：未能从 readmdict.readmdict 导入 MdxError。特定的 MDX 错误可能无法准确捕获。")
    # 如果找不到特定的 MdxError，下面的 except MdxError as e: 部分可能需要调整
    # 为 except Exception as e: 并检查错误内容，或者查找 readmdict 文档看推荐的错误处理方式。
    # 为了让脚本能继续运行，这里定义一个基础的 Exception 作为 MdxError 的替代。
    MdxError = Exception

try:
    from tqdm import tqdm
except ImportError:
    print("提示：tqdm 库未安装。进度条将不可用。可以运行 'pip install tqdm' 来安装它。")


    # 定义一个虚拟的 tqdm，以便在未安装时脚本仍能运行
    def tqdm(iterable, *args, **kwargs):
        return iterable

# --- 全局设置 ---
BATCH_SIZE = 1000  # 每批次插入数据库的词条数量


def get_user_paths():
    """
    通过 input() 获取用户输入的 MDX 文件路径和 SQLite 输出路径。
    """
    while True:
        mdx_file_path_str = input("请输入源 MDX 文件路径: ").strip()
        mdx_file_path = Path(mdx_file_path_str)
        if mdx_file_path.is_file() and mdx_file_path.suffix.lower() == '.mdx':
            break
        else:
            print("错误：MDX 文件路径无效或文件不存在。请确保路径正确且以 .mdx 结尾。")

    while True:
        sqlite_file_path_str = input("请输入目标 SQLite 数据库文件路径 (例如 output.db): ").strip()
        sqlite_file_path = Path(sqlite_file_path_str)
        if sqlite_file_path.suffix.lower() != '.db' and sqlite_file_path.suffix.lower() != '.sqlite':
            # 自动添加 .db 后缀（如果用户没有指定）
            print(
                f"提示：输出文件名 '{sqlite_file_path.name}' 没有标准 SQLite 后缀，将使用 '{sqlite_file_path.name}.db'。")
            sqlite_file_path = sqlite_file_path.with_suffix(".db")

        # 检查父目录是否存在，如果不存在则尝试创建
        try:
            sqlite_file_path.parent.mkdir(parents=True, exist_ok=True)
            break
        except OSError as e:
            print(f"错误：无法创建 SQLite 文件的输出目录 '{sqlite_file_path.parent}': {e}")
        except Exception as e:
            print(f"输入 SQLite 文件路径时发生未知错误: {e}")

    return mdx_file_path, sqlite_file_path


def create_tables(cursor):
    """
    在 SQLite 数据库中创建 'info' 和 'word' 表。
    """
    # 词典信息表
    # AttributeName: 元数据属性名 (如 Title, Description, Encoding)
    # AttributeValue: 元数据属性值
    cursor.execute('''
                   CREATE TABLE IF NOT EXISTS info
                   (
                       AttributeName
                       TEXT
                       PRIMARY
                       KEY,
                       AttributeValue
                       TEXT
                   )
                   ''')
    # 词条表
    # keyword: 词条 (设置为 PRIMARY KEY 以便快速查找，并自动创建索引)
    # definition_html: HTML 格式的释义
    cursor.execute('''
                   CREATE TABLE IF NOT EXISTS word
                   (
                       keyword
                       TEXT
                       PRIMARY
                       KEY,
                       definition_html
                       TEXT
                   )
                   ''')
    print("数据库表 'info' 和 'word' 已创建（如果它们不存在）。")


def populate_info_table(cursor, mdx_header):
    """
    将 MDX 头信息填充到 'info' 表中。
    """
    if not mdx_header:
        print("警告：MDX 头部信息为空。")
        return

    header_data_to_insert = []
    for key_bytes, value_bytes in mdx_header.items():
        try:
            key = key_bytes.decode('utf-8')  # Header keys are usually UTF-8
            value = value_bytes.decode('utf-8')  # Header values are usually UTF-8
            header_data_to_insert.append((key, value))
        except UnicodeDecodeError:
            try:
                # 尝试使用其他常见编码，或者将无法解码的字节串表示出来
                key = key_bytes.decode('latin-1', errors='replace')
                value = value_bytes.decode('latin-1', errors='replace')
                header_data_to_insert.append((key, value))
                print(f"警告：元数据 '{key}' 或其值可能包含非UTF-8字符，已尝试用latin-1解码。")
            except Exception as e:
                print(f"错误：解码元数据时出错 - Key: {key_bytes}, Value: {value_bytes}, Error: {e}")
        except Exception as e:
            print(f"处理元数据时发生未知错误 - Key: {key_bytes}, Value: {value_bytes}, Error: {e}")

    if header_data_to_insert:
        try:
            cursor.executemany("INSERT OR REPLACE INTO info (AttributeName, AttributeValue) VALUES (?, ?)",
                               header_data_to_insert)
            print(f"成功将 {len(header_data_to_insert)} 条元数据写入 'info' 表。")
        except sqlite3.Error as e:
            print(f"错误：写入元数据到 'info' 表时发生 SQLite 错误: {e}")
    else:
        print("没有有效的元数据可写入 'info' 表。")


def populate_word_table(conn, mdx_items_iterator, encoding, total_items):
    """
    将 MDX 词条和释义填充到 'word' 表中。
    包含在一个大的事务中，PRAGMA设置在外部处理。
    """
    cursor = conn.cursor()
    batch = []
    processed_count = 0
    print(f"开始处理词条，预计共有 {total_items if total_items else '未知数量的'} 词条...")
    print(f"词条和释义将使用编码: {encoding}")

    try:
        conn.execute("BEGIN TRANSACTION")  # 在循环开始前显式开始事务

        for key_bytes, definition_bytes in tqdm(mdx_items_iterator, total=total_items, desc="转换词条中", unit="词条"):
            try:
                keyword = key_bytes.decode(encoding)
                definition_html = definition_bytes.decode(encoding)
                batch.append((keyword, definition_html))
            except UnicodeDecodeError as ude:
                keyword = key_bytes.decode(encoding, errors='replace')
                definition_html = definition_bytes.decode(encoding, errors='replace')
                batch.append((keyword, definition_html))
                # print(f"\n警告：词条 '{keyword[:30]}...' 或其释义解码时遇到问题，已使用替换字符处理: {ude}")
            except Exception as e:
                print(f"\n错误：处理词条数据时出错 - Key: {key_bytes[:30]}..., Error: {e}")
                continue

            if len(batch) >= BATCH_SIZE:
                cursor.executemany("INSERT OR REPLACE INTO word (keyword, definition_html) VALUES (?, ?)", batch)
                processed_count += len(batch)
                batch = []

        # 插入剩余的批处理数据
        if batch:
            cursor.executemany("INSERT OR REPLACE INTO word (keyword, definition_html) VALUES (?, ?)", batch)
            processed_count += len(batch)
            batch = []  # 清空 batch

        conn.commit()  # 所有词条处理完毕后，提交整个事务
        print(f"词条处理完成。总共处理并尝试写入 {processed_count} 条词条到 'word' 表。")
        return True

    except MdxError as e:
        print(f"\n错误：读取 MDX 文件内容时发生错误 (可能与压缩或文件格式有关): {e}")
        print("这可能是因为词典使用了LZO等压缩方式，但系统中缺少相应的解码库 (如 python-lzo)。")
        print("请检查您的 MDX 文件和 readmdict 的依赖项。")
        if conn:
            conn.rollback()  # 发生错误，回滚事务
        return False
    except sqlite3.Error as e:
        print(f"\n错误：写入词条到 'word' 表时发生 SQLite 错误: {e}")
        if conn:
            conn.rollback()  # 发生错误，回滚事务
        return False
    except Exception as e:
        print(f"\n错误：填充词条表时发生未知错误: {e}")
        if conn:
            conn.rollback()  # 发生错误，回滚事务
        return False
    # finally 块在这里不再需要提交，因为事务要么成功提交，要么在 except 中回滚


def main():
    """
    主函数，执行 MDX 到 SQLite 的转换过程。
    """
    print("--- MDX 转 SQLite 脚本 ---")
    mdx_file, sqlite_file = get_user_paths()
    print(f"源 MDX 文件: {mdx_file}")
    print(f"目标 SQLite 文件: {sqlite_file}")

    mdx_instance = None
    conn = None

    try:
        print("正在打开和解析 MDX 文件...")
        mdx_instance = MDX(str(mdx_file))
        mdx_header = mdx_instance.header

        # (获取词条总数的代码保持不变)
        num_entries = None
        try:
            # 尝试通过迭代一次 keys 来获取总数，但要注意这会消耗迭代器
            # temp_keys = list(mdx_instance.keys()) # 如果 keys() 返回的是一次性迭代器，后续 items() 可能为空
            # num_entries = len(temp_keys)
            # items_iterator = mdx_instance.items() # 重新获取 items 迭代器
            # 为了安全，还是先设为None，依赖tqdm
            # 或者如果确定 mdx_instance.items() 可以多次调用或者 mdx_instance.keys() 不影响 items()
            # 对于大型词典，遍历两次 keys() 或 items() 会增加时间
            if hasattr(mdx_instance, '_num_entries'):  # 假设的属性
                num_entries = mdx_instance._num_entries
            print(f"MDX 文件元数据加载完毕。")
        except Exception:
            num_entries = None

        items_iterator = mdx_instance.items()  # 获取词条迭代器

        # (确定解码编码的代码保持不变)
        record_encoding = 'UTF-8'
        if mdx_header and b'Encoding' in mdx_header:
            try:
                record_encoding = mdx_header[b'Encoding'].decode('utf-8').strip()
                if not record_encoding:
                    record_encoding = 'UTF-8'
                else:
                    "".encode(record_encoding)
                print(f"从MDX元数据获取到词条编码: {record_encoding}")
            except Exception as e:
                record_encoding = 'UTF-8'
                print(f"警告：无法解析MDX元数据中的 'Encoding' (错误: {e})，将使用默认 UTF-8。")
        else:
            print("未在MDX元数据中找到 'Encoding' 信息，将使用默认 UTF-8 进行解码。")

        conn = sqlite3.connect(sqlite_file)
        cursor = conn.cursor()

        # PRAGMA 设置在所有事务之外
        cursor.execute("PRAGMA synchronous = OFF")
        cursor.execute("PRAGMA journal_mode = MEMORY")

        create_tables(cursor)  # 创建表

        populate_info_table(cursor, mdx_header)  # 填充info表
        conn.commit()  # 提交 info 表的更改 (这是一个单独的事务)

        print("开始填充词条数据...")
        if populate_word_table(conn, items_iterator, record_encoding, num_entries):
            print("词条数据填充成功。")
        else:
            print("词条数据填充过程中发生错误或被中止。")

        print("转换过程完成。")

    except FileNotFoundError:
        print(f"错误：MDX 文件 '{mdx_file}' 未找到。")
    except MdxError as e:
        print(f"错误：处理 MDX 文件时发生错误 (readmdict): {e}")
    except sqlite3.Error as e:
        print(f"错误：操作 SQLite 数据库时发生错误: {e}")
    except Exception as e:
        print(f"发生未知错误: {e}")
    finally:
        if conn:
            try:
                # 恢复默认的 PRAGMA 设置在连接关闭前来做
                # 确保此时没有活动事务
                cursor = conn.cursor()  # 获取一个新的 cursor
                cursor.execute("PRAGMA synchronous = FULL")
                cursor.execute("PRAGMA journal_mode = DELETE")
                conn.commit()  # 提交 PRAGMA 更改
                conn.close()
                print("数据库连接已关闭。")
            except sqlite3.Error as e:
                print(f"关闭数据库连接或恢复PRAGMA时出错: {e}")
        if mdx_instance:
            del mdx_instance



if __name__ == '__main__':
    main()