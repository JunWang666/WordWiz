#pragma once
#include <winrt/Windows.Foundation.Collections.h>
#include "DictionaryItemInWordDetail.g.h"

namespace winrt::WordWiz::implementation
{
    struct DictionaryItemInWordDetail : DictionaryItemInWordDetailT<DictionaryItemInWordDetail>
    {
      DictionaryItemInWordDetail(winrt::hstring const& id, winrt::hstring const& name, winrt::hstring const& displayName);

        winrt::hstring Id();
        winrt::hstring Name();
        winrt::hstring DisplayName();
        bool IsEqual(WordWiz::DictionaryItemInWordDetail const& other);

    private:
        winrt::hstring m_id;
        winrt::hstring m_name;
        winrt::hstring m_displayName;
    };
}

namespace winrt::WordWiz::factory_implementation
{
    struct DictionaryItemInWordDetail : DictionaryItemInWordDetailT<DictionaryItemInWordDetail, implementation::DictionaryItemInWordDetail>
    {
    };
}
