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
#include <winrt/Microsoft.UI.Xaml.Media.Animation.h> // For navigation transition info
// 用于实现背景切换
#include <NavigationService.h>

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Microsoft::UI::Xaml::Media::Animation;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::WordWiz::implementation
{
    winrt::Microsoft::UI::Xaml::Controls::Frame MainWindow::mainFrame{ nullptr };

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

    void MainWindow::NavigationView_SelectionChanged(const winrt::Microsoft::UI::Xaml::Controls::NavigationView sender, const winrt::Microsoft::UI::Xaml::Controls::NavigationViewSelectionChangedEventArgs args)
    {
        if (args.IsSettingsSelected())
        {
            // 如果需要，更新标题为 "设置"
            sender.Header(winrt::box_value(L"设置")); // 示例
        }
        else
        {
            auto selectedItem = args.SelectedItem().try_as<winrt::Microsoft::UI::Xaml::Controls::NavigationViewItem>();
            if (selectedItem)
            {
                // 更新标题
                if (sender.PaneDisplayMode() == winrt::Microsoft::UI::Xaml::Controls::NavigationViewPaneDisplayMode::Top)
                {
                    sender.Header(selectedItem.Content());
                }
                else
                {
                    sender.Header(nullptr);
                }
            }
        }
    }

    void MainWindow::NavigationView_ItemInvoked(winrt::Microsoft::UI::Xaml::Controls::NavigationView const& sender, winrt::Microsoft::UI::Xaml::Controls::NavigationViewItemInvokedEventArgs const& args)
    {
        winrt::hstring tag_to_navigate;

        if (args.IsSettingsInvoked())
        {
            // 处理设置项的调用
        }
        else
        {
            // 处理普通菜单项的调用
            // args.InvokedItemContainer() 返回被调用的 NavigationViewItem
            auto invokedItem = args.InvokedItemContainer().try_as<winrt::Microsoft::UI::Xaml::Controls::NavigationViewItem>();
            if (invokedItem)
            {
                // Tag 可能是 IInspectable，需要先判断类型
                auto tagInspectable = invokedItem.Tag();
                if (auto ref = tagInspectable.try_as<winrt::Windows::Foundation::IReference<winrt::hstring>>())
                {
                    tag_to_navigate = ref.Value();
                }
                else if (auto str = tagInspectable.try_as<winrt::hstring>())
                {
                    tag_to_navigate = str.value();
                }
                // 你可以根据需要继续 else if 判断其他类型
            }
        }

        if (!tag_to_navigate.empty())
        {
            if (mainFrame) // 确保你的 Frame 实例有效
            {
                WordWizServices::NavigationService::NavigateFromTag(
                    mainFrame,
                    tag_to_navigate,
                    nullptr, // 导航参数 (如果需要的话)
                    args.RecommendedNavigationTransitionInfo()
                );
            }
            else
            {
                WordWizServices::Log::LogMessage(L"NavigationView_ItemInvoked: m_contentFrame is null! Cannot navigate.");
            }
        }
        // else: tag 无效或为空，可以忽略或记录日志
    }

    void MainWindow::myButton_Click(IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& args)
    {
		
    }

    void MainWindow::BackButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        if (mainFrame && WordWizServices::NavigationService::CanGoBack(mainFrame))
        {
            WordWizServices::NavigationService::GoBack(mainFrame, EntranceNavigationTransitionInfo());
        }
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

    winrt::Microsoft::UI::Xaml::Controls::Frame MainWindow::GetMainFrame()
    {
        return mainFrame;
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
        mainFrame = contentFrame();
        // 初始化NavigationService，传递contentFrame和NavigationView
        WordWizServices::NavigationService::Initialize(contentFrame(), SideNavigationView());
    }
    catch (winrt::hresult_error const& ex)
    {
        // 处理错误 - 可能是 ContentFrame 属性名不正确
		WordWizServices::Log::LogMessage(L"NavigationService initialization failed: " + std::to_wstring(ex.code()) + L" - " + ex.message());
    }

    //注册页面
    WordWizServices::NavigationService::RegisterPageTypeForNavViewGlobal<WordWiz::HomePage>();
    WordWizServices::NavigationService::RegisterPageTypeForNavViewGlobal<WordWiz::WordSearchResultPage>();

    WordWizServices::NavigationService::NavigateTo<WordWiz::HomePage>(MainWindow::GetMainFrame());


    // Update back button enabled state on navigation
    auto updateBackButtonEnabled = [this]()
    {
        if (mainFrame && BackButton())
        {
            BackButton().IsEnabled(
                WordWizServices::NavigationService::CanGoBack(mainFrame)
            );
        }
    };

    // Register navigation listener to update back button
    static auto navToken = WordWizServices::NavigationService::AddNavigationListener(
        [updateBackButtonEnabled](auto&&...) 
        { 
            updateBackButtonEnabled(); 
        }
    );

    updateBackButtonEnabled(); // Initial state
}