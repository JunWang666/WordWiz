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

		void myButton_Click(const IInspectable& sender, const RoutedEventArgs& args);
		void SettingsNavigationView_SelectionChanged(const IInspectable& sender,
		                                             const NavigationViewSelectionChangedEventArgs& args);

	private:
		void NavigateToPage(const hstring& tag);
	};
}

namespace winrt::WordWiz::factory_implementation
{
	struct SettingsPage : SettingsPageT<SettingsPage, implementation::SettingsPage>
	{
	};
}
