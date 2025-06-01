// WordSearchResultPage.xaml.cpp
#include "pch.h"
#include "WordSearchResultPage.xaml.h"
#include "WordSearchResultPage.g.cpp" // MIDL 生成
#include "WordItem.h"       // for xaml_typename
#include <winrt/Microsoft.UI.Xaml.Navigation.h> // For NavigationEventArgs
#include <winrt/Windows.Foundation.Collections.h> // For IMap
#include <winrt/Windows.System.h> // For VirtualKey

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Windows::Foundation::Collections;
using namespace Windows::System; // For VirtualKey

namespace winrt::WordWiz::implementation // 确保命名空间正确
{
    Microsoft::UI::Xaml::DependencyProperty WordSearchResultPage::m_pageLevelSelectedItemProperty =
        Microsoft::UI::Xaml::DependencyProperty::Register(
            L"PageLevelSelectedItem",
            xaml_typename<WordWiz::WordItem>(),
            xaml_typename<WordWiz::WordSearchResultPage>(),
            Microsoft::UI::Xaml::PropertyMetadata{ nullptr }
        );    WordSearchResultPage::WordSearchResultPage()
    {
        // 初始化从 WordSearchResultList 移植的成员
        m_items = winrt::single_threaded_observable_vector<WordWiz::WordItem>();

        // 初始化 WordSearch 服务
        m_wordSearchService = winrt::make<WordWiz::implementation::WordSearch>();

        // 初始化防抖计时器
        m_debounceTimer = Microsoft::UI::Xaml::DispatcherTimer();
        m_debounceTimer.Interval(std::chrono::milliseconds{ 500 }); // 设置0.5秒间隔
        m_debounceTimer.Tick({ this, &WordSearchResultPage::OnDebounceTimerTick });

        InitializeComponent(); // XAML 生成的代码，必须调用
    }

    WordWiz::WordItem WordSearchResultPage::PageLevelSelectedItem()
    {
        return GetValue(m_pageLevelSelectedItemProperty).try_as<WordWiz::WordItem>();
    }    void WordSearchResultPage::PageLevelSelectedItem(WordWiz::WordItem const& value)
    {
        SetValue(m_pageLevelSelectedItemProperty, value);
    }

    // Items 属性的 getter 实现 (从 WordSearchResultList 移植而来)
    IObservableVector<WordWiz::WordItem> WordSearchResultPage::Items()
    {
        return m_items;
    }

    void WordSearchResultPage::OnNavigatedTo(Microsoft::UI::Xaml::Navigation::NavigationEventArgs const& e)
    {
        // e.Parameter() is of type IInspectable, expected to be a box_value or hstring
        // If you pass a struct or map, adjust accordingly
        auto param = e.Parameter();
        if (param)
        {
            // Try to extract a map parameter
            auto map = param.try_as<winrt::Windows::Foundation::Collections::IMap<winrt::hstring, winrt::Windows::Foundation::IInspectable>>();            if (map && map.HasKey(L"search_word"))
            {
                auto boxed = map.Lookup(L"search_word");
                if (auto searchWordValue = boxed.try_as<winrt::hstring>())
                {
                    searchWord(searchWordValue.value());
                }
            }
        }
    }

    // 从 WordSearchResultList 移植的事件处理函数
    void WordSearchResultPage::ResultsListView_SelectionChanged(IInspectable const& sender, Controls::SelectionChangedEventArgs const& /*args*/)
    {
        auto listView = sender.as<Microsoft::UI::Xaml::Controls::ListView>();
        auto selectedListViewItem = listView.SelectedItem(); // 获取当前选中的原始项

        if (selectedListViewItem) // 关键：仅当确实有一个选中项时才更新
        {
            // 将选中的项转换为 WordItem 并更新 PageLevelSelectedItem
            this->PageLevelSelectedItem(selectedListViewItem.try_as<WordWiz::WordItem>());
        }
    }

    void WordSearchResultPage::SearchTextBox_TextChanged(Windows::Foundation::IInspectable const& /*sender*/, Microsoft::UI::Xaml::Controls::TextChangedEventArgs const& /*args*/)
    {
        m_debounceTimer.Start();
    }

    void WordSearchResultPage::SearchTextBox_KeyDown(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::Input::KeyRoutedEventArgs const& e)
    {
        if (e.Key() == VirtualKey::Enter)
        {
            m_debounceTimer.Stop(); // 停止可能正在计时的防抖操作
            auto textBox = sender.as<Controls::TextBox>();
            ExecuteSearch(textBox.Text());
        }
    }

    void WordSearchResultPage::OnDebounceTimerTick(Windows::Foundation::IInspectable const& /*sender*/, Windows::Foundation::IInspectable const& /*e*/)
    {
        m_debounceTimer.Stop(); // 计时器触发后先停止
        ExecuteSearch(SearchTextBox().Text()); // SearchTextBox() 是 XAML 中定义的控件
    }

    void WordSearchResultPage::ExecuteSearch(winrt::hstring const& query)
    {
        // 调用 WordSearch 服务执行搜索
        auto searchResults = m_wordSearchService.Search(query);

        m_items.Clear(); // 清空现有列表项
        if (searchResults != nullptr) // 确保 searchResults 不是 nullptr
        {
            for (auto const& item : searchResults)
            {
                m_items.Append(item);
            }
        }
    }

    void WordSearchResultPage::searchWord(winrt::hstring const& query)
    {
        SearchTextBox().Text(query); // 更新搜索框文本
        ExecuteSearch(query); // 执行搜索
    }
}