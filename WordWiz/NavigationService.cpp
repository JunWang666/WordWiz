// NavigationService.cpp
#include "pch.h"
#include "NavigationService.h"

namespace WordWizServices
{
    // 初始化导航服务
    void NavigationService::Initialize(winrt::Microsoft::UI::Xaml::Controls::Frame frame)
    {
        m_frame = frame;
    }

    // 判断导航服务是否已初始化
    bool NavigationService::IsInitialized()
    {
        return m_frame != nullptr;
    }

    // 获取当前Frame
    winrt::Microsoft::UI::Xaml::Controls::Frame NavigationService::GetFrame()
    {
        return m_frame;
    }

    // 导航到页面 - 通过页面名称
    bool NavigationService::NavigateTo(std::wstring_view pageName, winrt::Windows::Foundation::IInspectable parameter)
    {
        if (!m_frame) return false;

        // 获取当前页面名称
        std::wstring sourcePage = m_currentPageName;

        auto it = m_pageTypes.find(std::wstring(pageName));
        if (it != m_pageTypes.end())
        {
            bool result = m_frame.Navigate(it->second, parameter);
            if (result)
            {
                // 更新当前页面名称
                m_currentPageName = std::wstring(pageName);

                // 触发导航完成事件
                OnNavigated(sourcePage, m_currentPageName, parameter);
            }
            return result;
        }

        WordWizServices::LogMessage(L"Page not registered: " + std::wstring(pageName));

        // 打印当前注册的页面类型
        for (const auto& pair : m_pageTypes)
        {
            WordWizServices::LogMessage(L"Registered page: " + pair.first);
        }

        return false;
    }

    // 返回上一页
    bool NavigationService::GoBack()
    {
        if (!m_frame || !m_frame.CanGoBack()) return false;

        m_frame.GoBack();

        // 更新当前页面名称（复杂度较高，这里简化处理）
        UpdateCurrentPageName();

        return true;
    }

    // 前进
    bool NavigationService::GoForward()
    {
        if (!m_frame || !m_frame.CanGoForward()) return false;

        m_frame.GoForward();

        // 更新当前页面名称
        UpdateCurrentPageName();

        return true;
    }

    // 判断是否可以返回
    bool NavigationService::CanGoBack()
    {
        return m_frame && m_frame.CanGoBack();
    }

    // 判断是否可以前进
    bool NavigationService::CanGoForward()
    {
        return m_frame && m_frame.CanGoForward();
    }

    // 清除导航历史
    void NavigationService::ClearNavigationHistory()
    {
        if (m_frame)
        {
            auto cacheSize = m_frame.CacheSize();
            m_frame.CacheSize(0);
            m_frame.CacheSize(cacheSize);
        }
    }

    // 获取当前页面名称
    std::wstring NavigationService::GetCurrentPageName()
    {
        return m_currentPageName;
    }

    // 添加导航事件监听
    void NavigationService::AddNavigationListener(std::function<void(NavigationEventArgs)> listener)
    {
        m_navigationListeners.push_back(listener);
    }

    // 清除导航事件监听
    void NavigationService::ClearNavigationListeners()
    {
        m_navigationListeners.clear();
    }

    // 更新当前页面名称
    void NavigationService::UpdateCurrentPageName()
    {
        if (!m_frame || !m_frame.Content()) return;

        auto content = m_frame.Content();
        auto typeName = winrt::get_class_name(content);

        // 尝试从类型找到页面名称
        for (const auto& pair : m_pageTypes)
        {
            // Compare the class name of the content with the TypeName.Name of the registered type
            if (typeName == pair.second.Name)
            {
                m_currentPageName = pair.first;
                break;
            }
        }
    }

    // 触发导航完成事件
    void NavigationService::OnNavigated(const std::wstring& sourcePage, const std::wstring& targetPage,
        winrt::Windows::Foundation::IInspectable parameter)
    {
        NavigationEventArgs args;
        args.SourcePageName = sourcePage;
        args.TargetPageName = targetPage;
        args.Parameter = parameter;

        // 通知所有监听器
        for (const auto& listener : m_navigationListeners)
        {
            listener(args);
        }
    }
}