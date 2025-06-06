#pragma once
#include "WordDetails.g.h"
#include "WordItem.h"
#include "WordSearch.h" // 引入 WordSearch 服务
#include "DictionaryItemInWordDetail.g.h" // 只 include g.h
#include "NavigationService.h" // 新增：引入NavigationService头文件

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Windows::Foundation::Collections;

namespace winrt::WordWiz::implementation
{
	struct WordDetails : WordDetailsT<WordDetails>
	{
		WordDetails();

		// ItemToDisplay (已存在)
		WordWiz::WordItem ItemToDisplay();
		void ItemToDisplay(const WordWiz::WordItem& value);
		static DependencyProperty ItemToDisplayProperty() { return m_itemToDisplayProperty; }

		// 更新：字典项列表 (用于 SelectorBar)
		IObservableVector<DictionaryItemInWordDetail> DictionaryItems();
		static DependencyProperty DictionaryItemsProperty() { return m_dictionaryItemsProperty; }

		// 当前选中字典的HTML内容 (用于 WebView2)
		// 注意：这个属性的更改将通过其回调函数来触发 WebView2 的 NavigateToString
		hstring SelectedDictionaryHtml();
		void SelectedDictionaryHtml(const hstring& value);
		static DependencyProperty SelectedDictionaryHtmlProperty() { return m_selectedDictionaryHtmlProperty; }

		// SelectorBar 的事件处理
		void DictionarySelectorBar_SelectionChanged(const SelectorBar& sender,
		                                            const SelectorBarSelectionChangedEventArgs& args);

		void UpdateDetailVisibility();

		void OnLoaded(const IInspectable& sender, const RoutedEventArgs& args);
		fire_and_forget InitializeWebView2Async(); // 异步初始化 WebView2

		// ItemToDisplay 属性更改回调
		static void OnItemToDisplayChanged(const DependencyObject& d, const DependencyPropertyChangedEventArgs& e);
		// SelectedDictionaryHtml 属性更改回调
		static void OnSelectedDictionaryHtmlChanged(const DependencyObject& d,
		                                            const DependencyPropertyChangedEventArgs& e);

		// 设置Frame的方法（外部页面初始化WordDetails后应调用）
		void SetHostFrame(const Frame& frame) { m_hostFrame = frame; }

	private:
		static DependencyProperty m_itemToDisplayProperty;
		static DependencyProperty m_dictionaryItemsProperty;
		static DependencyProperty m_selectedDictionaryHtmlProperty;


		bool m_isCoreWebView2Initialized{false};
		hstring m_pendingHtmlToNavigate{L""};

		// 用于记录Frame的弱引用（需在构造或初始化时赋值）
		Frame m_hostFrame{nullptr};
	};
}

namespace winrt::WordWiz::factory_implementation
{
	struct WordDetails : WordDetailsT<WordDetails, implementation::WordDetails>
	{
	};
}
