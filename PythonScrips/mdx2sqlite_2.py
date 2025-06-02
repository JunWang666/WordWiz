import sqlite3
import sys
import re
import base64
import os
from pathlib import Path

try:
    from readmdict import MDX, MDD
except ImportError:
    print("错误：readmdict 库未安装。请使用 'pip install readmdict' 命令安装。")
    sys.exit(1)

MdxError = Exception
try:
    from tqdm import tqdm
except ImportError:
    print("提示：tqdm 库未安装。进度条将不可用。可以运行 'pip install tqdm' 来安装它。")


    def tqdm(iterable, *args, **kwargs):
        return iterable

# --- 全局设置 ---
BATCH_SIZE = 1000


# --- 辅助函数 ---
def normalize_mdd_key(path_str):
    """
    规范化 HTML 中的路径字符串，以匹配 MDD 文件中的键名。
    MDD 键通常以 '\' 开头，并使用 '\' 作为路径分隔符。
    """
    if not path_str:
        return None
    # 移除可能的URL参数和锚点
    path_str = path_str.split('?')[0].split('#')[0]
    # 替换正斜杠为反斜杠
    normalized_path = path_str.replace('/', '\\')
    # 确保以单个反斜杠开头
    if normalized_path.startswith('\\'):
        return normalized_path.encode('utf-8')
    else:
        return (f'\\{normalized_path}').encode('utf-8')


def get_mime_type(filename):
    """根据文件扩展名获取常见的 MIME 类型"""
    ext = os.path.splitext(filename)[1].lower()
    if ext == '.png':
        return 'image/png'
    elif ext in ['.jpg', '.jpeg']:
        return 'image/jpeg'
    elif ext == '.gif':
        return 'image/gif'
    elif ext == '.webp':
        return 'image/webp'
    elif ext == '.css':
        return 'text/css'
    elif ext == '.js':
        return 'application/javascript'
    elif ext == '.mp3':
        return 'audio/mpeg'
    elif ext == '.ogg':  # Ogg Vorbis/Opus
        return 'audio/ogg'
    elif ext == '.wav':
        return 'audio/wav'
    elif ext == '.spx':
        return 'audio/x-speex'  # 非标准，但有时使用
    return 'application/octet-stream'  # 默认


