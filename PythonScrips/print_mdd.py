#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import argparse
from readmdict import MDD  # 从 readmdict 库导入 MDD 类

import argparse
from readmdict import MDD  # 从 readmdict 库导入 MDD 类


def list_mdd_resources(mdd_file_path):
    """
    列出指定 MDD 文件中的所有资源。
    """
    try:
        print(f"正在打开 MDD 文件: {mdd_file_path}")
        mdd_obj = MDD(mdd_file_path)

        # .keys() 方法可能返回一个生成器，我们将其转换为列表
        # 这样就可以获取长度并进行多次操作（如果需要）
        keys_generator = mdd_obj.keys()
        resource_keys_bytes = list(keys_generator)  # <--- 修改在这里：将生成器转换为列表

        if not resource_keys_bytes:  # 现在 resource_keys_bytes 是列表，这个判断是可靠的
            print(f"在 '{mdd_file_path}' 文件中未找到任何资源。")
            return

        # 现在可以安全地使用 len()
        print(f"\n在 '{mdd_file_path}' 中找到的资源 ({len(resource_keys_bytes)} 个):")

        successful_decode_count = 0
        failed_decode_keys = []

        for key_bytes in resource_keys_bytes:  # 迭代列表
            try:
                key_str = key_bytes.decode('utf-8')
                print(key_str)
                successful_decode_count += 1
            except UnicodeDecodeError:
                try:
                    key_str = key_bytes.decode('gbk')
                    print(f"{key_str} (使用 GBK 解码)")
                    successful_decode_count += 1
                except UnicodeDecodeError:
                    failed_decode_keys.append(key_bytes)
                    print(f"[解码失败] 原始字节: {key_bytes!r}")

        print(f"\n总计资源数: {len(resource_keys_bytes)}")  # 现在可以安全地使用 len()
        print(f"成功解码资源名数: {successful_decode_count}")
        if failed_decode_keys:
            print(f"解码失败资源名数: {len(failed_decode_keys)}")

    except FileNotFoundError:
        print(f"错误: 文件 '{mdd_file_path}' 未找到。")
    except Exception as e:
        print(f"处理 MDD 文件时发生错误: {e}")
        print("请确保你已正确安装 'readmdict' 库，并且提供的是一个有效的 MDD 文件。")


if __name__ == "__main__":
    mdd_file = input("请输入 MDD 文件路径: ").strip()
    if mdd_file[0] == '"' and mdd_file[-1] == '"':
        mdd_file = mdd_file[1:-1]
    list_mdd_resources(mdd_file)