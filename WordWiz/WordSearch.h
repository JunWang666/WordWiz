// WordSearch.h
#pragma once
#include "WordSearch.g.h"
#include "WordItem.h"
#include "DictionaryItemInWordDetail.g.h" // only include g.h

using namespace winrt;
using namespace Windows::Foundation::Collections;

namespace winrt::WordWiz::implementation
{    struct WordSearch : WordSearchT<WordSearch>
    {
        WordSearch();
        ~WordSearch();

        IVector<WordWiz::WordItem> Search(winrt::hstring const& query);
        IVector<WordWiz::DictionaryItemInWordDetail> GetAvailableDictionaries(winrt::hstring const& word);
        winrt::hstring GetDictionaryHtmlContent(winrt::hstring const& word, winrt::hstring const& dictionaryId);
        
        static void shutdown();

    private:
        // Helper methods for database interaction
        std::string getDictionaryTitle(const std::string& uuid);
        std::pair<std::string, std::string> getCssReplacement(const std::string& uuid);
    };
}

namespace winrt::WordWiz::factory_implementation
{
    struct WordSearch : WordSearchT<WordSearch, implementation::WordSearch>
    {
    };
}