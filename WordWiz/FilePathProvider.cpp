#include "pch.h" // 如果你的项目使用预编译头文件，请包含它
#include "FilePathProvider.h" // 确保这里的路径和你的项目结构一致
#include <winrt/Windows.Storage.h>
#include <locale> // 用于 wstring_convert
#include <codecvt> // 用于 wstring_convert

// 辅助函数：将 std::wstring (通常是 UTF-16 on Windows) 转换为 std::string (UTF-8)
std::string WideStringToString_UTF8(const std::wstring& wstr)
{
    if (wstr.empty())
    {
        return std::string();
    }
    // 计算转换后的UTF-8字符串所需的大小
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.length(), NULL, 0, NULL, NULL);
    if (size_needed <= 0) // 错误检查
    {
        // 可以根据需要处理错误，例如抛出异常或返回空字符串
        // For simplicity, returning empty string here.
        // Consider logging an error in a real application.
        return std::string();
    }
    std::string strTo(size_needed, 0);
    // 执行转换
    int chars_converted = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.length(), &strTo[0], size_needed, NULL, NULL);
    if (chars_converted <= 0) // 错误检查
    {
        // 同上，处理错误
        return std::string();
    }
    return strTo;
}


namespace WordWiz // 外部命名空间
{
    namespace Data // 内部命名空间
    {
        std::string FilePathProvider::GetAppLocalFolderPath()
        {
            try
            {
                // 获取应用的本地数据文件夹对象
                winrt::Windows::Storage::StorageFolder localFolder = winrt::Windows::Storage::ApplicationData::Current().LocalFolder();

                // 获取文件夹路径的 hstring (宽字符)
                winrt::hstring path_hstring = localFolder.Path();

                // 将 winrt::hstring (其 .c_str() 返回 const wchar_t*) 转换为 std::wstring
                std::wstring wide_path(path_hstring.c_str());

                // 使用辅助函数将 std::wstring (UTF-16) 转换为 std::string (UTF-8)
                return WideStringToString_UTF8(wide_path);
            }
            catch (winrt::hresult_error const& ex)
            {
                // 发生 WinRT API 错误，例如权限问题或API不可用
                // 在实际应用中，这里应该记录错误详情
                // winrt::hstring errorMessage = ex.message();
                // std::wstring wErrorMessage(errorMessage.c_str());
                // OutputDebugStringW(L"Error getting app local folder path: ");
                // OutputDebugStringW(wErrorMessage.c_str());
                // OutputDebugStringW(L"\n");
                return std::string(); // 返回空字符串表示失败
            }
            catch (const std::exception& e)
            {
                // 发生标准库异常 (例如，内存分配失败)
                // 在实际应用中，这里应该记录错误详情
                // OutputDebugStringA("Standard exception getting app local folder path: ");
                // OutputDebugStringA(e.what());
                // OutputDebugStringA("\n");
                return std::string(); // 返回空字符串表示失败
            }
            catch (...)
            {
                // 捕获所有其他未知类型的异常
                // OutputDebugStringW(L"Unknown error getting app local folder path.\n");
                return std::string(); // 返回空字符串表示失败
            }
        }
    }
}
