import sqlite3
import sys
import re
import base64
import os
from pathlib import Path

# 尝试导入 readmdict 的核心组件
try:
    from readmdict import MDX, MDD
except ImportError:
    print("错误：readmdict 库未安装或其核心组件导入失败。请使用 'pip install readmdict' 命令安装。")
    sys.exit(1)

try:
    from tqdm import tqdm
except ImportError:
    print("提示：tqdm 库未安装。进度条将不可用。可以运行 'pip install tqdm' 来安装它。")


    def tqdm(iterable, *args, **kwargs):
        return iterable

# --- 全局设置 ---
BATCH_SIZE = 1000
# 常见的 MDX 头部封面图片键名 (字节串形式)
COVER_IMAGE_HEADER_KEYS = [b'Cover', b'CoverImage', b'cover', b'COVER']


# --- 辅助函数 ---
def get_mime_type(filename_or_path):
    filename_str = str(os.path.basename(str(filename_or_path)))
    ext = os.path.splitext(filename_str)[1].lower()
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
    elif ext == '.ogg':
        return 'audio/ogg'
    elif ext == '.wav':
        return 'audio/wav'
    elif ext == '.spx':
        return 'audio/x-speex'
    elif ext in ['.ttf', '.otf', '.woff', '.woff2']:
        if ext == '.ttf': return 'font/ttf'
        if ext == '.otf': return 'font/otf'
        if ext == '.woff': return 'font/woff'
        if ext == '.woff2': return 'font/woff2'
    return 'application/octet-stream'


