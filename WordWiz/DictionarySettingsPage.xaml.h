#pragma once

#include "DictionarySettingsPage.g.h"

namespace winrt::WordWiz::implementation
{
    struct DictionarySettingsPage : DictionarySettingsPageT<DictionarySettingsPage>
    {
        DictionarySettingsPage();
    };
}

namespace winrt::WordWiz::factory_implementation
{
    struct DictionarySettingsPage : DictionarySettingsPageT<DictionarySettingsPage, implementation::DictionarySettingsPage>
    {
    };
}
