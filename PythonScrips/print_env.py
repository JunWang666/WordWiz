import sys
print("--- Python环境诊断信息 ---")
print(f"Python解释器路径 (sys.executable): {sys.executable}")
print("Python模块搜索路径 (sys.path):")
for p in sys.path:
    print(p)
print("--------------------------")

# 然后再是你脚本原有的 import readmdict 等代码
try:
    from readmdict import MDX, MdxError
    print("readmdict 库已成功导入！") # 可以在这里加一句成功导入的提示
except ImportError:
    print("错误：readmdict 库未安装。请使用 'pip install readmdict' 命令安装。")

import readmdict
print(readmdict) # 查看模块对象信息，特别是 file 路径
print(dir(readmdict)) # 列出 readmdict 模块下所有可用的属性和方法名