# --- 嵌入逻辑 ---
class ResourceEmbedder:
    def __init__(self, js_content_str=None, mdd_objects_list=None):
        self.js_content_str = js_content_str
        self.mdd_providers = []

        if mdd_objects_list:
            for i, mdd_obj in enumerate(mdd_objects_list):
                try:
                    keys_cache = {key.decode('utf-8', errors='replace').lower(): key for key in mdd_obj.keys()}
                    self.mdd_providers.append({'obj': mdd_obj, 'keys_cache': keys_cache, 'id': f"MDD_{i + 1}"})
                except Exception as e:
                    print(f"警告：初始化 MDD 解析器 {i + 1} 的键缓存时出错: {e}")

    def _find_mdd_resource(self, path_str_from_html):
        if not path_str_from_html or not self.mdd_providers:
            return None, None, None, None

        resource_name_cleaned = path_str_from_html.split('?')[0].split('#')[0]
        if not resource_name_cleaned.strip():
            return None, None, None, None

        possible_key_forms = [
            resource_name_cleaned,
            resource_name_cleaned.lstrip('/\\'),
            '/' + resource_name_cleaned.lstrip('/\\'),
            '\\' + resource_name_cleaned.lstrip('/\\').replace('/', '\\')
        ]
        for provider in self.mdd_providers:
            mdd_obj = provider['obj']
            keys_cache = provider['keys_cache']
            for pk_form_str in possible_key_forms:
                lookup_key_str_lower = pk_form_str.lower()
                if lookup_key_str_lower in keys_cache:
                    original_byte_key = keys_cache[lookup_key_str_lower]
                    try:
                        data_bytes = mdd_obj.get_bytestring(original_byte_key)
                        if data_bytes:
                            mime_type = get_mime_type(resource_name_cleaned)
                            return data_bytes, mime_type, provider['id'], original_byte_key
                    except Exception as e_get:
                        print(
                            f"警告：从 {provider['id']} 获取资源 '{pk_form_str}' (原始键: {original_byte_key}) 失败: {e_get}")
                        continue
        return None, None, None, None

    def embed_js(self, html_content):
        if not self.js_content_str:
            return html_content

        def replace_js_link(match):
            js_src_attr = match.group(1) if len(match.groups()) > 0 and match.group(1) else "未知JS源"
            return f'<script type="text/javascript">\n{self.js_content_str}\n</script>'

        html_content = re.sub(r'<script[^>]*?src\s*=\s*["\']([^"\']+)["\'][^>]*?>\s*</script>',
                              replace_js_link, html_content, flags=re.IGNORECASE | re.DOTALL)
        html_content = re.sub(r'<script[^>]*?src\s*=\s*["\']([^"\']+)["\'][^>]*?/>',
                              replace_js_link, html_content, flags=re.IGNORECASE | re.DOTALL)
        return html_content

    def embed_mdd_resources(self, html_content):
        if not self.mdd_providers: return html_content

        def replace_img_src(match):
            original_tag, img_src_from_html = match.group(0), match.group(1)
            data_bytes, mime_type, found_in, _ = self._find_mdd_resource(img_src_from_html)
            if data_bytes:
                b64_data = base64.b64encode(data_bytes).decode('utf-8')
                new_src = f'data:{mime_type};base64,{b64_data}'

                def escape_for_regex_replace(s): return s.replace('\\', r'\\')

                return re.sub(r'src\s*=\s*["\']' + re.escape(img_src_from_html) + r'["\']',
                              f'src="{escape_for_regex_replace(new_src)}"', original_tag, count=1, flags=re.IGNORECASE)
            return original_tag

        html_content = re.sub(r'<img[^>]*?src\s*=\s*["\']([^"\':]+)["\'][^>]*?>', replace_img_src, html_content,
                              flags=re.IGNORECASE)

        def replace_snd_link(match):
            original_tag, resource_key_str, link_content = match.group(0), match.group(1), match.group(2)
            data_bytes, mime_type, found_in, _ = self._find_mdd_resource(resource_key_str)
            if data_bytes:
                b64_data = base64.b64encode(data_bytes).decode('utf-8')
                if mime_type == 'audio/x-speex': print("警告: SPX 音频已嵌入，但浏览器可能无法直接播放。")
                return f'<audio controls src="data:{mime_type};base64,{b64_data}">{link_content} (音频)</audio>'
            return original_tag

        html_content = re.sub(r'<a[^>]*?href\s*=\s*["\']snd://([^"\']+)["\'][^>]*?>(.*?)</a>', replace_snd_link,
                              html_content, flags=re.IGNORECASE | re.DOTALL)

        def replace_audio_src(match):
            original_tag, audio_src_from_html = match.group(0), match.group(1)
            data_bytes, mime_type, found_in, _ = self._find_mdd_resource(audio_src_from_html)
            if data_bytes:
                b64_data = base64.b64encode(data_bytes).decode('utf-8')
                new_src = f'data:{mime_type};base64,{b64_data}'
                if mime_type == 'audio/x-speex': print("警告: SPX 音频已嵌入，但浏览器可能无法直接播放。")

                def escape_for_regex_replace(s):
                    return s.replace('\\', r'\\')

                return re.sub(r'src\s*=\s*["\']' + re.escape(audio_src_from_html) + r'["\']',
                              f'src="{escape_for_regex_replace(new_src)}"', original_tag, count=1, flags=re.IGNORECASE)
            return original_tag

        html_content = re.sub(r'<audio[^>]*?src\s*=\s*["\']([^"\':]+)["\'][^>]*?>', replace_audio_src, html_content,
                              flags=re.IGNORECASE)
        return html_content

    def process_html(self, html_content):
        html_content = self.embed_js(html_content)
        html_content = self.embed_mdd_resources(html_content)
        return html_content


