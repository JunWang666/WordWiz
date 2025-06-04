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
        void ImportSingleFileButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void ImportMultipleFilesButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void OpenDictionariesFolderButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);    private:
        std::unique_ptr<WordWizServices::Dictionary::DictionaryImporter> m_dictionaryImporter;
        
        // 辅助方法
        void ShowImportStatus(const winrt::hstring& message, bool isVisible = true);
        void HideImportStatus();
        // Flyout 状态更新方法
        void ShowSingleFileFlyout(const winrt::hstring& message, bool showProgress = true, bool isSuccess = false);
        void ShowMultipleFilesFlyout(const winrt::hstring& message, const winrt::hstring& detail = L"", bool showProgress = true, bool isSuccess = false);
        void ShowOpenFolderFlyout(const winrt::hstring& message, bool isSuccess = false);
        void CloseFlyouts();
        winrt::fire_and_forget ImportSingleFileAsync();
        winrt::fire_and_forget ImportMultipleFilesAsync();
        winrt::fire_and_forget ImportFilesAsync(std::vector<std::string> filePaths);
        void OpenFolderInExplorer(const std::string& folderPath);
        HWND GetCurrentWindowHandle();
    };
}

namespace winrt::WordWiz::factory_implementation
{
    struct DictionarySettingsPage : DictionarySettingsPageT<DictionarySettingsPage, implementation::DictionarySettingsPage>
    {
    };
}
