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
        winrt::Microsoft::UI::Xaml::Controls::Frame mainFrame = winrt::WordWiz::implementation::MainWindow::GetMainFrame();
        if (mainFrame)
        {
            // 现在可以使用获取到的 mainFrame 进行导航
            WordWizServices::NavigationService::NavigateTo<WordWiz::WordSearchResultPage>(
                mainFrame,
                winrt::box_value(L"文档ID") // 你的导航参数
            );
        }
        else
        {
            // 处理 Frame 未获取到的情况，例如记录错误日志
            WordWizServices::Log::LogMessage(L"HomePage::NavigateToSearchResults: MainWindow's main Frame is not available.");
        }
    }
}
