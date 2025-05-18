// ResultPage.xaml.cpp
#include "pch.h"
#include "WordSearchResultPage.xaml.h"
#include "WordSearchResultPage.g.cpp" // MIDL 生成
#include "WordItem.h"       // for xaml_typename

using namespace winrt;
using namespace Microsoft::UI::Xaml;

namespace winrt::WordWiz::implementation // 确保命名空间正确
{
    Microsoft::UI::Xaml::DependencyProperty WordSearchResultPage::m_pageLevelSelectedItemProperty =
        Microsoft::UI::Xaml::DependencyProperty::Register(
            L"PageLevelSelectedItem",
            xaml_typename<WordWiz::WordItem>(),
            xaml_typename<WordWiz::WordSearchResultPage>(),
            Microsoft::UI::Xaml::PropertyMetadata{ nullptr }
        );

    WordSearchResultPage::WordSearchResultPage()
    {
    }

    WordWiz::WordItem WordSearchResultPage::PageLevelSelectedItem()
    {
        return GetValue(m_pageLevelSelectedItemProperty).try_as<WordWiz::WordItem>();
    }

    void WordSearchResultPage::PageLevelSelectedItem(WordWiz::WordItem const& value)
    {
        SetValue(m_pageLevelSelectedItemProperty, value);
    }
}