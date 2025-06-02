#include "pch.h"
#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#include <winrt/Microsoft.UI.Interop.h> // �ṩ IWindowNative
#include <winrt/Microsoft.UI.h>         // �ṩ GetWindowIdFromWindow
#include <windows.h>                  // �ṩ HWND ����
#include <winrt/Microsoft.UI.Windowing.h> // 提供 AppWindow 和 AppWindowTitleBar
#include <winrt/Microsoft.UI.Xaml.h>     // 提供 UIElement (SetTitleBar 的参数类型)
#include <commctrl.h>                     // 提供 SetWindowSubclass
#endif

#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Composition.SystemBackdrops.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Microsoft.UI.Xaml.Media.Animation.h> // For navigation transition info
// ����ʵ�ֱ����л�
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
        // ��ȡ AppWindow ����
        auto appWindow = GetAppWindowForCurrentWindow();

        if (appWindow)
        {
            // ���ϵͳ�Ƿ�֧�ֱ������Զ��� (ͨ���� Win11 ��֧��)
            if (AppWindowTitleBar::IsCustomizationSupported())
            {
                auto titleBar = appWindow.TitleBar();
                // ��չ���ݵ�����������
                titleBar.ExtendsContentIntoTitleBar(true);
				// ���ñ�����
                this->SetTitleBar(AppTitleBar());
                titleBar.ButtonBackgroundColor(Windows::UI::Colors::Transparent());
            }
            else
            {
                // ���ϵͳ��֧���Զ��壬�������ҪһЩ���˴���
                // ���磬��������Զ�����������ݣ���ϵͳ��ʾĬ�ϱ�����
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
        if (args.SelectedItem() == nullptr) {
            // �հ���������ȡ��ѡ�񣬲����κδ���
            return;
        }

        if (args.IsSettingsSelected())
        {
            // �����Ҫ�����±���Ϊ "����"
            sender.Header(winrt::box_value(L"����")); // ʾ��
        }
        else
        {
            auto selectedItem = args.SelectedItem().try_as<winrt::Microsoft::UI::Xaml::Controls::NavigationViewItem>();
            if (selectedItem)
            {
                // ���±���
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
            // ����������ĵ���
			tag_to_navigate = L"WordWiz.SettingsPage"; // ʾ����ʵ��ֵ����������������   
        }
        else
        {
            // ������ͨ�˵���ĵ���
            // args.InvokedItemContainer() ���ر����õ� NavigationViewItem
            auto invokedItem = args.InvokedItemContainer().try_as<winrt::Microsoft::UI::Xaml::Controls::NavigationViewItem>();
            if (invokedItem)
            {
                // Tag ������ IInspectable����Ҫ���ж�����
                auto tagInspectable = invokedItem.Tag();
                if (auto ref = tagInspectable.try_as<winrt::Windows::Foundation::IReference<winrt::hstring>>())
                {
                    tag_to_navigate = ref.Value();
                }
                else if (auto str = tagInspectable.try_as<winrt::hstring>())
                {
                    tag_to_navigate = str.value();
                }
                // ����Ը�����Ҫ���� else if �ж���������
            }
        }

        if (!tag_to_navigate.empty())
        {
            if (mainFrame) // ȷ����� Frame ʵ����Ч
            {
                WordWizServices::NavigationService::NavigateFromTag(
                    mainFrame,
                    tag_to_navigate,
                    nullptr, // �������� (�����Ҫ�Ļ�)
                    args.RecommendedNavigationTransitionInfo()
                );
            }
            else
            {
                WordWizServices::Log::LogMessage(L"NavigationView_ItemInvoked: m_contentFrame is null! Cannot navigate.");
            }
        }
        else {
            WordWizServices::Log::LogMessage(L"NavigationView����ʧ�ܣ���Ч��Tag��"+ tag_to_navigate);
        }
        // else: tag ��Ч��Ϊ�գ����Ժ��Ի��¼��־
    }


    void MainWindow::BackButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        if (mainFrame && WordWizServices::NavigationService::CanGoBack(mainFrame))
        {
            WordWizServices::NavigationService::GoBack(mainFrame, EntranceNavigationTransitionInfo());
        }
    }

    void MainWindow::CustomPaneToggleButton_Click(IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&)
    {
        SideNavigationView().IsPaneOpen(!SideNavigationView().IsPaneOpen());
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
    }    winrt::Microsoft::UI::Xaml::Controls::Frame MainWindow::GetMainFrame()
    {
        return mainFrame;
    }

    // 窗口子类化处理函数，用于设置最小窗口尺寸
    LRESULT CALLBACK MainWindow::WindowSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
    {
        if (uMsg == WM_GETMINMAXINFO)
        {
            MINMAXINFO* pMinMaxInfo = reinterpret_cast<MINMAXINFO*>(lParam);
            // 设置最小窗口尺寸为 800x600
            pMinMaxInfo->ptMinTrackSize.x = 950;
            pMinMaxInfo->ptMinTrackSize.y = 1000;
            return 0;
        }
        return DefSubclassProc(hWnd, uMsg, wParam, lParam);
    }

}

void winrt::WordWiz::implementation::MainWindow::OnWindowLoaded(
    winrt::Windows::Foundation::IInspectable const& sender,
    winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
    // 设置窗口最小尺寸
    try {
        // 获取当前窗口的HWND
        auto windowNative = this->try_as<IWindowNative>();
        if (windowNative) {
            HWND hwnd;
            windowNative->get_WindowHandle(&hwnd);
            
            if (hwnd) {
                // 设置窗口子类化来处理WM_GETMINMAXINFO消息
                SetWindowSubclass(hwnd, WindowSubclassProc, 0, 0);
               
            }
        }    }
    catch (...) {
        // 忽略窗口尺寸设置错误
    }
    
    // 这里会执行窗口加载后的逻辑
    // 此时主窗体应该已经完全创建，可以安全访问 Frame
    try
    {
        mainFrame = contentFrame();
        // 初始化NavigationService，传入contentFrame和NavigationView
        WordWizServices::NavigationService::Initialize(contentFrame(), SideNavigationView());
    }
    catch (winrt::hresult_error const& ex)
    {
        // 处理错误 - 确保在 ContentFrame 设置过程中的正确
        WordWizServices::Log::LogMessage(L"NavigationService initialization failed: " + std::to_wstring(ex.code()) + L" - " + ex.message());
    }

    //注册页面
    WordWizServices::NavigationService::RegisterPageTypeForNavViewGlobal<WordWiz::HomePage>();
    WordWizServices::NavigationService::RegisterPageTypeForNavViewGlobal<WordWiz::WordSearchResultPage>();
    WordWizServices::NavigationService::RegisterPageTypeForNavViewGlobal<WordWiz::SettingsPage>();
    WordWizServices::NavigationService::RegisterPageTypeForNavViewGlobal<WordWiz::FavoritePage>();
    WordWizServices::NavigationService::RegisterPageTypeForNavViewGlobal<WordWiz::HistoryPage>();

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

void winrt::WordWiz::implementation::MainWindow::BackButton_RightTapped(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::RightTappedRoutedEventArgs const& e)
{
    e.Handled(true);
}
