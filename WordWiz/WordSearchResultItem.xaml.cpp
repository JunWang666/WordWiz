// WordSearchResultItem.xaml.cpp
#pragma once
#include "pch.h"
#include "WordSearchResultItem.xaml.h"
#include "WordSearchResultItem.g.cpp" // MIDL generates this

namespace winrt::WordWiz::implementation
{
    Microsoft::UI::Xaml::DependencyProperty WordSearchResultItem::m_wordDisplayProperty =
        Microsoft::UI::Xaml::DependencyProperty::Register(
            L"WordDisplay",
            xaml_typename<winrt::hstring>(),
            xaml_typename<WordWiz::WordSearchResultItem>(),
            Microsoft::UI::Xaml::PropertyMetadata{ box_value(L"") } // Default empty string
        );

    Microsoft::UI::Xaml::DependencyProperty WordSearchResultItem::m_explanationDisplayProperty =
        Microsoft::UI::Xaml::DependencyProperty::Register(
            L"ExplanationDisplay",
            xaml_typename<winrt::hstring>(),
            xaml_typename<WordWiz::WordSearchResultItem>(),
            Microsoft::UI::Xaml::PropertyMetadata{ box_value(L"") } // Default empty string
        );

    WordSearchResultItem::WordSearchResultItem()
    {
        InitializeComponent();
        // Enable this if you want the whole item to be clickable
        // this->PointerPressed({this, &WordSearchResultItem::OnPointerPressed });
        this->Loaded({ this, &WordSearchResultItem::OnLoadedEvent });
    }

    winrt::hstring WordSearchResultItem::WordDisplay()
    {
        return unbox_value<winrt::hstring>(GetValue(m_wordDisplayProperty));
    }

    void WordSearchResultItem::WordDisplay(winrt::hstring const& value)
    {
         WordWizServices::Log::LogMessage(L"WordDisplay_Setter - Received value: '" + value + L"'");
        SetValue(m_wordDisplayProperty, box_value(value));
        // 立刻读回，确认是否存入
         auto checkValue = unbox_value<winrt::hstring>(GetValue(m_wordDisplayProperty));
         WordWizServices::Log::LogMessage(L"WordDisplay_Setter - Value after SetValue: '" + checkValue + L"'");
    }

    winrt::hstring WordSearchResultItem::ExplanationDisplay()
    {
        return unbox_value<winrt::hstring>(GetValue(m_explanationDisplayProperty));
    }

    void WordSearchResultItem::ExplanationDisplay(winrt::hstring const& value)
    {
        SetValue(m_explanationDisplayProperty, box_value(value));
    }

    // Fix: Explicitly cast or convert the concatenated string to `winrt::hstring` before passing it to `LogMessage`.

    void WordSearchResultItem::OnLoadedEvent(Windows::Foundation::IInspectable const& /*sender*/, Microsoft::UI::Xaml::RoutedEventArgs const& /*args*/)
    {
       WordWizServices::Log::LogMessage(winrt::hstring(L"WordSearchResultItem_Loaded. Current WordDisplay DP: '") + WordDisplay() + L"'");
       WordWizServices::Log::LogMessage(winrt::hstring(L"WordSearchResultItem_Loaded. Current ExplanationDisplay DP: '") + ExplanationDisplay() + L"'");

       if (WordTextBlock() && ExplanationTextBlock()) {
           WordWizServices::Log::LogMessage(winrt::hstring(L"WordSearchResultItem_Loaded. TextBlocks found."));
           WordTextBlock().Text( WordDisplay());
           ExplanationTextBlock().Text( ExplanationDisplay());
       } else {
           WordWizServices::Log::LogMessage(winrt::hstring(L"WordSearchResultItem_Loaded. TextBlocks NOT found (check x:Name)."));
       }
    }

    // Optional Click Handler
    /*
    event_token WordSearchResultItem::ItemClicked(Microsoft::UI::Xaml::RoutedEventHandler const& handler) { return m_itemClicked.add(handler); }
    void WordSearchResultItem::ItemClicked(event_token const& token) noexcept { m_itemClicked.remove(token); }

    void WordSearchResultItem::OnPointerPressed(Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e)
    {
        // Example: Raise a custom click event or handle click directly
        // m_itemClicked(*this, Microsoft::UI::Xaml::RoutedEventArgs{});
        // Or, if you have specific logic for an item click:
        // WordWizServices::Log::LogMessage(L"WordSearchResultItem clicked: " + WordDisplay());
        e.Handled(true); // Mark as handled if you don't want it to bubble further in some cases
    }
    */
}