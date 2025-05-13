#include "pch.h"
#include "HomePage.xaml.h"
#if __has_include("HomePage.g.cpp")
#include "HomePage.g.cpp"
#endif
#include<NavigationService.h>

using namespace winrt;
using namespace Microsoft::UI::Xaml;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::WordWiz::implementation
{
    int32_t HomePage::MyProperty()
    {
        throw hresult_not_implemented();
    }

    void HomePage::MyProperty(int32_t /* value */)
    {
        throw hresult_not_implemented();
    }

    void HomePage::myButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        // 传递 WinRT 类型参数（hstring）
        WordWizServices::NavigationService::NavigateTo(
            L"WordSearchResultPage",
            winrt::box_value(L"文档ID") // 使用 hstring
        );
    }
}
