#include "pch.h"
#include "DictionaryItemInWordDetail.h"
#include "DictionaryItemInWordDetail.g.cpp"

namespace winrt::WordWiz::implementation
{
	DictionaryItemInWordDetail::DictionaryItemInWordDetail(const hstring& id, const hstring& name,
	                                                       const hstring& displayName) :
		m_id(id), m_name(name), m_displayName(displayName)
	{
	}

	hstring DictionaryItemInWordDetail::Id()
	{
		return m_id;
	}

	hstring DictionaryItemInWordDetail::Name()
	{
		return m_name;
	}

	hstring DictionaryItemInWordDetail::DisplayName()
	{
		return m_displayName;
	}

	bool DictionaryItemInWordDetail::IsEqual(const WordWiz::DictionaryItemInWordDetail& other)
	{
		return m_id == other.Id();
	}
}
