// WordSearchResultList.xaml.h
#pragma once
#include "WordSearchResultList.g.h" // 由 MIDL 生成
#include "WordItem.h"                 // WordItem 类型定义
#include "WordSearch.h"               // WordSearch 服务

// 新增包含
#include <winrt/Microsoft.UI.Xaml.Input.h> // For KeyRoutedEventArgs
#include <winrt/Windows.System.h>          // For VirtualKey

namespace winrt::WordWiz::implementation
{
    struct WordSearchResultList : WordSearchResultListT<WordSearchResultList>
    {
        WordSearchResultList();

        // Items 属性的 getter
        Windows::Foundation::Collections::IObservableVector<WordWiz::WordItem> Items();

        // CurrentDetailItem 依赖属性的 getter 和 setter
        WordWiz::WordItem CurrentDetailItem();
        void CurrentDetailItem(WordWiz::WordItem const& value);

        // CurrentDetailItemProperty 依赖属性的静态 getter
        static Microsoft::UI::Xaml::DependencyProperty CurrentDetailItemProperty() { return m_currentDetailItemProperty; }

        // 事件处理函数
        void ResultsListView_SelectionChanged(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& args);
        void SearchTextBox_TextChanged(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::Controls::TextChangedEventArgs const& args);
        // 新增 KeyDown 事件处理函数 for Enter key
        void SearchTextBox_KeyDown(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::Input::KeyRoutedEventArgs const& e);


        // 辅助方法 (如果需要)
        void AddSampleItems(); // 你已有的方法
        void AddNewEntry(winrt::hstring const& word, winrt::hstring const& explanation); // 你已有的方法


    private:
        // 存储 WordItem 对象的列表
        Windows::Foundation::Collections::IObservableVector<WordWiz::WordItem> m_items;

        // CurrentDetailItem 依赖属性的静态实例
        static Microsoft::UI::Xaml::DependencyProperty m_currentDetailItemProperty;

        // 新增成员
        Microsoft::UI::Xaml::DispatcherTimer m_debounceTimer{ nullptr };
        WordWiz::WordSearch m_wordSearchService{ nullptr }; // WordSearch 服务实例

        // 新增私有方法
        void OnDebounceTimerTick(Windows::Foundation::IInspectable const& sender, Windows::Foundation::IInspectable const& e);
        void ExecuteSearch(winrt::hstring const& query);
    };
}

namespace winrt::WordWiz::factory_implementation
{
    struct WordSearchResultList : WordSearchResultListT<WordSearchResultList, implementation::WordSearchResultList>
    {
    };
}