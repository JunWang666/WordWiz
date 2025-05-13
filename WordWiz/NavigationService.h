// NavigationService.h
#pragma once

#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Navigation.h>
#include <winrt/Windows.UI.Xaml.Interop.h>
#include <map>
#include <string>
#include <vector>
#include <functional>
#include <logger.h>

namespace WordWizServices
{
    // 导航事件参数
    struct NavigationEventArgs
    {
        std::wstring SourcePageName;
        std::wstring TargetPageName;
        winrt::Windows::Foundation::IInspectable Parameter;
    };

    // 导航服务类 - 用于管理应用的页面导航
    class NavigationService
    {
    public:
        // 初始化导航服务
        static void Initialize(winrt::Microsoft::UI::Xaml::Controls::Frame frame);
        
        // 扩展初始化方法，包含NavigationView
        static void Initialize(winrt::Microsoft::UI::Xaml::Controls::Frame frame, 
                             winrt::Microsoft::UI::Xaml::Controls::NavigationView navView);

        // 判断导航服务是否已初始化
        static bool IsInitialized();

        // 获取当前Frame
        static winrt::Microsoft::UI::Xaml::Controls::Frame GetFrame();

        // 注册页面类型 - 通过指定类型和名称
        template<typename T>
        static void RegisterPage(std::wstring_view pageName)
        {
            m_pageTypes[std::wstring(pageName)] = winrt::xaml_typename<T>();
        }

        // 导航到页面 - 通过页面名称
        static bool NavigateTo(std::wstring_view pageName, winrt::Windows::Foundation::IInspectable parameter = nullptr);

        // 通过类型直接导航
        template<typename T>
        static bool NavigateToType(winrt::Windows::Foundation::IInspectable parameter = nullptr)
        {
            if (!m_frame) return false;

            // 获取当前页面名称和目标页面名称
            std::wstring sourcePage = m_currentPageName;
            std::wstring targetPage;

            // 尝试从注册表中查找类型对应的名称
            for (const auto& pair : m_pageTypes)
            {
                if (pair.second == winrt::xaml_typename<T>())
                {
                    targetPage = pair.first;
                    break;
                }
            }

            bool result = m_frame.Navigate(winrt::xaml_typename<T>(), parameter);
            if (result && !targetPage.empty())
            {
                m_currentPageName = targetPage;
                OnNavigated(sourcePage, targetPage, parameter);
            }
            return result;
        }

        // 返回上一页
        static bool GoBack();

        // 前进
        static bool GoForward();

        // 判断是否可以返回
        static bool CanGoBack();

        // 判断是否可以前进
        static bool CanGoForward();

        // 清除导航历史
        static void ClearNavigationHistory();

        // 获取当前页面名称
        static std::wstring GetCurrentPageName();

        // 添加导航事件监听
        static void AddNavigationListener(std::function<void(NavigationEventArgs)> listener);

        // 清除导航事件监听
        static void ClearNavigationListeners();

        // 清除NavigationView的选中状态
        static void ClearNavigationViewSelection();

    private:
        // 更新当前页面名称
        static void UpdateCurrentPageName();

        // 触发导航完成事件
        static void OnNavigated(const std::wstring& sourcePage, const std::wstring& targetPage,
            winrt::Windows::Foundation::IInspectable parameter);

        // 静态成员变量
        inline static winrt::Microsoft::UI::Xaml::Controls::Frame m_frame{ nullptr };
        inline static winrt::Microsoft::UI::Xaml::Controls::NavigationView m_navigationView{ nullptr };
        inline static std::map<std::wstring, winrt::Windows::UI::Xaml::Interop::TypeName> m_pageTypes{};
        inline static std::wstring m_currentPageName{};
        inline static std::vector<std::function<void(NavigationEventArgs)>> m_navigationListeners{};
    };
}