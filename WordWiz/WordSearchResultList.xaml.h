#pragma once

#include "WordSearchResultList.g.h"

namespace winrt::WordWiz::implementation
{
    struct WordSearchResultList : WordSearchResultListT<WordSearchResultList>
    {
        WordSearchResultList()
        {
            // Xaml objects should not call InitializeComponent during construction.
            // See https://github.com/microsoft/cppwinrt/tree/master/nuget#initializecomponent
        }

        int32_t MyProperty();
        void MyProperty(int32_t value);

        void myButton_Click(IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& args);
    };
}

namespace winrt::WordWiz::factory_implementation
{
    struct WordSearchResultList : WordSearchResultListT<WordSearchResultList, implementation::WordSearchResultList>
    {
    };
}
