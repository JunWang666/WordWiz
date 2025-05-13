#include "pch.h"
#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#include <winrt/Microsoft.UI.Interop.h> // 提供 IWindowNative
#include <winrt/Microsoft.UI.h>         // 提供 GetWindowIdFromWindow
#include <windows.h>                  // 提供 HWND 定义
#include <winrt/Microsoft.UI.Windowing.h> // 提供 AppWindow 和 AppWindowTitleBar
#include <winrt/Microsoft.UI.Xaml.h>     // 提供 UIElement (SetTitleBar 的参数类型)
#endif

#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Composition.SystemBackdrops.h>
#include <winrt/Windows.Foundation.h>
// 用于实现背景切换
#include <NavigationService.h>

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::WordWiz::implementation
{
    MainWindow::MainWindow()
    {
        // 获取 AppWindow 对象
        auto appWindow = GetAppWindowForCurrentWindow();

        if (appWindow)
        {
            // 检查系统是否支持标题栏自定义 (通常在 Win11 上支持)
            if (AppWindowTitleBar::IsCustomizationSupported())
            {
                auto titleBar = appWindow.TitleBar();
                // 扩展内容到标题栏区域
                titleBar.ExtendsContentIntoTitleBar(true);
				// 设置标题栏
                this->SetTitleBar(AppTitleBar());
                titleBar.ButtonBackgroundColor(Windows::UI::Colors::Transparent());
            }
            else
            {
                // 如果系统不支持自定义，你可能需要一些回退处理
                // 例如，隐藏你的自定义标题栏内容，让系统显示默认标题栏
                AppTitleBar().Visibility(Visibility::Collapsed);
            }
        }

        
    }

    int32_t MainWindow::MyProperty()
    {
        throw hresult_not_implemented();
    }

    void MainWindow::MyProperty(int32_t /* value */)
    {
        throw hresult_not_implemented();
    }

    void MainWindow::NavigationView_SelectionChanged(const winrt::Microsoft::UI::Xaml::Controls::NavigationView currentNavigationView, const winrt::Microsoft::UI::Xaml::Controls::NavigationViewSelectionChangedEventArgs args)
    {
        if (args.IsSettingsSelected())
        {
            // 如果需要，导航到设置页面
            // contentFrame().Navigate(xaml_typename<WordWiz::SettingsPage>());
        }
        else
        {
            auto selectedItem = args.SelectedItem().as<NavigationViewItem>();
            if (selectedItem)
            {
                winrt::hstring tag = unbox_value<hstring>(selectedItem.Tag());

                if (tag == L"HomePageNavigation")
                {
                    contentFrame().Navigate(xaml_typename<WordWiz::HomePage>());
                }

                // 更新标题
                if (currentNavigationView.PaneDisplayMode() == NavigationViewPaneDisplayMode::Top)
                {
                    currentNavigationView.Header(box_value(selectedItem.Content()));
                }
                else
                {
                    currentNavigationView.Header(nullptr);
                }
            }
        }
    }

    void MainWindow::myButton_Click(IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& args)
    {
		
    }

    winrt::AppWindow MainWindow::GetAppWindowForCurrentWindow()
    {
        // Get access to IWindowNative
        winrt::WordWiz::MainWindow thisWindow = *this;
        winrt::com_ptr<IWindowNative> windowNative = thisWindow.as<IWindowNative>();

        //Get the HWND for the XAML Window
        HWND hWnd;
        windowNative->get_WindowHandle(&hWnd);

        // Get the WindowId for our window
        winrt::WindowId windowId;
        windowId = winrt::GetWindowIdFromWindow(hWnd);

        // Get the AppWindow for the WindowId
        Microsoft::UI::Windowing::AppWindow appWindow = Microsoft::UI::Windowing::AppWindow::GetFromWindowId(windowId);

        return appWindow;
    }
}

void winrt::WordWiz::implementation::MainWindow::OnWindowLoaded(
    winrt::Windows::Foundation::IInspectable const& sender,
    winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
    // 在这里执行窗口加载后的逻辑
    // 此时视觉树应该已完全构建，可以安全访问 Frame
    try
    {
        // 初始化NavigationService，传递contentFrame和NavigationView
        WordWizServices::NavigationService::Initialize(contentFrame(), SideNavigationView());
    }
    catch (winrt::hresult_error const& ex)
    {
        // 处理错误 - 可能是 ContentFrame 属性名不正确
        OutputDebugString((L"Frame access error: " + ex.message() + L"\n").c_str());
    }

    //注册页面
    WordWizServices::NavigationService::RegisterPage<WordWiz::WordSearchResultPage>(L"WordSearchResultPage");
	WordWizServices::NavigationService::RegisterPage<WordWiz::HomePage>(L"HomePage");
}