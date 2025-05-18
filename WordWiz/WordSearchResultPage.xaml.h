// ResultPage.xaml.h
#pragma once
#include "WordSearchResultPage.g.h" // MIDL 生成
#include "WordItem.h"     // WordItem 类型

namespace winrt::WordWiz::implementation // 确保命名空间正确
{
    struct WordSearchResultPage : WordSearchResultPageT<WordSearchResultPage>
    {
        WordSearchResultPage();

        WordWiz::WordItem PageLevelSelectedItem();
        void PageLevelSelectedItem(WordWiz::WordItem const& value);
        static Microsoft::UI::Xaml::DependencyProperty PageLevelSelectedItemProperty() { return m_pageLevelSelectedItemProperty; }

    private:
        static Microsoft::UI::Xaml::DependencyProperty m_pageLevelSelectedItemProperty;
    };
}
namespace winrt::WordWiz::factory_implementation
{
    struct WordSearchResultPage : WordSearchResultPageT<WordSearchResultPage, implementation::WordSearchResultPage>
    {
    };
}