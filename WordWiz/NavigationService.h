#pragma once

#include "pch.h"
#include <functional>
#include <map>
#include <string>
#include <vector>

// 前向声明
namespace WordWizServices::Log
{
    void LogMessage(const std::wstring& message);
    void LogMessage(const wchar_t* message);
}

namespace WordWizServices
{
    struct NavigationEventArgs
    {
        winrt::Microsoft::UI::Xaml::Controls::Frame SourceFrame{ nullptr }; // 发生导航的 Frame
        winrt::Windows::UI::Xaml::Interop::TypeName SourcePageType{};
        winrt::Windows::UI::Xaml::Interop::TypeName TargetPageType{};
        winrt::Windows::Foundation::IInspectable Parameter{ nullptr };
    };

    class NavigationService
    {
    private:
        // 每个 Frame 的导航上下文
        struct FrameContext
        {
            winrt::Microsoft::UI::Xaml::Controls::NavigationView NavigationView{ nullptr }; // 与此 Frame 关联的 NavigationView (可选)
            winrt::Windows::UI::Xaml::Interop::TypeName CurrentPageType{};
            winrt::Microsoft::UI::Xaml::Controls::Frame::Navigated_revoker FrameNavigatedRevoker{};
            winrt::Microsoft::UI::Xaml::Controls::Frame::NavigationFailed_revoker FrameNavigationFailedRevoker{};
            // 不再需要 WindowClosedRevoker，因为我们直接与 Frame 关联
        };

    public:
        NavigationService() = delete; // 静态类，禁止实例化

        // 初始化指定 Frame 的导航服务
        static void Initialize(
            winrt::Microsoft::UI::Xaml::Controls::Frame const& targetFrame,
            winrt::Microsoft::UI::Xaml::Controls::NavigationView const& associatedNavView = nullptr);

        // 反初始化指定 Frame 的导航服务 (例如，当 Frame 不再使用时)
        static void Uninitialize(winrt::Microsoft::UI::Xaml::Controls::Frame const& targetFrame);

        // 判断指定 Frame 的导航服务是否已初始化
        static bool IsInitialized(winrt::Microsoft::UI::Xaml::Controls::Frame const& targetFrame);

        // 设置或更新与指定 Frame 关联的 NavigationView
        static void SetNavigationViewForFrame(
            winrt::Microsoft::UI::Xaml::Controls::Frame const& targetFrame,
            winrt::Microsoft::UI::Xaml::Controls::NavigationView const& navView);

        template <typename PageType>
        static bool NavigateTo(
            winrt::Microsoft::UI::Xaml::Controls::Frame const& targetFrame,
            winrt::Windows::Foundation::IInspectable const& parameter = nullptr,
            winrt::Microsoft::UI::Xaml::Media::Animation::NavigationTransitionInfo const& infoOverride = nullptr)
        {
            if (!targetFrame)
            {
                Log::LogMessage(L"NavigationService: targetFrame cannot be null in NavigateTo.");
                return false;
            }
            auto context = GetFrameContext(targetFrame);
            if (!context) // 如果 Frame 未初始化，则不能导航
            {
                Log::LogMessage(L"NavigationService: Target Frame not initialized for NavigateTo.");
                return false;
            }
            auto targetPageTypeName = winrt::xaml_typename<PageType>();
            return targetFrame.Navigate(targetPageTypeName, parameter, infoOverride);
        }

        static bool NavigateFromTag(
            winrt::Microsoft::UI::Xaml::Controls::Frame const& targetFrame,
            winrt::hstring const& pageTypeNameString,
            winrt::Windows::Foundation::IInspectable const& parameter = nullptr,
            winrt::Microsoft::UI::Xaml::Media::Animation::NavigationTransitionInfo const& infoOverride = nullptr);

        // 页面类型注册是全局的
        template <typename PageType>
        static void RegisterPageTypeForNavViewGlobal()
        {
            auto tn = winrt::xaml_typename<PageType>();
            if (!tn.Name.empty())
            {
                s_knownPageTypesForNavView[tn.Name] = tn;
                Log::LogMessage(L"NavigationService: Registered global NavView page type - " + std::wstring(tn.Name.c_str()));
            }
            else
            {
                Log::LogMessage(L"NavigationService: Failed to register global NavView page type (empty TypeName.Name).");
            }
        }

        static bool AddCurrentPageToHistoryWithData(
            winrt::Microsoft::UI::Xaml::Controls::Frame const& targetFrame,
            winrt::Windows::Foundation::IInspectable const& dataPacket,
            winrt::Microsoft::UI::Xaml::Media::Animation::NavigationTransitionInfo const& infoOverride = nullptr);

        static bool GoBack(winrt::Microsoft::UI::Xaml::Controls::Frame const& targetFrame, winrt::Microsoft::UI::Xaml::Media::Animation::NavigationTransitionInfo const& infoOverride = nullptr);
        static bool GoForward(winrt::Microsoft::UI::Xaml::Controls::Frame const& targetFrame);
        static bool CanGoBack(winrt::Microsoft::UI::Xaml::Controls::Frame const& targetFrame);
        static bool CanGoForward(winrt::Microsoft::UI::Xaml::Controls::Frame const& targetFrame);
        static void ClearNavigationHistory(winrt::Microsoft::UI::Xaml::Controls::Frame const& targetFrame);
        static winrt::Windows::UI::Xaml::Interop::TypeName GetCurrentPageType(winrt::Microsoft::UI::Xaml::Controls::Frame const& targetFrame);

        // 导航事件监听器是全局的，但事件参数会包含 SourceFrame
        static winrt::event_token AddNavigationListener(std::function<void(NavigationEventArgs const&)> const& listener);
        static void RemoveNavigationListener(winrt::event_token const& token);

    private:
        static FrameContext* GetFrameContext(winrt::Microsoft::UI::Xaml::Controls::Frame const& targetFrame, bool createIfNotFound = false);

        // 静态事件处理函数
        static void OnFrameNavigated(
            winrt::Windows::Foundation::IInspectable const& sender, // This will be the Frame
            winrt::Microsoft::UI::Xaml::Navigation::NavigationEventArgs const& e);

        static void OnFrameNavigationFailed(
            winrt::Windows::Foundation::IInspectable const& sender, // This will be the Frame
            winrt::Microsoft::UI::Xaml::Navigation::NavigationFailedEventArgs const& e);

        static void RaiseNavigatedEvent(
            winrt::Microsoft::UI::Xaml::Controls::Frame const& sourceFrame,
            winrt::Windows::UI::Xaml::Interop::TypeName const& sourcePageType,
            winrt::Windows::UI::Xaml::Interop::TypeName const& targetPageType,
            winrt::Windows::Foundation::IInspectable const& parameter);

        static void UpdateNavigationViewSelection(
            winrt::Microsoft::UI::Xaml::Controls::Frame const& targetFrame,
            winrt::Windows::UI::Xaml::Interop::TypeName const& targetPageType);

        // 静态成员变量
        static std::map<winrt::Microsoft::UI::Xaml::Controls::Frame, FrameContext> s_frameContexts;
        static std::map<winrt::hstring, winrt::Windows::UI::Xaml::Interop::TypeName> s_knownPageTypesForNavView; // 全局页面类型

        struct NavigatedListener
        {
            winrt::event_token token;
            std::function<void(NavigationEventArgs const&)> handler;
        };
        static std::vector<NavigatedListener> s_navigationListeners; // 全局监听器
        static int64_t s_nextTokenValue;
    };
}
