#pragma once
#include "WordSearchResultItem.xaml.h"
#include "WordSearchResultPage.g.h"

namespace winrt::WordWiz::implementation
{
    struct WordSearchResultPage : WordSearchResultPageT<WordSearchResultPage>
    {
        WordSearchResultPage();

        int32_t MyProperty();
        void MyProperty(int32_t value);

        void OnNavigatedTo(Microsoft::UI::Xaml::Navigation::NavigationEventArgs const& e);
    };
}

namespace winrt::WordWiz::factory_implementation
{
    struct WordSearchResultPage : WordSearchResultPageT<WordSearchResultPage, implementation::WordSearchResultPage>
    {
    };
}
