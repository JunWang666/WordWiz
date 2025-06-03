#include "pch.h"
#include "GeneralSettingsPage.xaml.h"
#if __has_include("GeneralSettingsPage.g.cpp")
#include "GeneralSettingsPage.g.cpp"
#endif
#include "SettingsData.h"
#include "App.xaml.h"
#include "FilePathProvider.h"
#include <winrt/Windows.UI.ViewManagement.h>
#include <winrt/Windows.System.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.ApplicationModel.h> // Required for Package Version
#include <cstdlib>
#include <sstream> // Required for std::wstringstream

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;

namespace winrt::WordWiz::implementation
{
    GeneralSettingsPage::GeneralSettingsPage()
    {
        InitializeComponent();
        LoadCurrentTheme();
        LoadFolderPaths();
        LoadAppVersion(); // Call LoadAppVersion
    }

    void GeneralSettingsPage::ThemeRadio_Checked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
    {
        auto radio = sender.as<winrt::Microsoft::UI::Xaml::Controls::RadioButton>();
        auto tag = winrt::unbox_value<winrt::hstring>(radio.Tag());
        std::string themeMode = winrt::to_string(tag);
        
        // 保存主题设置到数据库
        try {
            ::WordWiz::Data::SettingsManager settingsManager("app_settings");
            if (settingsManager.isInitialized()) {
                settingsManager.setString("theme_mode", themeMode);
                
                // 立即应用主题
                ApplyTheme(themeMode);
            }
        }
        catch (const std::exception& e) {
            // 处理错误
        }
    }

    void GeneralSettingsPage::LoadCurrentTheme()
    {
        try {
            ::WordWiz::Data::SettingsManager settingsManager("app_settings");
            if (settingsManager.isInitialized()) {
                std::string themeMode = settingsManager.getString("theme_mode", "light");
                
                // 设置相应的RadioButton状态
                if (themeMode == "dark") {
                    DarkThemeRadio().IsChecked(true);
                } else if (themeMode == "system") {
                    SystemThemeRadio().IsChecked(true);
                } else {
                    LightThemeRadio().IsChecked(true);
                }
                // 应用主题到应用程序
                ApplyTheme(themeMode);
            }
        }
        catch (const std::exception& e) {
            // 使用默认主题
            LightThemeRadio().IsChecked(true);
            ApplyTheme("light");
        }
    }

    void GeneralSettingsPage::ApplyTheme(const std::string& themeMode)
    {
        bool isDarkMode = false;
        
        if (themeMode == "dark") {
            isDarkMode = true;
        } else if (themeMode == "system") {
            isDarkMode = IsSystemInDarkMode();
        } else {
            isDarkMode = false; // light mode
        }
        
        // 使用App类的全局主题设置方法
        try {
            ::winrt::WordWiz::implementation::App::SetGlobalTheme(isDarkMode);
        }
        catch (...) {
            // 忽略主题设置错误
        }    
    }

    bool GeneralSettingsPage::IsSystemInDarkMode()
    {
        try {
            // 检查系统主题设置
            auto uiSettings = winrt::Windows::UI::ViewManagement::UISettings();
            auto color = uiSettings.GetColorValue(winrt::Windows::UI::ViewManagement::UIColorType::Background);
            
            // 如果背景色比较暗，则认为是深色模式
            // RGB值越小表示越暗
            return (color.R + color.G + color.B) < (255 * 3 / 2);
        }
        catch (...) {
            // 如果无法检测，默认使用浅色模式
            return false;
        }
    }

