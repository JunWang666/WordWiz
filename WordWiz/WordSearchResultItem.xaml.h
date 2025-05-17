#pragma once

#include "WordSearchResultItem.g.h"

namespace winrt::WordWiz::implementation
{
    struct WordSearchResultItem : WordSearchResultItemT<WordSearchResultItem>
    {
        WordSearchResultItem();

        int32_t MyProperty();
        void MyProperty(int32_t value);

        hstring Word();
        void Word(hstring const& value);

        hstring Explanation();
        void Explanation(hstring const& value);

        void OnItemClick(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);

    private:
        hstring word;
        hstring explanation;
    };
}

namespace winrt::WordWiz::factory_implementation
{
    struct WordSearchResultItem : WordSearchResultItemT<WordSearchResultItem, implementation::WordSearchResultItem>
    {
    };
}
