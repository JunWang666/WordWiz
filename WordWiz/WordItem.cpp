// WordItem.cpp
#pragma once
#include "pch.h"
#include "WordItem.h"
#include "WordItem.g.cpp" // MIDL generates this

namespace winrt::WordWiz::implementation
{
    WordItem::WordItem(hstring const& word, hstring const& explanation)
        : m_word(word), m_explanation(explanation)
    {
    }
}