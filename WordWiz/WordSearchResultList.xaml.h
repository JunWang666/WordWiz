// WordSearchResultList.xaml.h
#pragma once
#include "WordSearchResultList.g.h"
#include "WordItem.h" // For WordItem type

namespace winrt::WordWiz::implementation
{
    struct WordSearchResultList : WordSearchResultListT<WordSearchResultList>
    {
        WordSearchResultList();

        Windows::Foundation::Collections::IObservableVector<WordWiz::WordItem> Items();

        void AddItemButton_Click(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& args);
        void AddSampleItems(); // From your original code
        void AddNewEntry(winrt::hstring const& word, winrt::hstring const& explanation); // From your original code, adapted

        // Added from generic example
        void RemoveSelectedItemsButton_Click(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& args);
        void ClearListButton_Click(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& args);
        void SelectionModeComboBox_SelectionChanged(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& args);


    private:
        Windows::Foundation::Collections::IObservableVector<WordWiz::WordItem> m_items;
        // If you need m_newItemCounter for AddItemButton_Click, declare it here.
    };
}
namespace winrt::WordWiz::factory_implementation
{
    struct WordSearchResultList : WordSearchResultListT<WordSearchResultList, implementation::WordSearchResultList>
    {
    };
}