// WordSearch.h
#pragma once
// The generated header name might change.
// If WordSearch.idl is in the root and defines WordWiz.WordSearch,
// it might be "WordSearch.g.h" or "WordWiz.WordSearch.g.h".
// Start with "WordSearch.g.h" and adjust if necessary after trying to build.
#include "WordSearch.g.h"
#include "WordItem.h" // Assuming WordItem.h is findable

// Adjust namespace to WordWiz::implementation
namespace winrt::WordWiz::implementation
{
    // Class name remains WordSearch, but it's now WordWiz::WordSearch
    struct WordSearch : WordSearchT<WordSearch>
    {
        WordSearch() = default;

        Windows::Foundation::Collections::IVector<WordWiz::WordItem> Search(winrt::hstring const& query);
    };
}

// Adjust namespace to WordWiz::factory_implementation
namespace winrt::WordWiz::factory_implementation
{
    struct WordSearch : WordSearchT<WordSearch, implementation::WordSearch>
    {
    };
}
