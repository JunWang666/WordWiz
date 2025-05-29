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

void winrt::WordWiz::implementation::HomePage::TextBox_DragEnter(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::DragEventArgs const& e)
{

}

void winrt::WordWiz::implementation::HomePage::TextBox_KeyDown(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::KeyRoutedEventArgs const& e)  
{  
   if (e.Key() != winrt::Windows::System::VirtualKey::Enter)
   {
       return; // Only process when Enter key is pressed
   }

   winrt::Microsoft::UI::Xaml::Controls::Frame mainFrame = winrt::WordWiz::implementation::MainWindow::GetMainFrame();  
   auto textBox = sender.as<Controls::TextBox>();  
   if (mainFrame)  
   {  
       // Create a parameter map to pass the search word to the result page
       auto map = winrt::single_threaded_map<winrt::hstring, winrt::Windows::Foundation::IInspectable>();  
       // Fix: Pass the actual text value instead of the TextBox control
       map.Insert(L"search_word", winrt::box_value(textBox.Text()));  

       WordWizServices::NavigationService::NavigateTo<WordWiz::WordSearchResultPage>(  
           mainFrame,  
           winrt::box_value(map)  
       );  
   }  
   else  
   {  
       // 处理 Frame 未获取到的情况，例如记录错误日志  
       WordWizServices::Log::LogMessage(L"HomePage::NavigateToSearchResults: MainWindow's main Frame is not available.");  
   }  
}
