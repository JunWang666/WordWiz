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
	}

	void DictionarySettingsPage::ImportSingleFileButton_Click(const Windows::Foundation::IInspectable& sender,
	                                                          const RoutedEventArgs& e)
	{
		ImportSingleFileAsync();
	}

	fire_and_forget DictionarySettingsPage::ImportSingleFileAsync()
	{
		auto strongThis = get_strong();

		try
		{
			// 显示开始导入的Flyout
			strongThis->ShowSingleFileFlyout(L"正在验证文件路径...");

			// 从文本框获取文件路径
			auto filePath = to_string(SingleFilePathTextBox().Text());
			if (filePath.empty())
			{
				strongThis->ShowSingleFileFlyout(L"请输入文件路径", false, false);
				co_await resume_after(std::chrono::seconds(2));
				strongThis->CloseFlyouts();
				co_return;
			}
			// 安全检查：只有在路径长度足够且首尾确实都是引号时才去除
			if (!filePath.empty())
			{
				// 检查是否同时以引号开始和结束
				if (filePath.size() >= 2 &&
					filePath[0] == '"' &&
					filePath[filePath.size() - 1] == '"')
				{
					// 去除首尾引号
					filePath = filePath.substr(1, filePath.size() - 2);
					WordWizServices::Log::LogMessage(L"检测到文件路径包含引号，已去除引号: " + to_hstring(filePath));

					// 验证去除引号后的路径不为空
					if (filePath.empty())
					{
						WordWizServices::Log::LogMessage(L"导入失败：去除引号后文件路径为空");
					}
				}
			}
			// 检查文件是否存在
			std::filesystem::path path(filePath);
			if (!std::filesystem::exists(path))
			{
				strongThis->ShowSingleFileFlyout(L"文件不存在，请检查路径是否正确", false, false);
				co_await resume_after(std::chrono::seconds(3));
				strongThis->CloseFlyouts();
				co_return;
			}

			// 检查文件扩展名
			std::string extension = path.extension().string();
			std::transform(extension.begin(), extension.end(), extension.begin(), tolower);
			if (extension != ".db" && extension != ".sqlite")
			{
				strongThis->ShowSingleFileFlyout(L"不支持的文件格式，请选择 .db 或 .sqlite 文件", false, false);
				co_await resume_after(std::chrono::seconds(3));
				strongThis->CloseFlyouts();
				co_return;
			}

			strongThis->ShowSingleFileFlyout(L"开始导入词典文件...");
			std::vector<std::string> filePaths = {filePath};
			strongThis->ImportFilesAsync(filePaths);
		}
		catch (const std::exception& ex)
		{
			std::string errorMsg = "处理文件路径失败: " + std::string(ex.what());
			WordWizServices::Log::LogMessage(to_hstring(errorMsg));
			strongThis->ShowSingleFileFlyout(L"处理文件路径失败", false, false);

			// 使用定时器延时关闭Flyout
			auto timer = DispatcherTimer();
			timer.Interval(std::chrono::seconds(3));
			timer.Tick([strongThis, timer](auto&&, auto&&)
			{
				strongThis->CloseFlyouts();
				timer.Stop();
			});
			timer.Start();
		}
		catch (...)
		{
			WordWizServices::Log::LogMessage(L"处理文件路径时发生未知错误");
			strongThis->ShowSingleFileFlyout(L"处理文件路径失败", false, false);

			// 使用定时器延时关闭Flyout
			auto timer = DispatcherTimer();
			timer.Interval(std::chrono::seconds(3));
			timer.Tick([strongThis, timer](auto&&, auto&&)
			{
				strongThis->CloseFlyouts();
				timer.Stop();
			});
			timer.Start();
		}
	}

	void DictionarySettingsPage::ImportMultipleFilesButton_Click(const Windows::Foundation::IInspectable& sender,
	                                                             const RoutedEventArgs& e)
	{
		ImportMultipleFilesAsync();
	}

	fire_and_forget DictionarySettingsPage::ImportMultipleFilesAsync()
	{
		auto strongThis = get_strong();

		try
		{
			// 从文本框获取多个文件路径
			auto pathsText = to_string(MultipleFilePathsTextBox().Text());
			if (pathsText.empty())
			{
				strongThis->ShowMultipleFilesFlyout(L"请输入文件路径", L"", false, false);
				co_await resume_after(std::chrono::seconds(2));
				strongThis->CloseFlyouts();
				co_return;
			}

			strongThis->ShowMultipleFilesFlyout(L"正在验证文件路径...", L"检查输入的文件路径");

			// 按行分割路径
			std::vector<std::string> filePaths;
			std::istringstream iss(pathsText);
			std::string line;
			int lineNumber = 0;

			while (std::getline(iss, line))
			{
				lineNumber++;
				// 去除首尾空白字符
				line.erase(0, line.find_first_not_of(" \t\r\n"));
				line.erase(line.find_last_not_of(" \t\r\n") + 1);

				if (!line.empty())
				{
					strongThis->ShowMultipleFilesFlyout(L"正在验证文件路径...",
					                                    L"检查第 " + to_hstring(lineNumber) + L" 行: " + to_hstring(line));

					// 检查文件是否存在
					std::filesystem::path path(line);
					if (!std::filesystem::exists(path))
					{
						strongThis->ShowMultipleFilesFlyout(
							L"文件不存在", L"第 " + to_hstring(lineNumber) + L" 行: " + to_hstring(line), false, false);
						co_await resume_after(std::chrono::seconds(3));
						strongThis->CloseFlyouts();
						co_return;
					}

					// 检查文件扩展名
					std::string extension = path.extension().string();
					std::transform(extension.begin(), extension.end(), extension.begin(), tolower);
					if (extension != ".db" && extension != ".sqlite")
					{
						strongThis->ShowMultipleFilesFlyout(L"不支持的文件格式",
						                                    L"第 " + to_hstring(lineNumber) + L" 行: " + to_hstring(line)
						                                    + L" (请选择 .db 或 .sqlite 文件)", false, false);
						co_await resume_after(std::chrono::seconds(3));
						strongThis->CloseFlyouts();
						co_return;
					}

					filePaths.push_back(line);
				}
			}
			if (filePaths.empty())
			{
				strongThis->ShowMultipleFilesFlyout(L"没有找到有效的文件路径", L"", false, false);
				co_await resume_after(std::chrono::seconds(2));
				strongThis->CloseFlyouts();
				co_return;
			}

			strongThis->ShowMultipleFilesFlyout(L"开始导入词典文件...", L"找到 " + to_hstring(filePaths.size()) + L" 个有效文件");
			strongThis->ImportFilesAsync(filePaths);
		}
		catch (const std::exception& ex)
		{
			std::string errorMsg = "处理文件路径失败: " + std::string(ex.what());
			WordWizServices::Log::LogMessage(to_hstring(errorMsg));
			strongThis->ShowMultipleFilesFlyout(L"处理文件路径失败", L"", false, false);

			// 使用定时器延时关闭Flyout
			auto timer = DispatcherTimer();
			timer.Interval(std::chrono::seconds(3));
			timer.Tick([strongThis, timer](auto&&, auto&&)
			{
				strongThis->CloseFlyouts();
				timer.Stop();
			});
			timer.Start();
		}
		catch (...)
		{
			WordWizServices::Log::LogMessage(L"处理文件路径时发生未知错误");
			strongThis->ShowMultipleFilesFlyout(L"处理文件路径失败", L"", false, false);

			// 使用定时器延时关闭Flyout
			auto timer = DispatcherTimer();
			timer.Interval(std::chrono::seconds(3));
			timer.Tick([strongThis, timer](auto&&, auto&&)
			{
				strongThis->CloseFlyouts();
				timer.Stop();
			});
			timer.Start();
		}
	}

	void DictionarySettingsPage::OpenDictionariesFolderButton_Click(const Windows::Foundation::IInspectable& sender,
	                                                                const RoutedEventArgs& e)
	{
		try
		{
			ShowOpenFolderFlyout(L"正在打开词典文件夹...");

			std::string dictionariesPath = m_dictionaryImporter->GetDictionariesFolderPath();
			if (!dictionariesPath.empty())
			{
				// 确保目录存在
				m_dictionaryImporter->EnsureDictionariesFolderExists();
				OpenFolderInExplorer(dictionariesPath);

				ShowOpenFolderFlyout(L"词典文件夹已打开", true);

				// 延时关闭Flyout
				auto strongThis = get_strong();
				auto timer = DispatcherTimer();
				timer.Interval(std::chrono::seconds(2));
				timer.Tick([strongThis, timer](auto&&, auto&&)
				{
					strongThis->CloseFlyouts();
					timer.Stop();
				});
				timer.Start();
			}
			else
			{
				ShowOpenFolderFlyout(L"无法获取词典文件夹路径", false);
				WordWizServices::Log::LogMessage(L"无法获取词典文件夹路径");
			}
		}
		catch (...)
		{
			ShowOpenFolderFlyout(L"打开词典文件夹失败", false);
			WordWizServices::Log::LogMessage(L"打开词典文件夹失败");
		}
	}

	void DictionarySettingsPage::ShowImportStatus(const hstring& message, bool isVisible)
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
		try
		{
			auto uri = Uri(L"file:///" + to_hstring(folderPath));
			Windows::System::Launcher::LaunchUriAsync(uri);
		}
		catch (...)
		{
			WordWizServices::Log::LogMessage(L"打开文件夹失败: " + to_hstring(folderPath));
		}
	}

	fire_and_forget DictionarySettingsPage::ImportFilesAsync(std::vector<std::string> filePaths)
	{
		ShowImportStatus(L"正在导入词典文件...");

		// 获取强引用，确保对象生命周期
		auto strongThis = get_strong();

		try
		{
			// 在后台线程执行导入
			co_await resume_background();

			// 异步导入文件
			auto future = strongThis->m_dictionaryImporter->ImportDictionariesAsync(filePaths);
			auto results = future.get();
			// 简化的UI更新方式 - 使用调度队列调用
			strongThis->DispatcherQueue().TryEnqueue([strongThis, results, filePaths]()
			{
				try
				{
					// 统计结果
					int successCount = 0;
					int totalCount = results.size();

					// 详细记录每个导入结果
					WordWizServices::Log::LogMessage(L"导入结果详细信息：");
					for (int i = 0; i < results.size(); ++i)
					{
						bool result = results[i];
						if (result) successCount++;

						WordWizServices::Log::LogMessage(L"文件 " + to_hstring(i + 1) + L": " +
							to_hstring(filePaths[i]) + L" -> " +
							(result ? L"成功" : L"失败"));
					}

					WordWizServices::Log::LogMessage(L"导入统计：成功 " + to_hstring(successCount) +
						L"/" + to_hstring(totalCount) + L" 个文件");

					// 显示结果
					hstring statusMessage;
					hstring detailMessage;

					bool isSuccess = false;
					if (successCount == totalCount) isSuccess = true;


					if (isSuccess)
					{
						statusMessage = L"导入完成！";
						detailMessage = L"成功导入 " + to_hstring(successCount) + L" 个词典文件";
					}
					else
					{
						statusMessage = L"导入失败";
						detailMessage = L"成功：" + to_hstring(successCount) + L"/" + to_hstring(totalCount) + L" 个文件";
					}

					// 根据文件数量选择合适的Flyout
					if (filePaths.size() == 1)
					{
						strongThis->ShowSingleFileFlyout(statusMessage, false, isSuccess);
					}
					else
					{
						strongThis->ShowMultipleFilesFlyout(statusMessage, detailMessage, false, isSuccess);
					}

					strongThis->ShowImportStatus(statusMessage + L" " + detailMessage);

					// 使用定时器在3秒后隐藏状态
					auto timer = DispatcherTimer();
					timer.Interval(std::chrono::seconds(3));
					timer.Tick([strongThis, timer](auto&&, auto&&)
					{
						strongThis->HideImportStatus();
						strongThis->CloseFlyouts();
						timer.Stop();
					});
					timer.Start();
				}
				catch (...)
				{
					// UI线程中的异常处理
					strongThis->ShowImportStatus(L"导入过程中发生错误");
					if (filePaths.size() == 1)
					{
						strongThis->ShowSingleFileFlyout(L"导入失败", false, false);
					}
					else
					{
						strongThis->ShowMultipleFilesFlyout(L"导入失败", L"发生未知错误", false, false);
					}
					auto timer = DispatcherTimer();
					timer.Interval(std::chrono::seconds(3));
					timer.Tick([strongThis, timer](auto&&, auto&&)
					{
						strongThis->HideImportStatus();
						strongThis->CloseFlyouts();
						timer.Stop();
					});
					timer.Start();
				}
			});
		}
		catch (...)
		{
			// 后台线程异常处理
			strongThis->DispatcherQueue().TryEnqueue([strongThis, filePaths]()
			{
				strongThis->ShowImportStatus(L"导入过程中发生未知错误");
				if (filePaths.size() == 1)
				{
					strongThis->ShowSingleFileFlyout(L"导入失败", false, false);
				}
				else
				{
					strongThis->ShowMultipleFilesFlyout(L"导入失败", L"发生未知错误", false, false);
				}
				auto timer = DispatcherTimer();
				timer.Interval(std::chrono::seconds(3));
				timer.Tick([strongThis, timer](auto&&, auto&&)
				{
					strongThis->HideImportStatus();
					strongThis->CloseFlyouts();
					timer.Stop();
				});
				timer.Start();
			});
		}
	} // Flyout 状态控制方法
	void DictionarySettingsPage::ShowSingleFileFlyout(const hstring& message, bool showProgress, bool isSuccess)
	{
		try
		{
			SingleFileStatusText().Text(message);

			// 设置图标
			if (showProgress)
			{
				SingleFileStatusIcon().Glyph(L"\uE946"); // 进度图标
				auto blueBrush = Media::SolidColorBrush(Windows::UI::ColorHelper::FromArgb(255, 0, 120, 215));
				SingleFileStatusIcon().Foreground(blueBrush);
				SingleFileProgressBar().Visibility(Visibility::Visible);
			}
			else
			{
				if (isSuccess)
				{
					SingleFileStatusIcon().Glyph(L"\uE73E"); // 成功图标
					auto greenBrush = Media::SolidColorBrush(Windows::UI::ColorHelper::FromArgb(255, 16, 124, 16));
					SingleFileStatusIcon().Foreground(greenBrush);
				}
				else
				{
					SingleFileStatusIcon().Glyph(L"\uE783"); // 错误图标  
					auto redBrush = Media::SolidColorBrush(Windows::UI::ColorHelper::FromArgb(255, 196, 43, 28));
					SingleFileStatusIcon().Foreground(redBrush);
				}
				SingleFileProgressBar().Visibility(Visibility::Collapsed);
			}

			// 显示Flyout
			SingleFileImportFlyout().ShowAt(ImportSingleFileButton());
		}
		catch (...)
		{
			WordWizServices::Log::LogMessage(L"显示单文件Flyout失败");
		}
	}

	void DictionarySettingsPage::ShowMultipleFilesFlyout(const hstring& message, const hstring& detail,
	                                                     bool showProgress, bool isSuccess)
	{
		try
		{
			MultipleFilesStatusText().Text(message);
			MultipleFilesDetailText().Text(detail);

			// 设置图标
			if (showProgress)
			{
				MultipleFilesStatusIcon().Glyph(L"\uE946"); // 进度图标
				auto blueBrush = Media::SolidColorBrush(Windows::UI::ColorHelper::FromArgb(255, 0, 120, 215));
				MultipleFilesStatusIcon().Foreground(blueBrush);
				MultipleFilesProgressBar().Visibility(Visibility::Visible);
			}
			else
			{
				if (isSuccess)
				{
					MultipleFilesStatusIcon().Glyph(L"\uE73E"); // 成功图标
					auto greenBrush = Media::SolidColorBrush(Windows::UI::ColorHelper::FromArgb(255, 16, 124, 16));
					MultipleFilesStatusIcon().Foreground(greenBrush);
				}
				else
				{
					MultipleFilesStatusIcon().Glyph(L"\uE783"); // 错误图标
					auto redBrush = Media::SolidColorBrush(Windows::UI::ColorHelper::FromArgb(255, 196, 43, 28));
					MultipleFilesStatusIcon().Foreground(redBrush);
				}
				MultipleFilesProgressBar().Visibility(Visibility::Collapsed);
			}

			// 显示Flyout
			MultipleFilesImportFlyout().ShowAt(ImportMultipleFilesButton());
		}
		catch (...)
		{
			WordWizServices::Log::LogMessage(L"显示多文件Flyout失败");
		}
	}

	void DictionarySettingsPage::ShowOpenFolderFlyout(const hstring& message, bool isSuccess)
	{
		try
		{
			OpenFolderStatusText().Text(message);

			// 设置图标
			if (isSuccess)
			{
				OpenFolderStatusIcon().Glyph(L"\uE73E"); // 成功图标
				auto greenBrush = Media::SolidColorBrush(Windows::UI::ColorHelper::FromArgb(255, 16, 124, 16));
				OpenFolderStatusIcon().Foreground(greenBrush);
			}
			else
			{
				OpenFolderStatusIcon().Glyph(L"\uE838"); // 文件夹图标
				auto blueBrush = Media::SolidColorBrush(Windows::UI::ColorHelper::FromArgb(255, 0, 120, 215));
				OpenFolderStatusIcon().Foreground(blueBrush);
			}

			// 显示Flyout
			OpenFolderFlyout().ShowAt(OpenDictionariesFolderButton());
		}
		catch (...)
		{
			WordWizServices::Log::LogMessage(L"显示打开文件夹Flyout失败");
		}
	}

	void DictionarySettingsPage::CloseFlyouts()
	{
		try
		{
			if (SingleFileImportFlyout().IsOpen())
			{
				SingleFileImportFlyout().Hide();
			}
			if (MultipleFilesImportFlyout().IsOpen())
			{
				MultipleFilesImportFlyout().Hide();
			}
			if (OpenFolderFlyout().IsOpen())
			{
				OpenFolderFlyout().Hide();
			}
		}
		catch (...)
		{
			WordWizServices::Log::LogMessage(L"关闭Flyout失败");
		}
	}
}
