#pragma once
#include <winrt/Windows.Foundation.Collections.h>
#include "DictionaryItemInWordDetail.g.h"

namespace winrt::WordWiz::implementation
{
	struct DictionaryItemInWordDetail : DictionaryItemInWordDetailT<DictionaryItemInWordDetail>
	{
		DictionaryItemInWordDetail(const hstring& id, const hstring& name, const hstring& displayName);

		hstring Id();
		hstring Name();
		hstring DisplayName();
		bool IsEqual(const WordWiz::DictionaryItemInWordDetail& other);

	private:
		hstring m_id;
		hstring m_name;
		hstring m_displayName;
	};
}

namespace winrt::WordWiz::factory_implementation
{
	struct DictionaryItemInWordDetail : DictionaryItemInWordDetailT<
			DictionaryItemInWordDetail, implementation::DictionaryItemInWordDetail>
	{
	};
}
