#include "pch.h"
#include "DictionaryItemInWordDetail.h"
#include "DictionaryItemInWordDetail.g.cpp"

namespace winrt::WordWiz::implementation
{
	DictionaryItemInWordDetail::DictionaryItemInWordDetail(winrt::hstring const& id, winrt::hstring const& name,
	                                                       winrt::hstring const& displayName) :
		m_id(id), m_name(name), m_displayName(displayName)
	{
	}

	winrt::hstring DictionaryItemInWordDetail::Id()
	{
		return m_id;
	}

	winrt::hstring DictionaryItemInWordDetail::Name()
	{
		return m_name;
	}

	winrt::hstring DictionaryItemInWordDetail::DisplayName()
	{
		return m_displayName;
	}

	bool DictionaryItemInWordDetail::IsEqual(WordWiz::DictionaryItemInWordDetail const& other)
	{
		return m_id == other.Id();
	}
}
