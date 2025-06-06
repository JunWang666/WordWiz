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

		int32_t MyProperty();
		void MyProperty(int32_t value);

		void myButton_Click(const IInspectable& sender, const RoutedEventArgs& args);
	};
}

namespace winrt::WordWiz::factory_implementation
{
	struct FavoritePage : FavoritePageT<FavoritePage, implementation::FavoritePage>
	{
	};
}
