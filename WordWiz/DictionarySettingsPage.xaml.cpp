#include "pch.h"
#include "DictionarySettingsPage.xaml.h"
#if __has_include("DictionarySettingsPage.g.cpp")
#include "DictionarySettingsPage.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;

namespace winrt::WordWiz::implementation
{
    DictionarySettingsPage::DictionarySettingsPage()
    {
        InitializeComponent();
    }
}
