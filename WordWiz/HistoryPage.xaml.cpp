#include "pch.h"
#include "HistoryPage.xaml.h"
#if __has_include("HistoryPage.g.cpp")
#include "HistoryPage.g.cpp"
#endif
#include "FilePathProvider.h"
#include "database.h"
#include <iostream>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Storage.Streams.h> // For FileIO (needed for CopyAsync)


using namespace winrt;
using namespace Microsoft::UI::Xaml;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::WordWiz::implementation
{
    int32_t HistoryPage::MyProperty()
    {
        throw hresult_not_implemented();
    }

    void HistoryPage::MyProperty(int32_t /* value */)
    {
        throw hresult_not_implemented();
    }

#include <winrt/Windows.ApplicationModel.h> // For Package::Current()
#include <winrt/Windows.Storage.h>        // For StorageFolder, StorageFile, ApplicationData
#include <winrt/Windows.Storage.Streams.h> // For FileIO (needed for CopyAsync)
#include <winrt/Windows.Foundation.h>     // For IAsyncAction, IAsyncOperation
#include <string>

    // 假设这些服务和日志函数已经定义
    namespace WordWizServices {
        namespace Log {
            void LogMessage(const winrt::hstring& message); // 你的日志函数
        }
    }

    // 异步函数：复制默认文件到 LocalState
    // 返回 winrt::Windows::Foundation::IAsyncAction，表示这是一个可等待的异步操作
    winrt::Windows::Foundation::IAsyncAction CopyDefaultFileToLocalStateAsync()
    {
        // 定义文件在 MSIX 包内的相对路径
        const winrt::hstring sourceRelativePathInPackage = L"Assets\\db\\englishwords.wordwiz";
        // 定义复制到 LocalState 后的文件名
        const winrt::hstring targetFileNameInLocalState = L"englishwords.wordwiz";

        try
        {
            WordWizServices::Log::LogMessage(L"异步：开始检查并复制默认文件到 LocalState...");

            // 1. 获取应用的 LocalState 文件夹 (可读写)
            winrt::Windows::Storage::StorageFolder localStateFolder =
                winrt::Windows::Storage::ApplicationData::Current().LocalFolder();

            // 2. 检查 LocalState 中是否存在目标文件
            winrt::Windows::Storage::StorageFile targetFileInLocalState = nullptr;
            bool fileExistsInLocalState = false;

            try {
                targetFileInLocalState = co_await localStateFolder.GetFileAsync(targetFileNameInLocalState);
                fileExistsInLocalState = true;
                WordWizServices::Log::LogMessage(L"异步：文件已存在于 LocalState: " + targetFileInLocalState.Path());
            }
            catch (winrt::hresult_error const& ex) {
                // HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) 是文件不存在的常见错误码 (0x80070002)
                if (ex.code() == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
                    WordWizServices::Log::LogMessage(L"异步：文件在 LocalState 中不存在，将从包中复制.");
                    fileExistsInLocalState = false;
                }
                else {
                    // 其他错误，记录并重新抛出
                    WordWizServices::Log::LogMessage(L"异步：检查 LocalState 文件时发生意外错误: " + ex.message());
                    throw;
                }
            }

            if (!fileExistsInLocalState) {
                // 3. 如果 LocalState 中不存在，则从应用安装包复制
                WordWizServices::Log::LogMessage(L"异步：开始从应用包复制文件到 LocalState...");

                // 获取应用安装包文件夹 (只读)
                winrt::Windows::Storage::StorageFolder installFolder =
                    winrt::Windows::ApplicationModel::Package::Current().InstalledLocation();

                // 获取包内的源文件
                winrt::Windows::Storage::StorageFile sourceFileInPackage =
                    co_await installFolder.GetFileAsync(sourceRelativePathInPackage);

                // 复制文件到 LocalState。
                // NameCollisionOption::ReplaceExisting：如果目标文件已存在，则替换。
                targetFileInLocalState = co_await sourceFileInPackage.CopyAsync(
                    localStateFolder,
                    targetFileNameInLocalState,
                    winrt::Windows::Storage::NameCollisionOption::ReplaceExisting);

                WordWizServices::Log::LogMessage(L"异步：文件已成功复制到 LocalState: " + targetFileInLocalState.Path());
            }

            // --- 文件复制完成后，你可以使用 targetFileInLocalState 对象进行后续操作 ---
            // 例如，如果你想获取复制后的文件路径：
            // winrt::hstring finalFilePath = targetFileInLocalState.Path();
            // WordWizServices::Log::LogMessage(L"最终文件路径: " + finalFilePath);

        }
        catch (winrt::hresult_error const& ex)
        {
            // 捕获并处理 WinRT 文件操作相关的错误
            WordWizServices::Log::LogMessage(L"异步：文件复制/检查过程中发生错误: " + ex.message());
            // 可以选择在这里不重新抛出，或者根据你的错误处理策略重新抛出
        }
        catch (const std::exception& e)
        {
            // 捕获标准库异常
            WordWizServices::Log::LogMessage(L"异步：文件复制/检查失败 (标准异常): " + winrt::to_hstring(e.what()));
        }
    }

    winrt::Windows::Foundation::IAsyncAction HistoryPage::myButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        co_await CopyDefaultFileToLocalStateAsync();
        WordWizServices::Log::LogMessage(L"异步：文件复制已完成，页面可以继续加载内容.");

        myButton().Content(box_value(L"Clicked"));
    }
}
