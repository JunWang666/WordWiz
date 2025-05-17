// WordSearchResultItem.xaml.h
#pragma once
#include "WordSearchResultItem.g.h"

namespace winrt::WordWiz::implementation
{
    struct WordSearchResultItem : WordSearchResultItemT<WordSearchResultItem>
    {
        WordSearchResultItem();
        void WordSearchResultItem::OnLoadedEvent(Windows::Foundation::IInspectable const& /*sender*/, Microsoft::UI::Xaml::RoutedEventArgs const& /*args*/);

        winrt::hstring WordDisplay();
        void WordDisplay(winrt::hstring const& value);
        static Microsoft::UI::Xaml::DependencyProperty WordDisplayProperty() { return m_wordDisplayProperty; }

        winrt::hstring ExplanationDisplay();
        void ExplanationDisplay(winrt::hstring const& value);
        static Microsoft::UI::Xaml::DependencyProperty ExplanationDisplayProperty() { return m_explanationDisplayProperty; }

        // Optional: Click event
        // event_token ItemClicked(Microsoft::UI::Xaml::RoutedEventHandler const& handler);
        // void ItemClicked(event_token const& token) noexcept;
        // void OnPointerPressed(Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e);


    private:
        static Microsoft::UI::Xaml::DependencyProperty m_wordDisplayProperty;
        static Microsoft::UI::Xaml::DependencyProperty m_explanationDisplayProperty;
        // Microsoft::UI::Xaml::event<Microsoft::UI::Xaml::RoutedEventHandler> m_itemClicked;
    };
}
namespace winrt::WordWiz::factory_implementation
{
    struct WordSearchResultItem : WordSearchResultItemT<WordSearchResultItem, implementation::WordSearchResultItem>
    {
    };
}