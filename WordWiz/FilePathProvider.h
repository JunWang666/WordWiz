#pragma once

#include <string>
#include <winrt/Windows.Storage.h> // 引入 Windows.Storage 命名空间

namespace WordWizServices
{
    namespace Data
    {        
        class FilePathProvider
        {
        public:
            // 获取应用的本地数据文件夹路径
            static std::string GetAppLocalFolderPath();

            // 获取应用的本地缓存文件夹路径
            static std::string GetAppLocalCacheFolderPath();   
            static std::string GetAppLocalSettingsPath();

            // 获取应用的共享本地文件夹路径
            static std::string GetAppSharedLocalFolderPath();

            // 获取应用的临时文件夹路径
            static std::string GetAppTemporaryFolderPath();
        };
    }
}
