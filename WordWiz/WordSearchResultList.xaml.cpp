// WordSearchResultList.xaml.cpp
#include "pch.h"
#include "WordSearchResultList.xaml.h"
#include "WordSearchResultList.g.cpp" // 由 MIDL 生成
#include "WordItem.h"                 // 创建 WordItem 实例需要
// #include "WordSearch.h" // 已经在 pch.h 或 WordSearchResultList.xaml.h 中包含了

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Windows::Foundation::Collections;
using namespace Windows::System; // For VirtualKey

namespace winrt::WordWiz::implementation
{
    // 静态依赖属性 m_currentDetailItemProperty 的定义和注册
    Microsoft::UI::Xaml::DependencyProperty WordSearchResultList::m_currentDetailItemProperty =
        Microsoft::UI::Xaml::DependencyProperty::Register(
            L"CurrentDetailItem",
            xaml_typename<WordWiz::WordItem>(),
            xaml_typename<WordWiz::WordSearchResultList>(),
            Microsoft::UI::Xaml::PropertyMetadata{ nullptr }
        );

    // 构造函数
    WordSearchResultList::WordSearchResultList()
    {
        m_items = winrt::single_threaded_observable_vector<WordWiz::WordItem>();

        // 初始化 WordSearch 服务
        m_wordSearchService = winrt::make<WordWiz::implementation::WordSearch>();

        // 初始化防抖计时器
        m_debounceTimer = Microsoft::UI::Xaml::DispatcherTimer();
        m_debounceTimer.Interval(std::chrono::milliseconds{ 500 }); // 设置0.5秒间隔
        m_debounceTimer.Tick({ this, &WordSearchResultList::OnDebounceTimerTick });

        InitializeComponent(); // XAML 生成的代码，必须调用

        // 你原有的示例数据添加代码
        // AddSampleItems(); 
        // 建议在实际使用时，列表初始为空，通过搜索填充
    }

    // Items 属性的 getter 实现
    IObservableVector<WordWiz::WordItem> WordSearchResultList::Items()
    {
        return m_items;
    }

    // CurrentDetailItem 依赖属性的 getter 实现
    WordWiz::WordItem WordSearchResultList::CurrentDetailItem()
    {
        return GetValue(m_currentDetailItemProperty).try_as<WordWiz::WordItem>();
    }

    // CurrentDetailItem 依赖属性的 setter 实现
    void WordSearchResultList::CurrentDetailItem(WordWiz::WordItem const& value)
    {
        SetValue(m_currentDetailItemProperty, value);
    }

    // ListView 的 SelectionChanged 事件处理函数实现
    void WordSearchResultList::ResultsListView_SelectionChanged(IInspectable const& sender, Controls::SelectionChangedEventArgs const& /*args*/)
    {
        auto listView = sender.as<Microsoft::UI::Xaml::Controls::ListView>();
        auto selectedListViewItem = listView.SelectedItem(); // 获取当前选中的原始项

        if (selectedListViewItem) // 关键：仅当确实有一个选中项时才更新
        {
            // 将选中的项转换为 WordItem 并更新 CurrentDetailItem
            this->CurrentDetailItem(selectedListViewItem.try_as<WordWiz::WordItem>());
        }
        // else (如果 selectedListViewItem 是 nullptr，例如列表被清空或用户取消选择)
        // {
        //     我们不执行任何操作，CurrentDetailItem 保持其之前的值。
        //     这样 WordDetails 控件就不会“复原”。
        // }
    }

    // SearchTextBox 的 TextChanged 事件处理函数 (防抖逻辑)
    void WordSearchResultList::SearchTextBox_TextChanged(Windows::Foundation::IInspectable const& /*sender*/, Microsoft::UI::Xaml::Controls::TextChangedEventArgs const& /*args*/)
    {
        //m_debounceTimer.Stop(); // 每次输入变化时，重置计时器
        m_debounceTimer.Start();
    }

    // SearchTextBox 的 KeyDown 事件处理函数 (回车立即搜索)
    void WordSearchResultList::SearchTextBox_KeyDown(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::Input::KeyRoutedEventArgs const& e)
    {
        if (e.Key() == VirtualKey::Enter)
        {
            m_debounceTimer.Stop(); // 停止可能正在计时的防抖操作
            auto textBox = sender.as<Controls::TextBox>();
            ExecuteSearch(textBox.Text());
        }
    }

    // 防抖计时器触发事件
    void WordSearchResultList::OnDebounceTimerTick(Windows::Foundation::IInspectable const& /*sender*/, Windows::Foundation::IInspectable const& /*e*/)
    {
        m_debounceTimer.Stop(); // 计时器触发后先停止
        // 假设 SearchTextBox 的 x:Name 是 "SearchTextBox"
        // 你需要确保在 XAML 中 SearchTextBox 有一个 x:Name，或者通过其他方式获取它
        // 如果 SearchTextBox 是此用户控件的 XAML 内容的一部分，可以直接使用其成员变量名
        ExecuteSearch(SearchTextBox().Text()); // SearchTextBox() 是 XAML 中定义的控件
    }

    // 执行搜索的辅助方法
    void WordSearchResultList::ExecuteSearch(winrt::hstring const& query)
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
        // 你可以在这里添加逻辑，比如当 searchResults 为空或 m_items 为空时显示 "无结果" 的提示
    }


    // 你已有的辅助方法
    void WordSearchResultList::AddSampleItems()
    {
        AddNewEntry(L"WinUI 3 Sample", L"Native UX platform from Microsoft.");
        AddNewEntry(L"C++/WinRT Sample", L"Standard C++ language projection aaaaaaaaaaa.");
    }

    void WordSearchResultList::AddNewEntry(winrt::hstring const& word, winrt::hstring const& explanation)
    {
        auto newItem = winrt::make<WordWiz::implementation::WordItem>(word, explanation);
        m_items.Append(newItem);
    }
}