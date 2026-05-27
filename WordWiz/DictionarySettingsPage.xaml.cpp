#include "pch.h"
#include "DictionarySettingsPage.xaml.h"
#if __has_include("DictionarySettingsPage.g.cpp")
#include "DictionarySettingsPage.g.cpp"
#endif
#include "FilePathProvider.h"
#include "logger.h"
#include <winrt/Windows.Storage.Pickers.h>
#include <winrt/Windows.System.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Microsoft.UI.Dispatching.h>
#include <filesystem>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <iomanip>
#include "CombineData.h" 

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Windows::Storage;
using namespace Windows::Storage::Pickers;
using namespace Windows::Foundation;
using namespace Microsoft::UI::Dispatching;

namespace winrt::WordWiz::implementation
{
	DictionarySettingsPage::DictionarySettingsPage()
    {
        m_dictionaryImporter = std::make_unique<WordWizServices::Dictionary::DictionaryImporter>();
		RefreshDictionaryList();
    }
	
    void DictionarySettingsPage::ImportSingleFileButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
    {
        ImportSingleFileAsync();
    }    
    
    winrt::fire_and_forget DictionarySettingsPage::ImportSingleFileAsync()
    {
        auto strongThis = get_strong();
        
        try {
            // 显示开始导入的Flyout
            strongThis->ShowSingleFileFlyout(L"正在验证文件路径...");
            
            // 从文本框获取文件路径
            auto filePath = winrt::to_string(SingleFilePathTextBox().Text());
              if (filePath.empty()) {
                strongThis->ShowSingleFileFlyout(L"请输入文件路径", false, false);
                co_await winrt::resume_after(std::chrono::seconds(2));
                strongThis->CloseFlyouts();
                co_return;
            }
              // 安全检查：只有在路径长度足够且首尾确实都是引号时才去除
              if (!filePath.empty()) {
                  // 检查是否同时以引号开始和结束
                  if (filePath.size() >= 2 &&
                      filePath[0] == '"' &&
                      filePath[filePath.size() - 1] == '"') {

                      // 去除首尾引号
                      filePath = filePath.substr(1, filePath.size() - 2);
                      WordWizServices::Log::LogMessage(L"检测到文件路径包含引号，已去除引号: " + winrt::to_hstring(filePath));

                      // 验证去除引号后的路径不为空
                      if (filePath.empty()) {
                          WordWizServices::Log::LogMessage(L"导入失败：去除引号后文件路径为空");
                      }
                  }
              }
            // 检查文件是否存在
            std::filesystem::path path(filePath);
            if (!std::filesystem::exists(path)) {
                strongThis->ShowSingleFileFlyout(L"文件不存在，请检查路径是否正确", false, false);
                co_await winrt::resume_after(std::chrono::seconds(3));
                strongThis->CloseFlyouts();
                co_return;
            }

            // 检查文件扩展名
            std::string extension = path.extension().string();
            std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
            if (extension != ".db" && extension != ".sqlite") {
                strongThis->ShowSingleFileFlyout(L"不支持的文件格式，请选择 .db 或 .sqlite 文件", false, false);
                co_await winrt::resume_after(std::chrono::seconds(3));
                strongThis->CloseFlyouts();
                co_return;
            }

            strongThis->ShowSingleFileFlyout(L"开始导入词典文件...");
            std::vector<std::string> filePaths = { filePath };
            strongThis->ImportFilesAsync(filePaths);        
        }
        catch (const std::exception& ex) {
            std::string errorMsg = "处理文件路径失败: " + std::string(ex.what());
            WordWizServices::Log::LogMessage(winrt::to_hstring(errorMsg));
            strongThis->ShowSingleFileFlyout(L"处理文件路径失败", false, false);
            
            // 使用定时器延时关闭Flyout
            auto timer = Microsoft::UI::Xaml::DispatcherTimer();
            timer.Interval(std::chrono::seconds(3));
            timer.Tick([strongThis, timer](auto&&, auto&&) {
                strongThis->CloseFlyouts();
                timer.Stop();
            });
            timer.Start();
        }
        catch (...) {
            WordWizServices::Log::LogMessage(L"处理文件路径时发生未知错误");
            strongThis->ShowSingleFileFlyout(L"处理文件路径失败", false, false);
            
            // 使用定时器延时关闭Flyout
            auto timer = Microsoft::UI::Xaml::DispatcherTimer();
            timer.Interval(std::chrono::seconds(3));
            timer.Tick([strongThis, timer](auto&&, auto&&) {
                strongThis->CloseFlyouts();
                timer.Stop();
            });
            timer.Start();
        }
    }
      
