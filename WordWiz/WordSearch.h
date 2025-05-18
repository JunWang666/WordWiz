// WordSearch.h
#pragma once
#include "WordSearch.g.h"
#include "WordItem.h" 

namespace winrt::WordWiz::implementation
{
    struct WordSearch : WordSearchT<WordSearch>
    {
        WordSearch() = default;

        // 原有的 Search 方法 (我们将从中移除延迟)
        Windows::Foundation::Collections::IVector<WordWiz::WordItem> Search(winrt::hstring const& query);

        // 新增方法
        Windows::Foundation::Collections::IVector<winrt::hstring> GetAvailableDictionaries(winrt::hstring const& word);
        winrt::hstring GetDictionaryHtmlContent(winrt::hstring const& word, winrt::hstring const& dictionaryName);
    };
}

namespace winrt::WordWiz::factory_implementation
{
    struct WordSearch : WordSearchT<WordSearch, implementation::WordSearch>
    {
    };
}