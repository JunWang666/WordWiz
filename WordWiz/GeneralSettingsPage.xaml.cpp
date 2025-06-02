#include "pch.h"
#include "GeneralSettingsPage.xaml.h"
#if __has_include("GeneralSettingsPage.g.cpp")
#include "GeneralSettingsPage.g.cpp"
#endif
#include "SettingsData.h"
#include "App.xaml.h"
#include <winrt/Windows.UI.ViewManagement.h>

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;

namespace winrt::WordWiz::implementation
{
    GeneralSettingsPage::GeneralSettingsPage()
    {
        InitializeComponent();
        LoadCurrentTheme();
    }

    void GeneralSettingsPage::ThemeRadio_Checked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
    {
        auto radio = sender.as<winrt::Microsoft::UI::Xaml::Controls::RadioButton>();
        auto tag = winrt::unbox_value<winrt::hstring>(radio.Tag());
        std::string themeMode = winrt::to_string(tag);
        
        // 保存主题设置到数据库
        try {
            ::WordWiz::Data::SettingsManager settingsManager("app_settings");            if (settingsManager.isInitialized()) {
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
}
