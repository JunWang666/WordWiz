// WordSearchResultList.xaml.h
#pragma once
#include "WordSearchResultList.g.h" // 由 MIDL 生成
#include "WordItem.h"                 // WordItem 类型定义

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

        // ListView (假设其 x:Name="ResultsListView") 的 SelectionChanged 事件处理函数
        void ResultsListView_SelectionChanged(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& args);

        // 辅助方法 (如果需要)
        void AddSampleItems();
        void AddNewEntry(winrt::hstring const& word, winrt::hstring const& explanation);


    private:
        // 存储 WordItem 对象的列表
        Windows::Foundation::Collections::IObservableVector<WordWiz::WordItem> m_items;

        // CurrentDetailItem 依赖属性的静态实例
        static Microsoft::UI::Xaml::DependencyProperty m_currentDetailItemProperty;
    };
}

namespace winrt::WordWiz::factory_implementation
{
    struct WordSearchResultList : WordSearchResultListT<WordSearchResultList, implementation::WordSearchResultList>
    {
    };
}