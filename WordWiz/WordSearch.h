// WordSearch.h
#pragma once
#include "WordSearch.g.h"
#include "WordItem.h"
#include "DictionaryItemInWordDetail.g.h" // ֻ include g.h

using namespace winrt;
using namespace Windows::Foundation::Collections;

namespace winrt::WordWiz::implementation
{
	struct WordSearch : WordSearchT<WordSearch>
	{
		WordSearch();
		~WordSearch() override;

		IVector<WordWiz::WordItem> Search(const hstring& query);
		IVector<DictionaryItemInWordDetail> GetAvailableDictionaries(const hstring& word);
		hstring GetDictionaryHtmlContent(const hstring& word, const hstring& dictionaryId);
	};
}

namespace winrt::WordWiz::factory_implementation
{
	struct WordSearch : WordSearchT<WordSearch, implementation::WordSearch>
	{
	};
}