# --- 文件路径获取 (修改以支持独立封面) ---
def get_file_paths():
    while True:
        mdx_file_path_str = input("请输入源 MDX 文件路径: ").strip()
        if mdx_file_path_str.startswith('"') and mdx_file_path_str.endswith('"'):
            mdx_file_path_str = mdx_file_path_str[1:-1]
        mdx_file_path = Path(mdx_file_path_str)
        if mdx_file_path.is_file() and mdx_file_path.suffix.lower() == '.mdx':
            break
        else:
            print("错误：MDX 文件路径无效或文件不存在。请确保路径正确且以 .mdx 结尾。")

    while True:
        sqlite_file_path_str = input("请输入目标 SQLite 数据库文件路径 (例如 output.db): ").strip()
        if sqlite_file_path_str.startswith('"') and sqlite_file_path_str.endswith('"'):
            sqlite_file_path_str = sqlite_file_path_str[1:-1]
        sqlite_file_path = Path(sqlite_file_path_str)
        if not sqlite_file_path.name:
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

    css_path_str = input("请输入可选的 CSS 文件路径 (直接回车跳过): ").strip()
    if css_path_str.startswith('"') and css_path_str.endswith('"'):
        css_path_str = css_path_str[1:-1]
    css_path = Path(css_path_str) if css_path_str and Path(css_path_str).is_file() else None
    if css_path_str and not css_path: print(f"警告: 输入的 CSS 文件 '{css_path_str}' 不存在或无效，将忽略。")

    js_path_str = input("请输入可选的 JavaScript 文件路径 (直接回车跳过): ").strip()
    if js_path_str.startswith('"') and js_path_str.endswith('"'):
        js_path_str = js_path_str[1:-1]
    js_path = Path(js_path_str) if js_path_str and Path(js_path_str).is_file() else None
    if js_path_str and not js_path: print(f"警告: 输入的 JS 文件 '{js_path_str}' 不存在或无效，将忽略。")

    mdd_paths_list = []
    print("请输入可选的 MDD 文件路径 (每行一个，直接回车结束输入):")
    while True:
        mdd_path_str = input(f"MDD 文件 {len(mdd_paths_list) + 1}: ").strip()
        if not mdd_path_str:
            break
        if mdd_path_str.startswith('"') and mdd_path_str.endswith('"'):
            mdd_path_str = mdd_path_str[1:-1]
        mdd_path_candidate = Path(mdd_path_str)
        if mdd_path_candidate.is_file() and mdd_path_candidate.suffix.lower() == '.mdd':
            mdd_paths_list.append(mdd_path_candidate)
            print(f"已添加 MDD: {mdd_path_candidate}")
        else:
            print(f"警告: 输入的 MDD 文件 '{mdd_path_str}' 无效或不存在，已忽略。")

    # 新增：获取独立封面图片路径
    standalone_cover_path_str = input("请输入可选的独立封面图片文件路径 (PNG, JPG, GIF, WEBP) (直接回车跳过): ").strip()
    if standalone_cover_path_str.startswith('"') and standalone_cover_path_str.endswith('"'):
        standalone_cover_path_str = standalone_cover_path_str[1:-1]

    standalone_cover_path = None
    if standalone_cover_path_str:
        candidate_path = Path(standalone_cover_path_str)
        if candidate_path.is_file() and candidate_path.suffix.lower() in ['.png', '.jpg', '.jpeg', '.gif', '.webp']:
            standalone_cover_path = candidate_path
        else:
            print(f"警告: 输入的独立封面图片文件 '{standalone_cover_path_str}' 无效、不存在或格式不支持，将忽略。")

    return mdx_file_path, sqlite_file_path, css_path, js_path, mdd_paths_list, standalone_cover_path


# --- SQLite 操作 ---
def create_tables(cursor):
    cursor.execute("CREATE TABLE IF NOT EXISTS info (AttributeName TEXT PRIMARY KEY, AttributeValue TEXT)")
    cursor.execute("CREATE TABLE IF NOT EXISTS word (keyword TEXT PRIMARY KEY, definition_html TEXT)")
    print("数据库表 'info' 和 'word' 已创建（如果它们不存在）。")


def populate_info_table(cursor, mdx_header, other_info_to_add=None):
    all_info_to_insert = []
    if mdx_header:
        for key_bytes, value_bytes in mdx_header.items():
            try:
                key = key_bytes.decode('utf-8', errors='replace')
                value = value_bytes.decode('utf-8', errors='replace')
                all_info_to_insert.append((key, value))
            except Exception as e:
                print(f"处理元数据时发生未知错误 - Key: {key_bytes}, Value: {value_bytes}, Error: {e}")

    if other_info_to_add:
        all_info_to_insert.extend(other_info_to_add)

    if all_info_to_insert:
        try:
            cursor.executemany("INSERT OR REPLACE INTO info (AttributeName, AttributeValue) VALUES (?, ?)",
                               all_info_to_insert)
            print(f"成功将 {len(all_info_to_insert)} 条元数据写入 'info' 表。")
        except sqlite3.Error as e:
            print(f"错误：写入元数据到 'info' 表时发生 SQLite 错误: {e}")
    else:
        print("没有元数据可写入 'info' 表。")


