// WordSearchResultList.xaml.cpp
#pragma once
#include "pch.h"
#include "WordSearchResultList.xaml.h"
#include "WordSearchResultList.g.cpp" // MIDL generates this
#include "WordItem.h" // For make<implementation::WordItem>

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Windows::Foundation::Collections;

namespace winrt::WordWiz::implementation
{
    WordSearchResultList::WordSearchResultList()
    {
        m_items = winrt::single_threaded_observable_vector<WordWiz::WordItem>();
        InitializeComponent(); // Crucial: call before accessing XAML elements by name

        AddSampleItems(); // Add some initial data
    }

    IObservableVector<WordWiz::WordItem> WordSearchResultList::Items()
    {
        return m_items;
    }

    // Adapted from your original AddNewEntry and AddSampleItems
    void WordSearchResultList::AddSampleItems()
    {
        AddNewEntry(L"WinUI 3", L"The latest native UX platform from Microsoft for Windows apps.");
        AddNewEntry(L"C++/WinRT", L"A standard C++ language projection for Windows Runtime APIs.");
        AddNewEntry(L"XAML", L"Extensible Application Markup Language, used for defining user interfaces.");
    }

    void WordSearchResultList::AddNewEntry(winrt::hstring const& word, winrt::hstring const& explanation)
    {
        // Create a new WordItem object
        auto newItem = winrt::make<WordWiz::implementation::WordItem>(word, explanation);
        m_items.Append(newItem);
    }

    void WordSearchResultList::AddItemButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        // Get text from input TextBoxes (defined with x:Name in XAML)
        winrt::hstring word = WordInputTextBox().Text();
        winrt::hstring explanation = ExplanationInputTextBox().Text();

        if (!word.empty() && !explanation.empty())
        {
            AddNewEntry(word, explanation);
            // Clear input fields after adding
            WordInputTextBox().Text(L"");
            ExplanationInputTextBox().Text(L"");
        }
        else
        {
            // Optional: Show a message if fields are empty
            // Example: WordInputTextBox().Header(box_value(L"Word: (Cannot be empty)"));
        }
    }

    void WordSearchResultList::RemoveSelectedItemsButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        if (ResultsListView()) // Access ListView by its x:Name
        {
            auto selectedItems = ResultsListView().SelectedItems();
            if (selectedItems.Size() > 0)
            {
                std::vector<WordWiz::WordItem> itemsToRemove;
                for (auto const& item : selectedItems)
                {
                    itemsToRemove.push_back(item.as<WordWiz::WordItem>());
                }

                for (auto const& itemToRemove : itemsToRemove)
                {
                    uint32_t index;
                    if (m_items.IndexOf(itemToRemove, index))
                    {
                        m_items.RemoveAt(index);
                    }
                }
            }
        }
    }

    void WordSearchResultList::ClearListButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        m_items.Clear();
    }

    void WordSearchResultList::SelectionModeComboBox_SelectionChanged(IInspectable const& sender, SelectionChangedEventArgs const& args)
    {
        auto comboBox = sender.as<ComboBox>();
        auto selectedComboBoxItem = comboBox.SelectedItem().as<ComboBoxItem>(); // Assuming ComboBoxItem is used
        if (selectedComboBoxItem) // Check if an item is actually selected
        {
            hstring selectedTag = unbox_value<hstring>(selectedComboBoxItem.Tag());

            ListViewSelectionMode mode = ListViewSelectionMode::Single; // Default
            if (selectedTag == L"None") mode = ListViewSelectionMode::None;
            else if (selectedTag == L"Single") mode = ListViewSelectionMode::Single;
            else if (selectedTag == L"Multiple") mode = ListViewSelectionMode::Multiple;
            else if (selectedTag == L"Extended") mode = ListViewSelectionMode::Extended;

            if (ResultsListView()) // Access ListView by its x:Name
            {
                ResultsListView().SelectionMode(mode);
            }
        }
    }
}