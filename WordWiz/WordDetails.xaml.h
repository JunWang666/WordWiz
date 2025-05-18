// WordDetails.xaml.h
#pragma once
#include "WordDetails.g.h"
#include "WordItem.h" 
#include "WordSearch.h" // 引入 WordSearch 服务

namespace winrt::WordWiz::implementation
{
    struct WordDetails : WordDetailsT<WordDetails>
    {
        WordDetails();

        // ItemToDisplay (已存在)
        WordWiz::WordItem ItemToDisplay();
        void ItemToDisplay(WordWiz::WordItem const& value);
        static Microsoft::UI::Xaml::DependencyProperty ItemToDisplayProperty() { return m_itemToDisplayProperty; }

        // 新增：字典名称列表 (用于 SelectorBar)
        Windows::Foundation::Collections::IObservableVector<winrt::hstring> DictionaryNames();
        static Microsoft::UI::Xaml::DependencyProperty DictionaryNamesProperty() { return m_dictionaryNamesProperty; }

        // 新增：当前选中字典的HTML内容 (用于 WebView2)
        // 注意：这个属性的更改将通过其回调函数来触发 WebView2 的 NavigateToString
        winrt::hstring SelectedDictionaryHtml();
        void SelectedDictionaryHtml(winrt::hstring const& value);
        static Microsoft::UI::Xaml::DependencyProperty SelectedDictionaryHtmlProperty() { return m_selectedDictionaryHtmlProperty; }

        // SelectorBar 的事件处理
        void DictionarySelectorBar_SelectionChanged(Microsoft::UI::Xaml::Controls::SelectorBar const& sender, Microsoft::UI::Xaml::Controls::SelectorBarSelectionChangedEventArgs const& args);

        void UpdateDetailVisibility();

        void OnLoaded(winrt::Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& args);
        winrt::fire_and_forget InitializeWebView2Async(); // 异步初始化 WebView2

        // ItemToDisplay 属性更改回调
        static void OnItemToDisplayChanged(Microsoft::UI::Xaml::DependencyObject const& d, Microsoft::UI::Xaml::DependencyPropertyChangedEventArgs const& e);
        // SelectedDictionaryHtml 属性更改回调
        static void OnSelectedDictionaryHtmlChanged(Microsoft::UI::Xaml::DependencyObject const& d, Microsoft::UI::Xaml::DependencyPropertyChangedEventArgs const& e);

    private:
        static Microsoft::UI::Xaml::DependencyProperty m_itemToDisplayProperty;
        // 新增依赖属性的静态成员
        static Microsoft::UI::Xaml::DependencyProperty m_dictionaryNamesProperty;
        static Microsoft::UI::Xaml::DependencyProperty m_selectedDictionaryHtmlProperty;

        // WordSearch 服务实例
        WordWiz::WordSearch m_wordSearchService{ nullptr };

        bool m_isCoreWebView2Initialized{ false };
        winrt::hstring m_pendingHtmlToNavigate{ L"" };

    };
}
namespace winrt::WordWiz::factory_implementation
{
    struct WordDetails : WordDetailsT<WordDetails, implementation::WordDetails>
    {
    };
}