    void GeneralSettingsPage::LoadFolderPaths()
    {
        try {
            // 加载本地数据文件夹路径
            std::string localPath = ::WordWizServices::Data::FilePathProvider::GetAppLocalFolderPath();
            if (localPath.empty()) {
                LocalFolderPathText().Text(L"无法获取路径");
                OpenLocalFolderButton().IsEnabled(false); // 假设按钮名为 OpenLocalFolderButton
            }
            else {
                LocalFolderPathText().Text(winrt::to_hstring(localPath));
                OpenLocalFolderButton().IsEnabled(true);
            }

            // 加载本地缓存文件夹路径
            std::string cachePath = ::WordWizServices::Data::FilePathProvider::GetAppLocalCacheFolderPath();
            if (cachePath.empty()) {
                LocalCacheFolderPathText().Text(L"无法获取路径");
                OpenLocalCacheFolderButton().IsEnabled(false); // 假设按钮名为 OpenCacheFolderButton
            }
            else {
                LocalCacheFolderPathText().Text(winrt::to_hstring(cachePath));
                OpenLocalCacheFolderButton().IsEnabled(true);
            }

            // 加载共享本地文件夹路径
            try {
                std::string sharedPath = ::WordWizServices::Data::FilePathProvider::GetAppSharedLocalFolderPath();
                if (sharedPath.empty()) {
                    SharedLocalFolderPathText().Text(L"此系统不支持共享本地文件夹");
                    OpenSharedFolderButton().IsEnabled(false); // 假设按钮名为 OpenSharedFolderButton
                }
                else {
                    SharedLocalFolderPathText().Text(winrt::to_hstring(sharedPath));
                    OpenSharedFolderButton().IsEnabled(true);
                }
            }
            catch (...) {
                SharedLocalFolderPathText().Text(L"此系统不支持共享本地文件夹");
                OpenSharedFolderButton().IsEnabled(false);
            }

            // 加载临时文件夹路径
            std::string tempPath = ::WordWizServices::Data::FilePathProvider::GetAppTemporaryFolderPath();
            if (tempPath.empty()) {
                TemporaryFolderPathText().Text(L"无法获取路径");
                OpenTemporaryFolderButton().IsEnabled(false); // 假设按钮名为 OpenTempFolderButton
            }
            else {
                TemporaryFolderPathText().Text(winrt::to_hstring(tempPath));
                OpenTemporaryFolderButton().IsEnabled(true);
            }
        }
        catch (...) {
            // 处理异常，显示错误信息并禁用所有相关按钮
            LocalFolderPathText().Text(L"加载失败");
            OpenLocalFolderButton().IsEnabled(false);

            LocalCacheFolderPathText().Text(L"加载失败");
            OpenLocalCacheFolderButton().IsEnabled(false);

            SharedLocalFolderPathText().Text(L"加载失败");
            OpenSharedFolderButton().IsEnabled(false);

            TemporaryFolderPathText().Text(L"加载失败");
            OpenTemporaryFolderButton().IsEnabled(false);
        }
    }

    void GeneralSettingsPage::OpenFolderButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
    {
        auto button = sender.as<winrt::Microsoft::UI::Xaml::Controls::Button>();
        auto tag = winrt::unbox_value<winrt::hstring>(button.Tag());
        std::string folderType = winrt::to_string(tag);

        try {
            std::string folderPath;
            
            if (folderType == "local") {
                folderPath = ::WordWizServices::Data::FilePathProvider::GetAppLocalFolderPath();
            }
            else if (folderType == "cache") {
                folderPath = ::WordWizServices::Data::FilePathProvider::GetAppLocalCacheFolderPath();
            }
            else if (folderType == "settings") {
                folderPath = ::WordWizServices::Data::FilePathProvider::GetAppLocalFolderPath(); // Settings is virtual, open parent folder
            }
            else if (folderType == "shared") {
                try {
                    folderPath = ::WordWizServices::Data::FilePathProvider::GetAppSharedLocalFolderPath();
                }
                catch (...) {
                    // 共享文件夹可能不受支持，直接返回
                    return;
                }
            }
            else if (folderType == "temp") {
                folderPath = ::WordWizServices::Data::FilePathProvider::GetAppTemporaryFolderPath();
            }

            if (!folderPath.empty()) {
                OpenFolderInExplorer(folderPath);
            }
        }
        catch (...) {
            // 处理异常
        }
    }

    void GeneralSettingsPage::OpenFolderInExplorer(const std::string& folderPath)
    {
        try {
            // 将路径转换为宽字符串
            winrt::hstring widePath = winrt::to_hstring(folderPath);
            
            // 使用 Windows::System::Launcher 打开文件夹
            auto uri = winrt::Windows::Foundation::Uri(L"file:///" + widePath);
            winrt::Windows::System::Launcher::LaunchUriAsync(uri);
        }
        catch (...) {
            // 如果上述方法失败，尝试使用传统的方法
            try {
                std::string command = "explorer.exe \"" + folderPath + "\"";
                std::wstring wcommand(command.begin(), command.end());
                _wsystem(wcommand.c_str());
            }
            catch (...) {
                // 忽略错误
            }
        }
    }

    void GeneralSettingsPage::LoadAppVersion()
    {
        try
        {
            auto packageVersion = Windows::ApplicationModel::Package::Current().Id().Version();
            std::wstringstream wss;
            wss << L"版本 " 
                << packageVersion.Major << L"."
                << packageVersion.Minor << L"."
                << packageVersion.Build << L"."
                << packageVersion.Revision;
            AppVersionText().Text(wss.str().c_str());
        }
        catch (const winrt::hresult_error& e)
        {
            AppVersionText().Text(L"版本 N/A");
        }
        catch (...)
        {
            AppVersionText().Text(L"版本 获取失败");
        }
    }
}
