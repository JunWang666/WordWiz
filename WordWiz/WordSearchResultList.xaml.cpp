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

}