# --- 嵌入逻辑 ---
class ResourceEmbedder:
    def __init__(self, css_content_str=None, js_content_str=None, mdd_obj=None):
        self.css_content_str = css_content_str
        self.js_content_str = js_content_str
        self.mdd_obj = mdd_obj
        self.mdd_keys_cache = None
        if self.mdd_obj:
            try:
                # 获取所有键并转换为小写以便不区分大小写查找 (MDD键有时大小写敏感)
                # 将键从字节串解码为字符串进行处理
                self.mdd_keys_cache = {key.lower().decode('utf-8', errors='replace'): key for key in
                                       self.mdd_obj.keys()}
            except Exception as e:
                print(f"警告：无法从MDD获取键列表：{e}")
                self.mdd_keys_cache = {}

    def _normalize_mdd_key_for_lookup(self, path_str):
        if not path_str: return None
        path_str = path_str.split('?')[0].split('#')[0]

        # 尝试几种常见的键格式
        variants = [
            path_str,  # 原样
            path_str.lstrip('/'),  # 去除前导 /
            path_str.lstrip('\\'),  # 去除前导 \
            '/' + path_str.lstrip('/'),  # 以 / 开头
            '\\' + path_str.lstrip('/').lstrip('\\').replace('/', '\\')  # 以 \ 开头，替换 /
        ]

        for var in variants:
            # 在缓存中不区分大小写地查找
            if var.lower() in self.mdd_keys_cache:
                return self.mdd_keys_cache[var.lower()]  # 返回原始（正确大小写的）字节键
        return None

    def embed_css(self, html_content):
        if not self.css_content_str:
            return html_content

        # 替换所有 <link rel="stylesheet" ...> 标签
        # 这个正则表达式比较简单，可能需要根据实际HTML调整
        def replace_css_link(match):
            return f'<style type="text/css">\n{self.css_content_str}\n</style>'

        html_content = re.sub(r'<link[^>]*?rel\s*=\s*["\']stylesheet["\'][^>]*?>',
                              replace_css_link, html_content, flags=re.IGNORECASE)
        return html_content

    def embed_js(self, html_content):
        if not self.js_content_str:
            return html_content

        # 替换所有 <script src="..."></script> 标签
        def replace_js_link(match):
            print(f"提示：正在嵌入JS，替换脚本源: {match.group(1) if len(match.groups()) > 0 else match.group(0)}")
            return f'<script type="text/javascript">\n{self.js_content_str}\n</script>'

        html_content = re.sub(r'<script[^>]*?src\s*=\s*["\']([^"\']+)["\'][^>]*?>\s*</script>',
                              replace_js_link, html_content, flags=re.IGNORECASE | re.DOTALL)
        # 也处理自闭合的 <script ... src="..." /> (虽然不常见于HTML5)
        html_content = re.sub(r'<script[^>]*?src\s*=\s*["\']([^"\']+)["\'][^>]*?/>',
                              replace_js_link, html_content, flags=re.IGNORECASE | re.DOTALL)
        return html_content

    def embed_mdd_resources(self, html_content):
        if not self.mdd_obj:
            return html_content

        # 1. 嵌入图片 (<img> src)
        def replace_img_src(match):
            original_tag = match.group(0)
            img_src = match.group(1)

            mdd_key_bytes = self._normalize_mdd_key_for_lookup(img_src)
            if mdd_key_bytes:
                try:
                    data_bytes = self.mdd_obj.get_bytestring(mdd_key_bytes)
                    if data_bytes:
                        b64_data = base64.b64encode(data_bytes).decode('utf-8')
                        mime_type = get_mime_type(img_src)  # 从原始src获取文件名推断MIME
                        new_src = f'data:{mime_type};base64,{b64_data}'
                        print(f"提示：嵌入图片 '{img_src}' -> data URI (长度: {len(b64_data)})")
                        return original_tag.replace(img_src, new_src, 1)
                except Exception as e:
                    print(f"警告：嵌入图片 '{img_src}' (键: {mdd_key_bytes}) 失败: {e}")
            else:
                print(f"警告：在MDD中未找到图片资源: '{img_src}'")
            return original_tag  # 未找到或处理失败，返回原样

        html_content = re.sub(r'<img[^>]*?src\s*=\s*["\']([^"\']+)["\'][^>]*?>',
                              replace_img_src, html_content, flags=re.IGNORECASE)

        # 2. 嵌入音频 (snd:// links and <audio src>)
        # 处理 snd:// 链接
        def replace_snd_link(match):
            original_tag = match.group(0)
            resource_key_str = match.group(1)  # e.g., "abacus__gb_1.spx"
            link_content = match.group(2)  # e.g., "<audio-gb>🔊</audio-gb>"

            mdd_key_bytes = self._normalize_mdd_key_for_lookup(resource_key_str)
            if mdd_key_bytes:
                try:
                    data_bytes = self.mdd_obj.get_bytestring(mdd_key_bytes)
                    if data_bytes:
                        b64_data = base64.b64encode(data_bytes).decode('utf-8')
                        mime_type = get_mime_type(resource_key_str)
                        print(
                            f"提示：嵌入音频 (snd://) '{resource_key_str}' -> data URI (MIME: {mime_type}, 长度: {len(b64_data)})")
                        if mime_type == 'audio/x-speex':
                            print("警告: SPX 音频已嵌入，但浏览器可能无法直接播放。")
                        # 替换整个 <a> 标签为一个 <audio> 标签
                        return f'<audio controls src="data:{mime_type};base64,{b64_data}">{link_content} (音频)</audio>'
                except Exception as e:
                    print(f"警告：嵌入音频 (snd://) '{resource_key_str}' (键: {mdd_key_bytes}) 失败: {e}")
            else:
                print(f"警告：在MDD中未找到音频资源 (snd://): '{resource_key_str}'")
            return original_tag

        html_content = re.sub(r'<a[^>]*?href\s*=\s*["\']snd://([^"\']+)["\'][^>]*?>(.*?)</a>',
                              replace_snd_link, html_content, flags=re.IGNORECASE | re.DOTALL)

        # 处理 <audio src="..."> 标签 (如果词典也用这种方式引用MDD资源)
        def replace_audio_src(match):
            original_tag = match.group(0)
            audio_src = match.group(1)

            mdd_key_bytes = self._normalize_mdd_key_for_lookup(audio_src)
            if mdd_key_bytes:
                try:
                    data_bytes = self.mdd_obj.get_bytestring(mdd_key_bytes)
                    if data_bytes:
                        b64_data = base64.b64encode(data_bytes).decode('utf-8')
                        mime_type = get_mime_type(audio_src)
                        print(
                            f"提示：嵌入音频 (<audio src>) '{audio_src}' -> data URI (MIME: {mime_type}, 长度: {len(b64_data)})")
                        if mime_type == 'audio/x-speex':
                            print("警告: SPX 音频已嵌入，但浏览器可能无法直接播放。")
                        return original_tag.replace(audio_src, f'data:{mime_type};base64,{b64_data}', 1)
                except Exception as e:
                    print(f"警告：嵌入音频 (<audio src>) '{audio_src}' (键: {mdd_key_bytes}) 失败: {e}")
            else:
                print(f"警告：在MDD中未找到音频资源 (<audio src>): '{audio_src}'")
            return original_tag

        html_content = re.sub(r'<audio[^>]*?src\s*=\s*["\']([^"\']+)["\'][^>]*?>',
                              replace_audio_src, html_content, flags=re.IGNORECASE)
        return html_content

    def process_html(self, html_content):
        html_content = self.embed_css(html_content)
        html_content = self.embed_js(html_content)
        html_content = self.embed_mdd_resources(html_content)  # MDD最后处理，因为它可能替换了JS/CSS修改的标签
        return html_content