def populate_word_table(conn, mdx_items_iterator, encoding, total_items, embedder):
    cursor = conn.cursor()
    batch = []
    processed_count = 0
    print(f"开始处理词条，预计共有 {total_items if total_items is not None else '未知数量的'} 词条...")
    print(f"词条和释义将使用编码: {encoding}")
    if embedder.js_content_str: print("提示: 将尝试嵌入JS。")
    if embedder.mdd_providers: print(f"提示: 将尝试从 {len(embedder.mdd_providers)} 个MDD文件中嵌入资源。")

    try:
        conn.execute("BEGIN TRANSACTION")
        for key_bytes, definition_bytes in tqdm(mdx_items_iterator, total=total_items, desc="转换词条中", unit="词条"):
            try:
                keyword = key_bytes.decode(encoding, errors='replace')
                definition_html_original = definition_bytes.decode(encoding, errors='replace')
                definition_html_processed = embedder.process_html(definition_html_original)
                batch.append((keyword, definition_html_processed))
            except Exception as e:
                keyword_preview = key_bytes[:30].decode(encoding, errors='ignore')
                print(f"\n错误：处理词条 '{keyword_preview}...' 数据或嵌入资源时出错: {e}")
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
    except sqlite3.Error as e:
        print(f"\n错误：写入词条到 'word' 表时发生 SQLite 错误: {e}")
        if conn: conn.rollback()
        return False
    except Exception as e:
        print(f"\n错误：填充词条表时发生错误 (可能来自MDX/MDD处理或其它): {e}")
        if conn: conn.rollback()
        return False


