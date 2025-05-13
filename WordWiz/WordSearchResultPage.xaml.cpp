#include "pch.h"
#include "WordSearchResultPage.xaml.h"
#if __has_include("WordSearchResultPage.g.cpp")
#include "WordSearchResultPage.g.cpp"
#endif
#include <NavigationService.h>

using namespace winrt;
using namespace Microsoft::UI::Xaml;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::WordWiz::implementation
{
    WordSearchResultPage::WordSearchResultPage()
    {
		
    }
    int32_t WordSearchResultPage::MyProperty()
    {
        throw hresult_not_implemented();
    }

    void WordSearchResultPage::MyProperty(int32_t /* value */)
    {
        throw hresult_not_implemented();
    }

    void WordSearchResultPage::myButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        myButton().Content(box_value(L"Clicked"));
    }

    void WordSearchResultPage::OnNavigatedTo(Microsoft::UI::Xaml::Navigation::NavigationEventArgs const& e)
    {
        // 获取传递的参数并设置按钮内容
        auto param = e.Parameter();
        if (param)
        {
            try
            {
                // 使用 unbox_value 正确解包参数
                winrt::hstring str = winrt::unbox_value<winrt::hstring>(param);
                myButton().Content(winrt::box_value(str));
            }
            catch (winrt::hresult_error const& ex)
            {
                // 参数类型不是预期的 hstring
                // 可以在这里添加错误处理或日志记录
            }
        }
    }
}
