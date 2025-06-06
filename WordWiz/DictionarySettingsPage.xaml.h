#pragma once

#include "DictionarySettingsPage.g.h"
#include "DictionaryImpoter.h"
#include <memory>
#include <vector>
#include <winrt/Windows.Foundation.h>

namespace winrt::WordWiz::implementation
{
	struct DictionarySettingsPage : DictionarySettingsPageT<DictionarySettingsPage>
	{
		DictionarySettingsPage();

		// 词典导入事件处理程序
		void ImportSingleFileButton_Click(const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e);
		void ImportMultipleFilesButton_Click(const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e);
		void OpenDictionariesFolderButton_Click(const Windows::Foundation::IInspectable& sender,
		                                        const RoutedEventArgs& e);

	private:
		std::unique_ptr<WordWizServices::Dictionary::DictionaryImporter> m_dictionaryImporter;

		// 辅助方法
		void ShowImportStatus(const hstring& message, bool isVisible = true);
		void HideImportStatus();
		// Flyout 状态更新方法
		void ShowSingleFileFlyout(const hstring& message, bool showProgress = true, bool isSuccess = false);
		void ShowMultipleFilesFlyout(const hstring& message, const hstring& detail = L"", bool showProgress = true,
		                             bool isSuccess = false);
		void ShowOpenFolderFlyout(const hstring& message, bool isSuccess = false);
		void CloseFlyouts();
		fire_and_forget ImportSingleFileAsync();
		fire_and_forget ImportMultipleFilesAsync();
		fire_and_forget ImportFilesAsync(std::vector<std::string> filePaths);
		void OpenFolderInExplorer(const std::string& folderPath);
		HWND GetCurrentWindowHandle();
	};
}

namespace winrt::WordWiz::factory_implementation
{
	struct DictionarySettingsPage : DictionarySettingsPageT<
			DictionarySettingsPage, implementation::DictionarySettingsPage>
	{
	};
}
