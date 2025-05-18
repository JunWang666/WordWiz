// WordSearchResultList.xaml.cpp
#include "pch.h"
#include "WordSearchResultList.xaml.h"
#include "WordSearchResultList.g.cpp" // 由 MIDL 生成
#include "WordItem.h"                 // 创建 WordItem 实例需要

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls; // For ListView, ComboBox, SelectionChangedEventArgs etc.
using namespace Windows::Foundation::Collections;

namespace winrt::WordWiz::implementation
{
    // 静态依赖属性 m_currentDetailItemProperty 的定义和注册
    Microsoft::UI::Xaml::DependencyProperty WordSearchResultList::m_currentDetailItemProperty =
        Microsoft::UI::Xaml::DependencyProperty::Register(
            L"CurrentDetailItem",                               // 属性名称
            xaml_typename<WordWiz::WordItem>(),                 // 属性的类型
            xaml_typename<WordWiz::WordSearchResultList>(),     // 拥有此属性的类的类型
            Microsoft::UI::Xaml::PropertyMetadata{ nullptr }      // 属性的元数据，默认值为 nullptr
        );

    // 构造函数
    WordSearchResultList::WordSearchResultList()
    {
        // 初始化 m_items 可观察向量
        m_items = winrt::single_threaded_observable_vector<WordWiz::WordItem>();

        InitializeComponent(); // XAML 生成的代码，必须调用

        // 添加一些示例数据（如果需要）
        AddSampleItems();
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
        auto listView = sender.as<Controls::ListView>();
        // 当 ListView 的选中项改变时，更新本控件的 CurrentDetailItem 依赖属性
        // 由于 CurrentDetailItem 是一个依赖属性，SetValue 会自动通知任何绑定到它的UI元素或属性
        this->CurrentDetailItem(listView.SelectedItem().try_as<WordWiz::WordItem>());
    }

    // --- WordSearchResultList 内部其他交互逻辑的实现 ---
    // (这些方法的 XAML 连接需要在 WordSearchResultList.xaml 中设置)

    void WordSearchResultList::AddSampleItems()
    {
        AddNewEntry(L"WinUI 3 Sample", L"Native UX platform from Microsoft.");
        AddNewEntry(L"C++/WinRT Sample", L"Standard C++ language projection aaaaaaaaaaa.");
        // ... 根据需要添加更多
    }

    void WordSearchResultList::AddNewEntry(winrt::hstring const& word, winrt::hstring const& explanation)
    {
        auto newItem = winrt::make<WordWiz::implementation::WordItem>(word, explanation);
        m_items.Append(newItem);
    }


}