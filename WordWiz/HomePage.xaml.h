#pragma once

#include "HomePage.g.h"

namespace winrt::WordWiz::implementation
{
	struct HomePage : HomePageT<HomePage>
	{
		HomePage()
		{
			// Xaml objects should not call InitializeComponent during construction.
			// See https://github.com/microsoft/cppwinrt/tree/master/nuget#initializecomponent
		}

		int32_t MyProperty();
		void MyProperty(int32_t value);

		void myButton_Click(const IInspectable& sender, const RoutedEventArgs& args);
		void TextBox_DragEnter(const Windows::Foundation::IInspectable& sender, const DragEventArgs& e);
		void TextBox_KeyDown(const Windows::Foundation::IInspectable& sender, const Input::KeyRoutedEventArgs& e);
	};
}

namespace winrt::WordWiz::factory_implementation
{
	struct HomePage : HomePageT<HomePage, implementation::HomePage>
	{
	};
}