# --- 文件路径获取 ---
def get_file_paths():
    # (与之前脚本中的 get_user_paths 类似，但添加可选参数)
    while True:
        mdx_file_path_str = input("请输入源 MDX 文件路径: ").strip()
        if mdx_file_path_str[0] == '"' and mdx_file_path_str[-1] == '"':
            mdx_file_path_str = mdx_file_path_str[1:-1]
        mdx_file_path = Path(mdx_file_path_str)
        if mdx_file_path.is_file() and mdx_file_path.suffix.lower() == '.mdx':
            break
        else:
            print("错误：MDX 文件路径无效或文件不存在。请确保路径正确且以 .mdx 结尾。")

    while True:
        sqlite_file_path_str = input("请输入目标 SQLite 数据库文件路径 (例如 output.db): ").strip()
        if sqlite_file_path_str[0] == '"' and sqlite_file_path_str[-1] == '"':
            sqlite_file_path_str = sqlite_file_path_str[1:-1]
        sqlite_file_path = Path(sqlite_file_path_str)
        if not sqlite_file_path.name:  # 空输入
            print("错误: SQLite 文件名不能为空。")
            continue
        if sqlite_file_path.suffix.lower() not in ['.db', '.sqlite', '.sqlite3']:
            print(
                f"提示：输出文件名 '{sqlite_file_path.name}' 没有标准 SQLite 后缀，将使用 '{sqlite_file_path.with_suffix('.db').name}'。")
            sqlite_file_path = sqlite_file_path.with_suffix(".db")
        try:
            sqlite_file_path.parent.mkdir(parents=True, exist_ok=True)
            break
        except OSError as e:
            print(f"错误：无法创建 SQLite 文件的输出目录 '{sqlite_file_path.parent}': {e}")
        except Exception as e:
            print(f"输入 SQLite 文件路径时发生未知错误: {e}")

    # 可选参数
    css_path_str = input("请输入可选的 CSS 文件路径 (直接回车跳过): ").strip()
    if css_path_str and css_path_str[0] == '"' and css_path_str[-1] == '"':
        css_path_str = css_path_str[1:-1]
    css_path = Path(css_path_str) if css_path_str and Path(css_path_str).is_file() else None
    if css_path_str and not css_path: print(f"警告: 输入的 CSS 文件 '{css_path_str}' 不存在或无效，将忽略。")

    js_path_str = input("请输入可选的 JavaScript 文件路径 (直接回车跳过): ").strip()
    if js_path_str and js_path_str[0] == '"' and js_path_str[-1] == '"':
        js_path_str = js_path_str[1:-1]
    js_path = Path(js_path_str) if js_path_str and Path(js_path_str).is_file() else None
    if js_path_str and not js_path: print(f"警告: 输入的 JS 文件 '{js_path_str}' 不存在或无效，将忽略。")

    mdd_path_str = input("请输入可选的 MDD 文件路径 (直接回车跳过): ").strip()
    if mdd_path_str and mdd_path_str[0] == '"' and mdd_path_str[-1] == '"':
        mdd_path_str = mdd_path_str[1:-1]
    mdd_path = Path(mdd_path_str) if mdd_path_str and Path(mdd_path_str).is_file() else None
    if mdd_path_str and not mdd_path: print(f"警告: 输入的 MDD 文件 '{mdd_path_str}' 不存在或无效，将忽略。")

    return mdx_file_path, sqlite_file_path, css_path, js_path, mdd_path


