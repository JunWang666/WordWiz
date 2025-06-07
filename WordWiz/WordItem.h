// WordItem.h
#pragma once
#include "WordItem.g.h"

namespace winrt::WordWiz::implementation
{
	struct WordItem : WordItemT<WordItem>
	{
		WordItem(hstring const& word, hstring const& explanation);

		hstring Word() { return m_word; }
		hstring Explanation() { return m_explanation; }

	private:
		hstring m_word;
		hstring m_explanation;
	};
}

namespace winrt::WordWiz::factory_implementation
{
	struct WordItem : WordItemT<WordItem, implementation::WordItem>
	{
	};
}
