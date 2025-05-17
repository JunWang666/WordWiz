// WordDetails.xaml.h
#pragma once
#include "WordDetails.g.h"
#include "WordItem.h" // 引入 WordItem 类型

namespace winrt::WordWiz::implementation
{
    struct WordDetails : WordDetailsT<WordDetails>
    {
        WordDetails();

        WordWiz::WordItem ItemToDisplay();
        void ItemToDisplay(WordWiz::WordItem const& value);
        static Microsoft::UI::Xaml::DependencyProperty ItemToDisplayProperty() { return m_itemToDisplayProperty; }

    private:
        static Microsoft::UI::Xaml::DependencyProperty m_itemToDisplayProperty;
        // 可选：属性更改回调的声明
        // static void OnItemToDisplayChanged(Microsoft::UI::Xaml::DependencyObject const& d, Microsoft::UI::Xaml::DependencyPropertyChangedEventArgs const& e);
    };
}
namespace winrt::WordWiz::factory_implementation
{
    struct WordDetails : WordDetailsT<WordDetails, implementation::WordDetails>
    {
    };
}