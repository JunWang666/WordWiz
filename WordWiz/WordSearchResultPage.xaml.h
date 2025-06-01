// WordSearchResultPage.xaml.h
#pragma once
#include "WordSearchResultPage.g.h" // MIDL 生成
#include "WordItem.h"     // WordItem 类型
#include "WordSearch.h"   // WordSearch 服务
#include <winrt/Microsoft.UI.Xaml.Navigation.h> // Add this include for NavigationEventArgs
#include <winrt/Microsoft.UI.Xaml.Input.h> // For KeyRoutedEventArgs
#include <winrt/Windows.System.h>          // For VirtualKey
#include <winrt/Windows.Foundation.Collections.h> // For IMap

namespace winrt::WordWiz::implementation // 确保命名空间正确
{
    struct WordSearchResultPage : WordSearchResultPageT<WordSearchResultPage>
    {
        WordSearchResultPage();

        // PageLevelSelectedItem 依赖属性
        WordWiz::WordItem PageLevelSelectedItem();
        void PageLevelSelectedItem(WordWiz::WordItem const& value);
        static Microsoft::UI::Xaml::DependencyProperty PageLevelSelectedItemProperty() { return m_pageLevelSelectedItemProperty; }

        // Items 属性的 getter
        Windows::Foundation::Collections::IObservableVector<WordWiz::WordItem> Items();

        // Add override for OnNavigatedTo
        void OnNavigatedTo(Microsoft::UI::Xaml::Navigation::NavigationEventArgs const& e);

        // 事件处理函数 (从 WordSearchResultList 移植而来)
        void ResultsListView_SelectionChanged(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& args);
        void SearchTextBox_TextChanged(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::Controls::TextChangedEventArgs const& args);
        void SearchTextBox_KeyDown(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::Input::KeyRoutedEventArgs const& e);

        // 搜索相关方法
        void searchWord(winrt::hstring const& query);

    private:
        static Microsoft::UI::Xaml::DependencyProperty m_pageLevelSelectedItemProperty;
        
        // 从 WordSearchResultList 移植的成员变量
        Windows::Foundation::Collections::IObservableVector<WordWiz::WordItem> m_items;
        Microsoft::UI::Xaml::DispatcherTimer m_debounceTimer{ nullptr };
        WordWiz::WordSearch m_wordSearchService{ nullptr }; // WordSearch 服务实例

        // 私有方法 (从 WordSearchResultList 移植而来)
        void OnDebounceTimerTick(Windows::Foundation::IInspectable const& sender, Windows::Foundation::IInspectable const& e);
        void ExecuteSearch(winrt::hstring const& query);
    };
}
namespace winrt::WordWiz::factory_implementation
{
    struct WordSearchResultPage : WordSearchResultPageT<WordSearchResultPage, implementation::WordSearchResultPage>
    {
    };
}