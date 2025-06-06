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

	void HomePage::myButton_Click(const IInspectable&, const RoutedEventArgs&)
	{
		Controls::Frame mainFrame = MainWindow::GetMainFrame();
		if (mainFrame)
		{
			// 现在可以使用获取到的 mainFrame 进行导航
			WordWizServices::NavigationService::NavigateTo<WordSearchResultPage>(
				mainFrame,
				box_value(L"文档ID") // 你的导航参数
			);
		}
		else
		{
			// 处理 Frame 未获取到的情况，例如记录错误日志
			WordWizServices::Log::LogMessage(
				L"HomePage::NavigateToSearchResults: MainWindow's main Frame is not available.");
		}
	}
}

void WordWiz::implementation::HomePage::TextBox_DragEnter(const Windows::Foundation::IInspectable& sender,
                                                          const DragEventArgs& e)
{
}

void WordWiz::implementation::HomePage::TextBox_KeyDown(const Windows::Foundation::IInspectable& sender,
                                                        const Input::KeyRoutedEventArgs& e)
{
	if (e.Key() != Windows::System::VirtualKey::Enter)
	{
		return; // Only process when Enter key is pressed
	}

	Controls::Frame mainFrame = MainWindow::GetMainFrame();
	auto textBox = sender.as<TextBox>();
	if (mainFrame)
	{
		// Create a parameter map to pass the search word to the result page
		auto map = winrt::single_threaded_map<hstring, Windows::Foundation::IInspectable>();
		// Fix: Pass the actual text value instead of the TextBox control
		map.Insert(L"search_word", box_value(textBox.Text()));

		WordWizServices::NavigationService::NavigateTo<WordSearchResultPage>(
			mainFrame,
			box_value(map)
		);
	}
	else
	{
		// 处理 Frame 未获取到的情况，例如记录错误日志  
		WordWizServices::Log::LogMessage(
			L"HomePage::NavigateToSearchResults: MainWindow's main Frame is not available.");
	}
}
