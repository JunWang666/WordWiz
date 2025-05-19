// WordDetails.xaml.cpp
#include "pch.h"
#include "WordDetails.xaml.h"
#include "WordDetails.g.cpp"
#include "WordItem.h" // For xaml_typename for WordWiz::WordItem

using namespace winrt;
using namespace Microsoft::UI::Xaml;

namespace winrt::WordWiz::implementation
{
    Microsoft::UI::Xaml::DependencyProperty WordDetails::m_itemToDisplayProperty =
        Microsoft::UI::Xaml::DependencyProperty::Register(
            L"ItemToDisplay",
            xaml_typename<WordWiz::WordItem>(),
            xaml_typename<WordWiz::WordDetails>(),
            Microsoft::UI::Xaml::PropertyMetadata{ nullptr } // 初始值为 nullptr，可以添加一个属性更改回调
            // PropertyMetadata{nullptr, PropertyChangedCallback(&WordDetails::OnItemToDisplayChanged)}
        );

    WordDetails::WordDetails()
    {
        InitializeComponent();
    }

    WordWiz::WordItem WordDetails::ItemToDisplay()
    {
        return GetValue(m_itemToDisplayProperty).try_as<WordWiz::WordItem>();
    }

    void WordDetails::ItemToDisplay(WordWiz::WordItem const& value)
    {
        SetValue(m_itemToDisplayProperty, value);
    }

    // 可选的属性更改回调函数
    /*
    void WordDetails::OnItemToDisplayChanged(DependencyObject const& d, DependencyPropertyChangedEventArgs const& e)
    {
        if (auto SendersThis{ d.try_as<WordDetails>() })
        {
            // 当 ItemToDisplay 属性变化时，可以在这里执行一些操作
            // 不过，对于显示 ItemToDisplay.Word 和 ItemToDisplay.Explanation，
            // XAML 中的 {x:Bind ItemToDisplay.Word} 通常会自动处理更新。
            // 此回调更多用于需要额外逻辑处理的场景。
        }
    }
    */
}