# --- SQLite 操作 (create_tables, populate_info_table 基本不变) ---
def create_tables(cursor):
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
    if not mdx_header:
        print("警告：MDX 头部信息为空。")
        return
    header_data_to_insert = []
    for key_bytes, value_bytes in mdx_header.items():
        try:
            key = key_bytes.decode('utf-8', errors='replace')
            value = value_bytes.decode('utf-8', errors='replace')
            header_data_to_insert.append((key, value))
        except Exception as e:
            print(f"处理元数据时发生未知错误 - Key: {key_bytes}, Value: {value_bytes}, Error: {e}")
    if header_data_to_insert:
        try:
            cursor.executemany("INSERT OR REPLACE INTO info (AttributeName, AttributeValue) VALUES (?, ?)",
                               header_data_to_insert)
            print(f"成功将 {len(header_data_to_insert)} 条元数据写入 'info' 表。")
        except sqlite3.Error as e:
            print(f"错误：写入元数据到 'info' 表时发生 SQLite 错误: {e}")


# --- 填充词条表（修改版，集成嵌入逻辑） ---
def populate_word_table(conn, mdx_items_iterator, encoding, total_items, embedder):
    cursor = conn.cursor()
    batch = []
    processed_count = 0
    print(f"开始处理词条，预计共有 {total_items if total_items is not None else '未知数量的'} 词条...")
    print(f"词条和释义将使用编码: {encoding}")
    if embedder.css_content_str: print("提示: 将尝试嵌入CSS。")
    if embedder.js_content_str: print("提示: 将尝试嵌入JS。")
    if embedder.mdd_obj: print("提示: 将尝试嵌入MDD资源。")

    try:
        conn.execute("BEGIN TRANSACTION")
        for key_bytes, definition_bytes in tqdm(mdx_items_iterator, total=total_items, desc="转换词条中", unit="词条"):
            try:
                keyword = key_bytes.decode(encoding, errors='replace')
                definition_html_original = definition_bytes.decode(encoding, errors='replace')

                # 应用嵌入逻辑
                definition_html_processed = embedder.process_html(definition_html_original)

                batch.append((keyword, definition_html_processed))
            except Exception as e:  # 更广泛地捕获单个词条处理错误
                keyword_preview = key_bytes[:30].decode(encoding, errors='ignore')
                print(f"\n错误：处理词条 '{keyword_preview}...' 数据或嵌入资源时出错: {e}")
                # 可以选择是否将原始（未嵌入）HTML加入，或者跳过
                # batch.append((keyword_preview, definition_html_original)) # 例如，存入原始HTML
                continue

            if len(batch) >= BATCH_SIZE:
                cursor.executemany("INSERT OR REPLACE INTO word (keyword, definition_html) VALUES (?, ?)", batch)
                processed_count += len(batch)
                batch = []

        if batch:
            cursor.executemany("INSERT OR REPLACE INTO word (keyword, definition_html) VALUES (?, ?)", batch)
            processed_count += len(batch)

        conn.commit()
        print(f"词条处理完成。总共处理并尝试写入 {processed_count} 条词条到 'word' 表。")
        return True

    except (MdxError) as e:  # readmdict 相关的特定错误
        print(f"\n错误：读取 MDX 文件内容时发生错误 (readmdict): {e}")
        if conn: conn.rollback()
        return False
    except sqlite3.Error as e:
        print(f"\n错误：写入词条到 'word' 表时发生 SQLite 错误: {e}")
        if conn: conn.rollback()
        return False
    except Exception as e:
        print(f"\n错误：填充词条表时发生未知错误: {e}")
        if conn: conn.rollback()
        return False


