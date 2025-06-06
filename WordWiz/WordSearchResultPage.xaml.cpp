// ResultPage.xaml.cpp
#include "pch.h"
#include "WordSearchResultPage.xaml.h"
#include "WordSearchResultPage.g.cpp" // MIDL 生成
#include "WordSearchResultList.xaml.h" // Include the header for WordSearchResultList  
#include "WordItem.h"       // for xaml_typename
#include <winrt/Microsoft.UI.Xaml.Navigation.h> // For NavigationEventArgs
#include <winrt/Windows.Foundation.Collections.h> // For IMap

using namespace winrt;
using namespace Microsoft::UI::Xaml;

namespace winrt::WordWiz::implementation // 确保命名空间正确
{
	Xaml::DependencyProperty WordSearchResultPage::m_pageLevelSelectedItemProperty =
		Xaml::DependencyProperty::Register(
			L"PageLevelSelectedItem",
			xaml_typename<WordWiz::WordItem>(),
			xaml_typename<WordWiz::WordSearchResultPage>(),
			PropertyMetadata{nullptr}
		);

	WordSearchResultPage::WordSearchResultPage()
	{
	}

	WordWiz::WordItem WordSearchResultPage::PageLevelSelectedItem()
	{
		return GetValue(m_pageLevelSelectedItemProperty).try_as<WordWiz::WordItem>();
	}

	void WordSearchResultPage::PageLevelSelectedItem(const WordWiz::WordItem& value)
	{
		SetValue(m_pageLevelSelectedItemProperty, value);
	}

	void WordSearchResultPage::OnNavigatedTo(const NavigationEventArgs& e)
	{
		// e.Parameter() is of type IInspectable, expected to be a box_value or hstring
		// If you pass a struct or map, adjust accordingly
		auto param = e.Parameter();
		if (param)
		{
			// Try to extract a map parameter
			auto map = param.try_as<IMap<hstring, Windows::Foundation::IInspectable>>();
			if (map && map.HasKey(L"search_word"))
			{
				auto boxed = map.Lookup(L"search_word");
				if (auto searchWord = boxed.try_as<hstring>())
				{
					WordSearchResultListControl().searchWord(searchWord.value());
				}
			}
		}
	}
}