# --- Main 函数 (修改以处理独立封面) ---
def main():
    print("--- MDX 转 SQLite (CSS/Font/Cover信息存入info表, 支持独立封面) ---")
    mdx_file, sqlite_file, css_path, js_path, mdd_paths, standalone_cover_file_path = get_file_paths()

    print(f"\n源 MDX 文件: {mdx_file}")
    print(f"目标 SQLite 文件: {sqlite_file}")

    additional_info_for_db = []

    # 1. 处理 CSS 和相关字体
    if css_path:
        print(f"处理 CSS 文件: {css_path}")
        try:
            with open(css_path, 'r', encoding='utf-8') as f:
                css_content_data = f.read()
            print(f"成功读取 CSS 文件内容 (长度: {len(css_content_data)}).")
            additional_info_for_db.append(('CSS_TARGET_HREF', css_path.name))
            additional_info_for_db.append(('CSS_REPLACEMENT_CONTENT', css_content_data))
            print(f"提示: CSS 文件 '{css_path.name}' 的信息将存储在 info 表中。")

            font_face_blocks = re.findall(r"@font-face\s*\{[^{}]*\}", css_content_data, re.IGNORECASE | re.DOTALL)
            if font_face_blocks:
                print(f"在CSS中找到 {len(font_face_blocks)} 个 @font-face 块。正在尝试提取字体URL...")

            temp_mdd_reader_list_for_fonts = []
            if mdd_paths:
                for mdd_p_font in mdd_paths:
                    try:
                        temp_mdd_reader_list_for_fonts.append(MDD(str(mdd_p_font)))
                    except:
                        pass

            if temp_mdd_reader_list_for_fonts or not font_face_blocks:  # 如果没有font_face，也不需要MDD来找字体
                font_embedder_helper = ResourceEmbedder(mdd_objects_list=temp_mdd_reader_list_for_fonts)
                found_font_urls = set()

                for block in font_face_blocks:
                    urls_in_block = re.findall(r"url\((['\"]?)([^()\"']+?)\1\)", block, re.IGNORECASE)
                    for _, font_url in urls_in_block:
                        font_filename = os.path.basename(font_url.strip())
                        if not font_filename or font_filename in found_font_urls:
                            continue
                        found_font_urls.add(font_filename)

                        print(f"  尝试在MDD中查找字体文件: '{font_filename}' (从CSS URL: '{font_url}')")
                        font_data_bytes, font_mime, found_in_mdd, _ = font_embedder_helper._find_mdd_resource(
                            font_filename)

                        if font_data_bytes:
                            font_base64_content = base64.b64encode(font_data_bytes).decode('utf-8')
                            additional_info_for_db.append((f'FONT_TARGET_URL_{font_filename}', font_filename))
                            additional_info_for_db.append((f'FONT_BASE64_CONTENT_{font_filename}', font_base64_content))
                            print(
                                f"    -> 找到并为 '{font_filename}' (MIME: {font_mime}) 生成Base64数据 (来自 {found_in_mdd})。将存入info表。")
                        else:
                            print(f"    -> 未在提供的MDD文件中找到字体: '{font_filename}'")
            elif font_face_blocks:  # 有font_face但没有MDD
                print("警告: CSS中找到@font-face规则，但未提供MDD文件，无法查找和嵌入字体文件。")
        except Exception as e:
            print(f"错误: 无法读取或处理 CSS 文件 '{css_path}': {e}. CSS及字体信息将不会存储。")
    else:
        print("未提供 CSS 文件路径，不处理CSS及相关字体信息存储。")

    # 2. 处理独立封面图片文件
    if standalone_cover_file_path:
        print(f"处理独立封面图片文件: {standalone_cover_file_path}")
        try:
            with open(standalone_cover_file_path, 'rb') as f_cover:
                cover_bytes = f_cover.read()
            cover_base64 = base64.b64encode(cover_bytes).decode('utf-8')
            additional_info_for_db.append(('STANDALONE_COVER_FILENAME', standalone_cover_file_path.name))
            additional_info_for_db.append(('STANDALONE_COVER_BASE64_CONTENT', cover_base64))
            print(f"成功读取并为独立封面 '{standalone_cover_file_path.name}' 生成Base64数据。将存入info表。")
        except Exception as e_scover:
            print(f"错误: 无法读取或处理独立封面图片文件 '{standalone_cover_file_path}': {e_scover}")

    # 3. JS 文件处理 (仍然是直接嵌入到词条HTML)
    js_content_data = None
    if js_path:
        print(f"处理 JS 文件: {js_path}")
        try:
            with open(js_path, 'r', encoding='utf-8') as f:
                js_content_data = f.read()
            print(f"成功读取 JS 文件内容 (长度: {len(js_content_data)}). JS将嵌入词条HTML。")
        except Exception as e:
            print(f"错误: 无法读取 JS '{js_path}': {e}. 跳过JS嵌入。")

    # 4. MDD 文件列表和 ResourceEmbedder 初始化 (用于词条内资源)
    mdd_reader_list_main = []
    if mdd_paths:
        print(f"将从以下 MDD 文件中嵌入主要资源 (图片、音频到词条HTML):")
        for idx, mdd_p in enumerate(mdd_paths):
            try:
                mdd_reader_list_main.append(MDD(str(mdd_p)))
                print(f"  - 成功打开 MDD 文件 ({idx + 1}/{len(mdd_paths)}): {mdd_p}")
            except Exception as e:
                print(f"错误: 无法打开 MDD '{mdd_p}' 用于主要资源: {e}. 将忽略此MDD文件。")
    # else: # 保持之前的打印逻辑
    #     print("未提供 MDD 文件，不进行主要资源嵌入。")

    embedder = ResourceEmbedder(js_content_str=js_content_data, mdd_objects_list=mdd_reader_list_main)

    mdx_instance = None
    conn = None
    num_entries = None

    try:
        print("正在打开和解析 MDX 文件...")
        mdx_instance = MDX(str(mdx_file))
        mdx_header = mdx_instance.header

        # 5. 处理 MDX 头部定义的词典封面图片
        if mdx_header and mdd_reader_list_main:
            found_header_cover = False
            for cover_key_bytes in COVER_IMAGE_HEADER_KEYS:
                if cover_key_bytes in mdx_header:
                    cover_resource_name_bytes = mdx_header[cover_key_bytes]
                    try:
                        cover_resource_name = cover_resource_name_bytes.decode('utf-8', errors='replace').strip()
                        if not cover_resource_name: continue

                        print(
                            f"在MDX头部找到封面图片引用 (键: {cover_key_bytes.decode()}), 资源名: '{cover_resource_name}'")
                        cover_data_bytes, cover_mime, found_in_mdd, _ = embedder._find_mdd_resource(cover_resource_name)

                        if cover_data_bytes:
                            cover_base64_content = base64.b64encode(cover_data_bytes).decode('utf-8')
                            additional_info_for_db.append(('MDX_HEADER_COVER_FILENAME', cover_resource_name))  # 使用不同键名
                            additional_info_for_db.append(('MDX_HEADER_COVER_BASE64_CONTENT', cover_base64_content))
                            print(
                                f"  -> 找到并为MDX头部封面 '{cover_resource_name}' (MIME: {cover_mime}) 生成Base64数据 (来自 {found_in_mdd})。将存入info表。")
                            found_header_cover = True
                            break
                        else:
                            print(f"  -> 未在提供的MDD文件中找到MDX头部封面图片资源: '{cover_resource_name}'")
                    except Exception as e_cover_decode:
                        print(f"警告: 解码MDX头部封面图片资源名失败: {e_cover_decode}")
            if not found_header_cover and any(k in mdx_header for k in COVER_IMAGE_HEADER_KEYS):  # 如果有相关键但没找到文件
                print("MDX头部包含封面引用，但未在MDD中找到对应资源。")
        elif not mdd_reader_list_main and mdx_header and any(k in mdx_header for k in COVER_IMAGE_HEADER_KEYS):
            print("警告: MDX头部可能包含封面信息，但未提供MDD文件，无法查找MDX头部定义的封面图片。")

        # (后续的词条总数获取、编码确定、数据库连接等逻辑不变)
        try:
            key_list = list(mdx_instance.keys())
            num_entries = len(key_list)
            del key_list
            print(f"MDX 文件元数据加载完毕，包含 {num_entries} 个词条。")
            items_iterator = mdx_instance.items()
        except Exception as e_count:
            print(f"提示: 获取词条总数时出错 ({e_count})，进度条可能不显示百分比。")
            items_iterator = mdx_instance.items()

        record_encoding = 'UTF-8'
        if mdx_header and b'Encoding' in mdx_header:
            try:
                enc = mdx_header[b'Encoding'].decode('utf-8', 'replace').strip().upper()
                if enc:
                    "".encode(enc)
                    record_encoding = enc
                print(f"从MDX元数据获取到词条编码: {record_encoding}")
            except Exception as e_enc:
                print(f"警告：解析编码失败 ({e_enc})，使用默认 UTF-8。")

        conn = sqlite3.connect(sqlite_file)
        cursor = conn.cursor()
        cursor.execute("PRAGMA synchronous = OFF")
        cursor.execute("PRAGMA journal_mode = MEMORY")

        create_tables(cursor)
        populate_info_table(cursor, mdx_header, additional_info_for_db)
        conn.commit()

        if populate_word_table(conn, items_iterator, record_encoding, num_entries, embedder):
            print("词条数据填充成功。")
        else:
            print("词条数据填充过程中发生错误或被中止。")
        print("转换过程完成。")

    except FileNotFoundError:
        print(f"错误：MDX 文件 '{mdx_file}' 未找到。")
    except sqlite3.Error as e:
        print(f"错误：SQLite 操作: {e}")
    except Exception as e:
        print(f"发生未知错误 (可能来自MDX/MDD处理或其它): {e}")
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
                print(f"关闭数据库时出错: {e}")


if __name__ == '__main__':
    main()