# --- Main 函数 ---
def main():
    print("--- MDX 转 SQLite 脚本 (带资源嵌入功能) ---")
    mdx_file, sqlite_file, css_path, js_path, mdd_path = get_file_paths()

    print(f"\n源 MDX 文件: {mdx_file}")
    print(f"目标 SQLite 文件: {sqlite_file}")
    if css_path: print(f"嵌入 CSS 文件: {css_path}")
    if js_path: print(f"嵌入 JS 文件: {js_path}")
    if mdd_path: print(f"嵌入 MDD 资源文件: {mdd_path}")

    css_content = None
    if css_path:
        try:
            with open(css_path, 'r', encoding='utf-8') as f:
                css_content = f.read()
            print(f"成功读取 CSS 文件内容 (长度: {len(css_content)}).")
        except Exception as e:
            print(f"错误: 无法读取 CSS 文件 '{css_path}': {e}. CSS嵌入将跳过。")

    js_content = None
    if js_path:
        try:
            with open(js_path, 'r', encoding='utf-8') as f:
                js_content = f.read()
            print(f"成功读取 JS 文件内容 (长度: {len(js_content)}).")
        except Exception as e:
            print(f"错误: 无法读取 JS 文件 '{js_path}': {e}. JS嵌入将跳过。")

    mdd_reader = None
    if mdd_path:
        try:
            mdd_reader = MDD(str(mdd_path))
            print(f"成功打开 MDD 文件。包含约 {len(mdd_reader.keys())} 个资源键。")
        except Exception as e:
            print(f"错误: 无法打开 MDD 文件 '{mdd_path}': {e}. MDD资源嵌入将跳过。")

    embedder = ResourceEmbedder(css_content_str=css_content, js_content_str=js_content, mdd_obj=mdd_reader)

    mdx_instance = None
    conn = None
    num_entries = None

    try:
        print("正在打开和解析 MDX 文件...")
        mdx_instance = MDX(str(mdx_file))
        mdx_header = mdx_instance.header

        try:  # 尝试获取词条总数
            # 注意: mdx_instance.items() 是一个迭代器。如果想获取总数，
            # list(mdx_instance.keys()) 会消耗键迭代器。
            # 如果 keys() 和 items() 共享状态，这可能会导致 items() 为空。
            # readmdict 的 MDX 对象通常可以安全地多次调用 .keys() 或 .items()
            # 但为了避免意外，这里先用 keys() 获取数量，再用 items() 迭代。
            # 或者，如果MDX文件非常大，避免加载所有键到内存，将 num_entries 设为 None。
            key_list = list(mdx_instance.keys())  # 这会加载所有键
            num_entries = len(key_list)
            del key_list  # 释放内存
            print(f"MDX 文件元数据加载完毕，包含 {num_entries} 个词条。")
            items_iterator = mdx_instance.items()  # 重新获取迭代器
        except Exception as e_count:
            print(f"提示: 获取词条总数时出错 ({e_count})，进度条可能不显示百分比。")
            items_iterator = mdx_instance.items()

        record_encoding = 'UTF-8'  # 默认编码
        if mdx_header and b'Encoding' in mdx_header:
            try:
                enc = mdx_header[b'Encoding'].decode('utf-8', errors='replace').strip().upper()
                if enc:
                    "".encode(enc)  # Test encoding
                    record_encoding = enc
                print(f"从MDX元数据获取到词条编码: {record_encoding}")
            except Exception as e_enc:
                print(f"警告：无法解析MDX元数据中的 'Encoding' (错误: {e_enc})，将使用默认 UTF-8。")

        conn = sqlite3.connect(sqlite_file)
        cursor = conn.cursor()

        cursor.execute("PRAGMA synchronous = OFF")
        cursor.execute("PRAGMA journal_mode = MEMORY")

        create_tables(cursor)
        populate_info_table(cursor, mdx_header)
        conn.commit()

        if populate_word_table(conn, items_iterator, record_encoding, num_entries, embedder):
            print("词条数据填充成功。")
        else:
            print("词条数据填充过程中发生错误或被中止。")

        print("转换过程完成。")

    except FileNotFoundError:
        print(f"错误：MDX 文件 '{mdx_file}' 未找到。")
    except (MdxError) as e:
        print(f"错误：处理 MDX 文件时发生错误 (readmdict): {e}")
    except sqlite3.Error as e:
        print(f"错误：操作 SQLite 数据库时发生错误: {e}")
    except Exception as e:
        print(f"发生未知错误: {e}")
    finally:
        if conn:
            try:
                cursor = conn.cursor()
                cursor.execute("PRAGMA synchronous = FULL")
                cursor.execute("PRAGMA journal_mode = DELETE")
                conn.commit()
                conn.close()
                print("数据库连接已关闭。")
            except sqlite3.Error as e:
                print(f"关闭数据库连接或恢复PRAGMA时出错: {e}")
        if mdx_instance: del mdx_instance
        if mdd_reader: del mdd_reader  # MDD 对象也可能需要清理


if __name__ == '__main__':
    main()