#pragma once

#include "SettingsPage.g.h"
#include "SettingsData.h"
#include "GeneralSettingsPage.xaml.h"
#include "DictionarySettingsPage.xaml.h"

namespace winrt::WordWiz::implementation
{
    struct SettingsPage : SettingsPageT<SettingsPage>
    {
        SettingsPage();

        int32_t MyProperty();
        void MyProperty(int32_t value);

        void myButton_Click(IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& args);
        void SettingsNavigationView_SelectionChanged(IInspectable const& sender, Microsoft::UI::Xaml::Controls::NavigationViewSelectionChangedEventArgs const& args);

    private:
        void NavigateToPage(winrt::hstring const& tag);
    };
}

namespace winrt::WordWiz::factory_implementation
{
    struct SettingsPage : SettingsPageT<SettingsPage, implementation::SettingsPage>
    {
    };
}
