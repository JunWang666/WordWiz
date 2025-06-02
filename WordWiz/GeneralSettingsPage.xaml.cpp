#include "pch.h"
#include "GeneralSettingsPage.xaml.h"
#if __has_include("GeneralSettingsPage.g.cpp")
#include "GeneralSettingsPage.g.cpp"
#endif
#include "SettingsData.h"
#include "App.xaml.h"

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;

namespace winrt::WordWiz::implementation
{    GeneralSettingsPage::GeneralSettingsPage()
    {
        InitializeComponent();
        LoadCurrentTheme();
    }

    void GeneralSettingsPage::DarkModeToggle_Toggled(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
    {
        auto toggle = sender.as<ToggleSwitch>();
        bool isDarkMode = toggle.IsOn();
        
        // 保存主题设置到数据库
        try {
            ::WordWiz::Data::SettingsManager settingsManager("app_settings");
            if (settingsManager.isInitialized()) {
                settingsManager.setString("theme_mode", isDarkMode ? "dark" : "light");
                
                // 立即应用主题
                ApplyTheme(isDarkMode);
            }
        }        catch (const std::exception& e) {
            // 处理错误
        }
    }

    void GeneralSettingsPage::LoadCurrentTheme()
    {
        try {
            ::WordWiz::Data::SettingsManager settingsManager("app_settings");
            if (settingsManager.isInitialized()) {
                std::string themeMode = settingsManager.getString("theme_mode", "light");
                bool isDarkMode = (themeMode == "dark");
                
                // 设置 Toggle 状态
                DarkModeToggle().IsOn(isDarkMode);
                
                // 同时应用主题到应用程序
                ApplyTheme(isDarkMode);
            }
        }        catch (const std::exception& e) {
            // 使用默认主题
            DarkModeToggle().IsOn(false);
            ApplyTheme(false);
        }
    }    void GeneralSettingsPage::ApplyTheme(bool isDarkMode)
    {
        // 使用App类的全局主题设置方法
        try {
            ::winrt::WordWiz::implementation::App::SetGlobalTheme(isDarkMode);
        }
        catch (...) {
            // 忽略主题设置错误
        }
    }
}
