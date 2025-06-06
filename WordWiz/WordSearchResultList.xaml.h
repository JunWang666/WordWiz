// WordSearchResultList.xaml.h
#pragma once
#include "WordSearchResultList.g.h" // 由 MIDL 生成
#include "WordItem.h"                 // WordItem 类型定义
#include "WordSearch.h"               // WordSearch 服务

// 新增包含
#include <winrt/Microsoft.UI.Xaml.Input.h> // For KeyRoutedEventArgs
#include <winrt/Windows.System.h>          // For VirtualKey
#include <winrt/Windows.Foundation.h>      // For IAsyncAction

namespace winrt::WordWiz::implementation
{
	struct WordSearchResultList : WordSearchResultListT<WordSearchResultList>
	{
		WordSearchResultList();

		// Items 属性的 getter
		IObservableVector<WordWiz::WordItem> Items();

		// CurrentDetailItem 依赖属性的 getter 和 setter
		WordWiz::WordItem CurrentDetailItem();
		void CurrentDetailItem(const WordWiz::WordItem& value);

		// CurrentDetailItemProperty 依赖属性的静态 getter
		static Xaml::DependencyProperty CurrentDetailItemProperty()
		{
			return m_currentDetailItemProperty;
		}

		// 事件处理函数
		void ResultsListView_SelectionChanged(const Windows::Foundation::IInspectable& sender,
		                                      const SelectionChangedEventArgs& args);
		void SearchTextBox_TextChanged(const Windows::Foundation::IInspectable& sender,
		                               const TextChangedEventArgs& args);
		// 新增 KeyDown 事件处理函数 for Enter key
		void SearchTextBox_KeyDown(const Windows::Foundation::IInspectable& sender, const Input::KeyRoutedEventArgs& e);


		// 辅助方法 (如果需要)
		void AddSampleItems(); // 你已有的方法
		void AddNewEntry(const hstring& word, const hstring& explanation); // 你已有的方法

		void searchWord(const hstring& query);

	private:
		// 存储 WordItem 对象的列表
		IObservableVector<WordWiz::WordItem> m_items;

		// CurrentDetailItem 依赖属性的静态实例
		static Xaml::DependencyProperty m_currentDetailItemProperty;

		// 新增成员
		DispatcherTimer m_debounceTimer{nullptr};
		WordWiz::WordSearch m_wordSearchService{nullptr}; // WordSearch 服务实例        // 新增私有方法
		void OnDebounceTimerTick(const Windows::Foundation::IInspectable& sender,
		                         const Windows::Foundation::IInspectable& e);
		IAsyncAction ExecuteSearchAsync(hstring query);
		void ShowLoadingState();
		void HideLoadingState();
		void ShowNoResultsState();
		void HideNoResultsState();
		void ShowNoInputState();
		void HideNoInputState();
	};
}

namespace winrt::WordWiz::factory_implementation
{
	struct WordSearchResultList : WordSearchResultListT<WordSearchResultList, implementation::WordSearchResultList>
	{
	};
}
