#pragma once

#include <string>
#include <winrt/Windows.Storage.h> // 引入 Windows.Storage 命名空间

namespace WordWiz // 外部命名空间
{
    namespace Data // 内部命名空间
    {
        class FilePathProvider
        {
        public:
            // 获取应用的本地数据文件夹路径
            // 返回一个 UTF-8 编码的 std::string
            static std::string GetAppLocalFolderPath();
        };
    }
}
