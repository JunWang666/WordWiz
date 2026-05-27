#pragma once

#include "FavoritePage.g.h"

namespace winrt::WordWiz::implementation
{
	struct FavoritePage : FavoritePageT<FavoritePage>
	{
		FavoritePage()
		{
			// Xaml objects should not call InitializeComponent during construction.
			// See https://github.com/microsoft/cppwinrt/tree/master/nuget#initializecomponent
		}
		void OnNavigatedTo(winrt::Microsoft::UI::Xaml::Navigation::NavigationEventArgs const& e);

		int32_t MyProperty();
		void MyProperty(int32_t value);

		void myButton_Click(IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& args);
		void RefreshButton_Click(IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& args);
		void SortComboBox_SelectionChanged(winrt::Windows::Foundation::IInspectable const& sender,
		                                   winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e);
		void SearchTextBox_TextChanged(winrt::Windows::Foundation::IInspectable const& sender,
		                               winrt::Microsoft::UI::Xaml::Controls::TextChangedEventArgs const& e);
		void RemoveFavoriteButton_Click(winrt::Windows::Foundation::IInspectable const& sender,
		                                winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);

	private:
		void RefreshFavorites();
		std::vector<winrt::hstring> m_allFavorites;
	};
}

namespace winrt::WordWiz::factory_implementation
{
	struct FavoritePage : FavoritePageT<FavoritePage, implementation::FavoritePage>
	{
	};
}
