#pragma once

#include "GeneralSettingsPage.g.h"

namespace winrt::WordWiz::implementation
{
	struct GeneralSettingsPage : GeneralSettingsPageT<GeneralSettingsPage>
	{
		GeneralSettingsPage();

		void ThemeRadio_Checked(const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e);
		void OpenFolderButton_Click(const Windows::Foundation::IInspectable& sender, const RoutedEventArgs& e);

	private:
		void LoadCurrentTheme();
		void ApplyTheme(const std::string& themeMode);
		bool IsSystemInDarkMode();
		void LoadFolderPaths();
		void OpenFolderInExplorer(const std::string& folderPath);
		void LoadAppVersion();
	};
}

namespace winrt::WordWiz::factory_implementation
{
	struct GeneralSettingsPage : GeneralSettingsPageT<GeneralSettingsPage, implementation::GeneralSettingsPage>
	{
	};
}
