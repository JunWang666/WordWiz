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
                // 扩展内容到标题栏区域
                appWindow.TitleBar().ExtendsContentIntoTitleBar(true);
				// 设置标题栏
                this->SetTitleBar(AppTitleBar());
            }
            else
            {
                // 如果系统不支持自定义，你可能需要一些回退处理
                // 例如，隐藏你的自定义标题栏内容，让系统显示默认标题栏
                AppTitleBar().Visibility(Visibility::Collapsed);
            }
        }
    }

    winrt::AppWindow MainWindow::MyAppWindow()
    {
        return m_mainAppWindow;
    }

    int32_t MainWindow::MyProperty()
    {
        throw hresult_not_implemented();
    }

    void MainWindow::MyProperty(int32_t /* value */)
    {
        throw hresult_not_implemented();
    }

    void MainWindow::myButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        myButton().Content(box_value(L"Clicked"));
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
