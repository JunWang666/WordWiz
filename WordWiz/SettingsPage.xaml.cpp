#include "pch.h"
#include "SettingsPage.xaml.h"
#if __has_include("SettingsPage.g.cpp")
#include "SettingsPage.g.cpp"
#endif
#include "SettingsData.h"
#include "GeneralSettingsPage.xaml.h"
#include "DictionarySettingsPage.xaml.h"

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::WordWiz::implementation
{
    SettingsPage::SettingsPage()
    {
        InitializeComponent();
        
        // 默认选择第一个项目（常规）
        if (SettingsNavigationView().MenuItems().Size() > 0) {
            SettingsNavigationView().SelectedItem(SettingsNavigationView().MenuItems().GetAt(0));
            NavigateToPage(L"General");
        }
    }
    int32_t SettingsPage::MyProperty()
    {
        throw hresult_not_implemented();
    }

    void SettingsPage::MyProperty(int32_t /* value */)
    {
        throw hresult_not_implemented();
    }    void SettingsPage::myButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        // 这个方法现在不再使用，因为我们已经移除了测试按钮
        // 保留空实现以避免编译错误
    }

    void SettingsPage::SettingsNavigationView_SelectionChanged(IInspectable const& sender, NavigationViewSelectionChangedEventArgs const& args)
    {
        if (auto selectedItem = args.SelectedItem()) {
            if (auto navItem = selectedItem.try_as<NavigationViewItem>()) {
                auto tag = winrt::unbox_value_or<winrt::hstring>(navItem.Tag(), L"");
                NavigateToPage(tag);
            }
        }
    }

    void SettingsPage::NavigateToPage(winrt::hstring const& tag)
    {
        if (tag == L"General") {
            SettingsContentFrame().Navigate(winrt::xaml_typename<WordWiz::GeneralSettingsPage>());
        }
        else if (tag == L"Dictionary") {
            SettingsContentFrame().Navigate(winrt::xaml_typename<WordWiz::DictionarySettingsPage>());
        }
    }
}
