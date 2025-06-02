#pragma once

#include "GeneralSettingsPage.g.h"

namespace winrt::WordWiz::implementation
{    struct GeneralSettingsPage : GeneralSettingsPageT<GeneralSettingsPage>
    {
        GeneralSettingsPage();

        void ThemeRadio_Checked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);

    private:
        void LoadCurrentTheme();
        void ApplyTheme(const std::string& themeMode);
        bool IsSystemInDarkMode();
    };
}

namespace winrt::WordWiz::factory_implementation
{
    struct GeneralSettingsPage : GeneralSettingsPageT<GeneralSettingsPage, implementation::GeneralSettingsPage>
    {
    };
}
