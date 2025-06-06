// ResultPage.xaml.h
#pragma once
#include "WordSearchResultPage.g.h" // MIDL 生成
#include "WordItem.h"     // WordItem 类型
#include "WordSearchResultList.xaml.h" // Include the header for WordSearchResultList  
#include <winrt/Microsoft.UI.Xaml.Navigation.h> // Add this include for NavigationEventArgs
#include <winrt/Windows.Foundation.Collections.h> // For IMap

namespace winrt::WordWiz::implementation // 确保命名空间正确
{
	struct WordSearchResultPage : WordSearchResultPageT<WordSearchResultPage>
	{
		WordSearchResultPage();

		WordWiz::WordItem PageLevelSelectedItem();
		void PageLevelSelectedItem(WordWiz::WordItem const& value);

		static Microsoft::UI::Xaml::DependencyProperty PageLevelSelectedItemProperty()
		{
			return m_pageLevelSelectedItemProperty;
		}

		// Add override for OnNavigatedTo
		void OnNavigatedTo(Microsoft::UI::Xaml::Navigation::NavigationEventArgs const& e);

	private:
		static Microsoft::UI::Xaml::DependencyProperty m_pageLevelSelectedItemProperty;
	};
}

namespace winrt::WordWiz::factory_implementation
{
	struct WordSearchResultPage : WordSearchResultPageT<WordSearchResultPage, implementation::WordSearchResultPage>
	{
	};
}