    void DictionarySettingsPage::ImportMultipleFilesButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
    {
        ImportMultipleFilesAsync();
    }    
      
      winrt::fire_and_forget DictionarySettingsPage::ImportMultipleFilesAsync()
    {
        auto strongThis = get_strong();
        
        try {
            // 从文本框获取多个文件路径
            auto pathsText = winrt::to_string(MultipleFilePathsTextBox().Text());
              if (pathsText.empty()) {
                strongThis->ShowMultipleFilesFlyout(L"请输入文件路径", L"", false, false);
                co_await winrt::resume_after(std::chrono::seconds(2));
                strongThis->CloseFlyouts();
                co_return;
            }

            strongThis->ShowMultipleFilesFlyout(L"正在验证文件路径...", L"检查输入的文件路径");

            // 按行分割路径
            std::vector<std::string> filePaths;
            std::istringstream iss(pathsText);
            std::string line;
            int lineNumber = 0;
            
            while (std::getline(iss, line)) {
                lineNumber++;
                // 去除首尾空白字符
                line.erase(0, line.find_first_not_of(" \t\r\n"));
                line.erase(line.find_last_not_of(" \t\r\n") + 1);
                
                if (!line.empty()) {
                    strongThis->ShowMultipleFilesFlyout(L"正在验证文件路径...", 
                        L"检查第 " + winrt::to_hstring(lineNumber) + L" 行: " + winrt::to_hstring(line));
                    
                    // 检查文件是否存在
                    std::filesystem::path path(line);                    if (!std::filesystem::exists(path)) {
                        strongThis->ShowMultipleFilesFlyout(L"文件不存在", L"第 " + winrt::to_hstring(lineNumber) + L" 行: " + winrt::to_hstring(line), false, false);
                        co_await winrt::resume_after(std::chrono::seconds(3));
                        strongThis->CloseFlyouts();
                        co_return;
                    }

                    // 检查文件扩展名
                    std::string extension = path.extension().string();
                    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);                    if (extension != ".db" && extension != ".sqlite") {
                        strongThis->ShowMultipleFilesFlyout(L"不支持的文件格式", L"第 " + winrt::to_hstring(lineNumber) + L" 行: " + winrt::to_hstring(line) + L" (请选择 .db 或 .sqlite 文件)", false, false);
                        co_await winrt::resume_after(std::chrono::seconds(3));
                        strongThis->CloseFlyouts();
                        co_return;
                    }

                    filePaths.push_back(line);
                }
            }            if (filePaths.empty()) {
                strongThis->ShowMultipleFilesFlyout(L"没有找到有效的文件路径", L"", false, false);
                co_await winrt::resume_after(std::chrono::seconds(2));
                strongThis->CloseFlyouts();
                co_return;
            }

            strongThis->ShowMultipleFilesFlyout(L"开始导入词典文件...", L"找到 " + winrt::to_hstring(filePaths.size()) + L" 个有效文件");
            strongThis->ImportFilesAsync(filePaths);        
        }
        catch (const std::exception& ex) {
            std::string errorMsg = "处理文件路径失败: " + std::string(ex.what());
            WordWizServices::Log::LogMessage(winrt::to_hstring(errorMsg));
            strongThis->ShowMultipleFilesFlyout(L"处理文件路径失败", L"", false, false);
            
            // 使用定时器延时关闭Flyout
            auto timer = Microsoft::UI::Xaml::DispatcherTimer();
            timer.Interval(std::chrono::seconds(3));
            timer.Tick([strongThis, timer](auto&&, auto&&) {
                strongThis->CloseFlyouts();
                timer.Stop();
            });
            timer.Start();
        }
        catch (...) {
            WordWizServices::Log::LogMessage(L"处理文件路径时发生未知错误");
            strongThis->ShowMultipleFilesFlyout(L"处理文件路径失败", L"", false, false);
            
            // 使用定时器延时关闭Flyout
            auto timer = Microsoft::UI::Xaml::DispatcherTimer();
            timer.Interval(std::chrono::seconds(3));
            timer.Tick([strongThis, timer](auto&&, auto&&) {
                strongThis->CloseFlyouts();
                timer.Stop();
            });
            timer.Start();
        }
    }    
      
    void DictionarySettingsPage::OpenDictionariesFolderButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
    {
        try {
            ShowOpenFolderFlyout(L"正在打开词典文件夹...");
            
            std::string dictionariesPath = m_dictionaryImporter->GetDictionariesFolderPath();
            if (!dictionariesPath.empty()) {
                // 确保目录存在
                m_dictionaryImporter->EnsureDictionariesFolderExists();
                OpenFolderInExplorer(dictionariesPath);
                
                ShowOpenFolderFlyout(L"词典文件夹已打开", true);
                
                // 延时关闭Flyout
                auto strongThis = get_strong();
                auto timer = Microsoft::UI::Xaml::DispatcherTimer();
                timer.Interval(std::chrono::seconds(2));
                timer.Tick([strongThis, timer](auto&&, auto&&) {
                    strongThis->CloseFlyouts();
                    timer.Stop();
                });
                timer.Start();
            } else {
                ShowOpenFolderFlyout(L"无法获取词典文件夹路径", false);
                WordWizServices::Log::LogMessage(L"无法获取词典文件夹路径");
            }
        }
        catch (...) {
            ShowOpenFolderFlyout(L"打开词典文件夹失败", false);
            WordWizServices::Log::LogMessage(L"打开词典文件夹失败");
        }
    }

    void DictionarySettingsPage::RefreshDictionaryListButton_Click(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&)
    {
        RefreshDictionaryList();
    }

    void DictionarySettingsPage::DictionarySortComboBox_SelectionChanged(
        winrt::Windows::Foundation::IInspectable const&,
        winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const&)
    {
        RefreshDictionaryList();
    }

    void DictionarySettingsPage::ShowImportStatus(const winrt::hstring& message, bool isVisible)
    {
        ImportStatusText().Text(message);
        ImportStatusPanel().Visibility(isVisible ? Visibility::Visible : Visibility::Collapsed);
    }

    void DictionarySettingsPage::HideImportStatus()
    {
        ImportStatusPanel().Visibility(Visibility::Collapsed);
    }

    void DictionarySettingsPage::OpenFolderInExplorer(const std::string& folderPath)
    {
        try {
            auto uri = winrt::Windows::Foundation::Uri(L"file:///" + winrt::to_hstring(folderPath));
            winrt::Windows::System::Launcher::LaunchUriAsync(uri);
        }
        catch (...) {
            WordWizServices::Log::LogMessage(L"打开文件夹失败: " + winrt::to_hstring(folderPath));
        }
    }

    winrt::fire_and_forget DictionarySettingsPage::ImportFilesAsync(std::vector<std::string> filePaths)
    {
        // 检查 main.db 是否存在，不存在则重建，抛弃先前全部扫描
        try {
            std::string mainDbPath = WordWizServices::Data::FilePathProvider::GetAppLocalFolderPath() + "\\main.db";
            std::string dictFolderPath = WordWizServices::Data::FilePathProvider::GetAppLocalFolderPath() + "\\Dictionaries";
            if (!std::filesystem::exists(mainDbPath)) {
                WordWizServices::Log::LogMessage(L"main.db 不存在，自动重建...");
                combine_all_dictionaries_to_main_db(mainDbPath, dictFolderPath);
                WordWizServices::Log::LogMessage(L"main.db 重建完成。");
            }
        } catch (const std::exception& e) {
            WordWizServices::Log::LogMessage(L"检测或重建 main.db 时发生异常: " + winrt::to_hstring(e.what()));
        }

        ShowImportStatus(L"正在导入词典文件...");
        
        // 获取强引用，确保对象生命周期
        auto strongThis = get_strong();
        
        try {
            // 在后台线程执行导入
            co_await winrt::resume_background();
            
            // 异步导入文件
            auto future = strongThis->m_dictionaryImporter->ImportDictionariesAsync(filePaths);
            auto results = future.get();
              // 简化的UI更新方式 - 使用调度队列调用
            strongThis->DispatcherQueue().TryEnqueue([strongThis, results, filePaths]() {
                try {
                    // 统计结果
                    int successCount = 0;
                    int totalCount = results.size();
                    
                    // 详细记录每个导入结果
                    WordWizServices::Log::LogMessage(L"导入结果详细信息：");
                    for (int i = 0; i < results.size(); ++i) {
                        bool result = results[i];
                        if (result) successCount++;
                        
                        WordWizServices::Log::LogMessage(L"文件 " + winrt::to_hstring(i + 1) + L": " + 
                            winrt::to_hstring(filePaths[i]) + L" -> " + 
                            (result ? L"成功" : L"失败"));
                    }
                    
                    WordWizServices::Log::LogMessage(L"导入统计：成功 " + winrt::to_hstring(successCount) + 
                                                    L"/" + winrt::to_hstring(totalCount) + L" 个文件");
                    
                    // 导入完成后，融合所有词典到主库
                    try {
                        std::string mainDbPath = WordWizServices::Data::FilePathProvider::GetAppLocalFolderPath() + "\\main.db";
                        combine_new_dictionaries_to_main_db(mainDbPath, filePaths);
                        WordWizServices::Log::LogMessage(L"词典融合到主库完成。");
                    } catch (const std::exception& e) {
                        WordWizServices::Log::LogMessage(L"融合词典到主库时发生异常: " + winrt::to_hstring(e.what()));
                    }

                    // 显示结果
                    winrt::hstring statusMessage;
                    winrt::hstring detailMessage;

                    bool isSuccess = false;
                    if (successCount == totalCount) isSuccess = true;

                    
                    if (isSuccess) {
                        statusMessage = L"导入完成！";
                        detailMessage = L"成功导入 " + winrt::to_hstring(successCount) + L" 个词典文件";
                    } else {
                        statusMessage = L"导入失败";
                        detailMessage = L"成功：" + winrt::to_hstring(successCount) + L"/" + winrt::to_hstring(totalCount) + L" 个文件";
                    }
                    
                    // 根据文件数量选择合适的Flyout
                    if (filePaths.size() == 1) {
                        strongThis->ShowSingleFileFlyout(statusMessage, false, isSuccess);
                    } else {
                        strongThis->ShowMultipleFilesFlyout(statusMessage, detailMessage, false, isSuccess);
                    }
                    strongThis->RefreshDictionaryList();
                    
                    strongThis->ShowImportStatus(statusMessage + L" " + detailMessage);
                    
                    // 使用定时器在3秒后隐藏状态
                    auto timer = Microsoft::UI::Xaml::DispatcherTimer();
                    timer.Interval(std::chrono::seconds(3));
                    timer.Tick([strongThis, timer](auto&&, auto&&) {
                        strongThis->HideImportStatus();
                        strongThis->CloseFlyouts();
                        timer.Stop();
                    });
                    timer.Start();
                } catch (...) {
                    // UI线程中的异常处理
                    strongThis->ShowImportStatus(L"导入过程中发生错误");
                    if (filePaths.size() == 1) {
                        strongThis->ShowSingleFileFlyout(L"导入失败", false, false);
                    } else {
                        strongThis->ShowMultipleFilesFlyout(L"导入失败", L"发生未知错误", false, false);
                    }
                    auto timer = Microsoft::UI::Xaml::DispatcherTimer();
                    timer.Interval(std::chrono::seconds(3));
                    timer.Tick([strongThis, timer](auto&&, auto&&) {
                        strongThis->HideImportStatus();
                        strongThis->CloseFlyouts();
                        timer.Stop();
                    });
                    timer.Start();
                }
            });
        }
        catch (...) {
            // 后台线程异常处理
            strongThis->DispatcherQueue().TryEnqueue([strongThis, filePaths]() {
                strongThis->ShowImportStatus(L"导入过程中发生未知错误");
                if (filePaths.size() == 1) {
                    strongThis->ShowSingleFileFlyout(L"导入失败", false, false);
                } else {
                    strongThis->ShowMultipleFilesFlyout(L"导入失败", L"发生未知错误", false, false);
                }
                auto timer = Microsoft::UI::Xaml::DispatcherTimer();
                timer.Interval(std::chrono::seconds(3));
                timer.Tick([strongThis, timer](auto&&, auto&&) {
                    strongThis->HideImportStatus();
                    strongThis->CloseFlyouts();
                    timer.Stop();
                });
                timer.Start();
            });
        }
    }    // Flyout 状态控制方法
    void DictionarySettingsPage::ShowSingleFileFlyout(const winrt::hstring& message, bool showProgress, bool isSuccess)
    {
        try {
            SingleFileStatusText().Text(message);
            
            // 设置图标
            if (showProgress) {
                SingleFileStatusIcon().Glyph(L"\uE946"); // 进度图标
                auto blueBrush = Microsoft::UI::Xaml::Media::SolidColorBrush(winrt::Windows::UI::ColorHelper::FromArgb(255, 0, 120, 215));
                SingleFileStatusIcon().Foreground(blueBrush);
                SingleFileProgressBar().Visibility(Microsoft::UI::Xaml::Visibility::Visible);
            } else {
                if (isSuccess) {
                    SingleFileStatusIcon().Glyph(L"\uE73E"); // 成功图标
                    auto greenBrush = Microsoft::UI::Xaml::Media::SolidColorBrush(winrt::Windows::UI::ColorHelper::FromArgb(255, 16, 124, 16));
                    SingleFileStatusIcon().Foreground(greenBrush);
                } else {
                    SingleFileStatusIcon().Glyph(L"\uE783"); // 错误图标  
                    auto redBrush = Microsoft::UI::Xaml::Media::SolidColorBrush(winrt::Windows::UI::ColorHelper::FromArgb(255, 196, 43, 28));
                    SingleFileStatusIcon().Foreground(redBrush);
                }
                SingleFileProgressBar().Visibility(Microsoft::UI::Xaml::Visibility::Collapsed);
            }
            
            // 显示Flyout
            SingleFileImportFlyout().ShowAt(ImportSingleFileButton());
        }
        catch (...) {
            WordWizServices::Log::LogMessage(L"显示单文件Flyout失败");
        }
    }    void DictionarySettingsPage::ShowMultipleFilesFlyout(const winrt::hstring& message, const winrt::hstring& detail, bool showProgress, bool isSuccess)
    {
        try {
            MultipleFilesStatusText().Text(message);
            MultipleFilesDetailText().Text(detail);
            
            // 设置图标
            if (showProgress) {
                MultipleFilesStatusIcon().Glyph(L"\uE946"); // 进度图标
                auto blueBrush = Microsoft::UI::Xaml::Media::SolidColorBrush(winrt::Windows::UI::ColorHelper::FromArgb(255, 0, 120, 215));
                MultipleFilesStatusIcon().Foreground(blueBrush);
                MultipleFilesProgressBar().Visibility(Microsoft::UI::Xaml::Visibility::Visible);
            } else {
                if (isSuccess) {
                    MultipleFilesStatusIcon().Glyph(L"\uE73E"); // 成功图标
                    auto greenBrush = Microsoft::UI::Xaml::Media::SolidColorBrush(winrt::Windows::UI::ColorHelper::FromArgb(255, 16, 124, 16));
                    MultipleFilesStatusIcon().Foreground(greenBrush);
                } else {
                    MultipleFilesStatusIcon().Glyph(L"\uE783"); // 错误图标
                    auto redBrush = Microsoft::UI::Xaml::Media::SolidColorBrush(winrt::Windows::UI::ColorHelper::FromArgb(255, 196, 43, 28));
                    MultipleFilesStatusIcon().Foreground(redBrush);
                }
                MultipleFilesProgressBar().Visibility(Microsoft::UI::Xaml::Visibility::Collapsed);
            }
            
            // 显示Flyout
            MultipleFilesImportFlyout().ShowAt(ImportMultipleFilesButton());
        }
        catch (...) {
            WordWizServices::Log::LogMessage(L"显示多文件Flyout失败");
        }
    }

    void DictionarySettingsPage::ShowOpenFolderFlyout(const winrt::hstring& message, bool isSuccess)
    {
        try {
            OpenFolderStatusText().Text(message);
            
            // 设置图标
            if (isSuccess) {
                OpenFolderStatusIcon().Glyph(L"\uE73E"); // 成功图标
                auto greenBrush = Microsoft::UI::Xaml::Media::SolidColorBrush(winrt::Windows::UI::ColorHelper::FromArgb(255, 16, 124, 16));
                OpenFolderStatusIcon().Foreground(greenBrush);
            } else {
                OpenFolderStatusIcon().Glyph(L"\uE838"); // 文件夹图标
                auto blueBrush = Microsoft::UI::Xaml::Media::SolidColorBrush(winrt::Windows::UI::ColorHelper::FromArgb(255, 0, 120, 215));
                OpenFolderStatusIcon().Foreground(blueBrush);
            }
            
            // 显示Flyout
            OpenFolderFlyout().ShowAt(OpenDictionariesFolderButton());
        }
        catch (...) {
            WordWizServices::Log::LogMessage(L"显示打开文件夹Flyout失败");
        }
    }

    void DictionarySettingsPage::CloseFlyouts()
    {
        try {
            if (SingleFileImportFlyout().IsOpen()) {
                SingleFileImportFlyout().Hide();
            }
            if (MultipleFilesImportFlyout().IsOpen()) {
                MultipleFilesImportFlyout().Hide();
            }
            if (OpenFolderFlyout().IsOpen()) {
                OpenFolderFlyout().Hide();
            }
        }
        catch (...) {
            WordWizServices::Log::LogMessage(L"关闭Flyout失败");
        }
    }

    void DictionarySettingsPage::RefreshDictionaryList()
    {
        auto items = winrt::single_threaded_observable_vector<winrt::IInspectable>();
        try
        {
            const auto folder = m_dictionaryImporter->GetDictionariesFolderPath();
            m_dictionaryImporter->EnsureDictionariesFolderExists();

            struct FileInfo
            {
                std::string name;
                std::uintmax_t size{ 0 };
                std::filesystem::file_time_type modified;
            };

            std::vector<FileInfo> files;
            for (const auto& entry : std::filesystem::directory_iterator(folder))
            {
                if (!entry.is_regular_file()) continue;
                auto ext = entry.path().extension().string();
                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                if (ext == ".db" || ext == ".sqlite")
                {
                    files.push_back({ entry.path().filename().string(), entry.file_size(), entry.last_write_time() });
                }
            }

            const int sortMode = DictionarySortComboBox() ? DictionarySortComboBox().SelectedIndex() : 0;
            std::sort(files.begin(), files.end(), [sortMode](const FileInfo& a, const FileInfo& b)
            {
                switch (sortMode)
                {
                case 1: return a.name > b.name;
                case 2: return a.modified > b.modified;
                case 3: return a.size > b.size;
                default: return a.name < b.name;
                }
            });

            for (const auto& f : files)
            {
                std::wstringstream ss;
                ss << winrt::to_hstring(f.name).c_str() << L"    (" << (f.size / 1024) << L" KB)";
                items.Append(winrt::box_value(winrt::hstring(ss.str())));
            }

            DictionaryListView().ItemsSource(items);
            DictionaryListStatusText().Text(L"共 " + winrt::to_hstring(files.size()) + L" 个词典文件");
        }
        catch (const std::exception& ex)
        {
            WordWizServices::Log::LogMessage(L"刷新词典列表失败: " + winrt::to_hstring(ex.what()));
            DictionaryListView().ItemsSource(items);
            DictionaryListStatusText().Text(L"刷新词典列表失败");
        }
    }
}
