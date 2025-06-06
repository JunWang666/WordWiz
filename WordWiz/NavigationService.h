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
		winrt::Microsoft::UI::Xaml::Controls::Frame SourceFrame{nullptr}; // 发生导航的 Frame
		winrt::Windows::UI::Xaml::Interop::TypeName SourcePageType{};
		winrt::Windows::UI::Xaml::Interop::TypeName TargetPageType{};
		winrt::Windows::Foundation::IInspectable Parameter{nullptr};
	};

	class NavigationService
	{
		// 每个 Frame 的导航上下文
		struct FrameContext
		{
			winrt::Microsoft::UI::Xaml::Controls::NavigationView NavigationView{nullptr};
			// 与此 Frame 关联的 NavigationView (可选)
			winrt::Windows::UI::Xaml::Interop::TypeName CurrentPageType{};
			winrt::Microsoft::UI::Xaml::Controls::Frame::Navigated_revoker FrameNavigatedRevoker{};
			winrt::Microsoft::UI::Xaml::Controls::Frame::NavigationFailed_revoker FrameNavigationFailedRevoker{};
			// 不再需要 WindowClosedRevoker，因为我们直接与 Frame 关联
		};

	public:
		NavigationService() = delete; // 静态类，禁止实例化

		// 初始化指定 Frame 的导航服务
		static void Initialize(
			const winrt::Microsoft::UI::Xaml::Controls::Frame& targetFrame,
			const winrt::Microsoft::UI::Xaml::Controls::NavigationView& associatedNavView = nullptr);

		// 反初始化指定 Frame 的导航服务 (例如，当 Frame 不再使用时)
		static void Uninitialize(const winrt::Microsoft::UI::Xaml::Controls::Frame& targetFrame);

		// 判断指定 Frame 的导航服务是否已初始化
		static bool IsInitialized(const winrt::Microsoft::UI::Xaml::Controls::Frame& targetFrame);

		// 设置或更新与指定 Frame 关联的 NavigationView
		static void SetNavigationViewForFrame(
			const winrt::Microsoft::UI::Xaml::Controls::Frame& targetFrame,
			const winrt::Microsoft::UI::Xaml::Controls::NavigationView& navView);

		template <typename PageType>
		static bool NavigateTo(
			const winrt::Microsoft::UI::Xaml::Controls::Frame& targetFrame,
			const winrt::Windows::Foundation::IInspectable& parameter = nullptr,
			const winrt::Microsoft::UI::Xaml::Media::Animation::NavigationTransitionInfo& infoOverride = nullptr)
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
			const winrt::Microsoft::UI::Xaml::Controls::Frame& targetFrame,
			const winrt::hstring& pageTypeNameString,
			const winrt::Windows::Foundation::IInspectable& parameter = nullptr,
			const winrt::Microsoft::UI::Xaml::Media::Animation::NavigationTransitionInfo& infoOverride = nullptr);

		// 页面类型注册是全局的
		template <typename PageType>
		static void RegisterPageTypeForNavViewGlobal()
		{
			auto tn = winrt::xaml_typename<PageType>();
			if (!tn.Name.empty())
			{
				s_knownPageTypesForNavView[tn.Name] = tn;
				Log::LogMessage(
					L"NavigationService: Registered global NavView page type - " + std::wstring(tn.Name.c_str()));
			}
			else
			{
				Log::LogMessage(
					L"NavigationService: Failed to register global NavView page type (empty TypeName.Name).");
			}
		}

		static bool AddCurrentPageToHistoryWithData(
			const winrt::Microsoft::UI::Xaml::Controls::Frame& targetFrame,
			const winrt::Windows::Foundation::IInspectable& dataPacket,
			const winrt::Microsoft::UI::Xaml::Media::Animation::NavigationTransitionInfo& infoOverride = nullptr);

		static bool GoBack(const winrt::Microsoft::UI::Xaml::Controls::Frame& targetFrame,
		                   const winrt::Microsoft::UI::Xaml::Media::Animation::NavigationTransitionInfo& infoOverride =
			                   nullptr);
		static bool GoForward(const winrt::Microsoft::UI::Xaml::Controls::Frame& targetFrame);
		static bool CanGoBack(const winrt::Microsoft::UI::Xaml::Controls::Frame& targetFrame);
		static bool CanGoForward(const winrt::Microsoft::UI::Xaml::Controls::Frame& targetFrame);
		static void ClearNavigationHistory(const winrt::Microsoft::UI::Xaml::Controls::Frame& targetFrame);
		static winrt::Windows::UI::Xaml::Interop::TypeName GetCurrentPageType(
			const winrt::Microsoft::UI::Xaml::Controls::Frame& targetFrame);

		// 导航事件监听器是全局的，但事件参数会包含 SourceFrame
		static winrt::event_token
		AddNavigationListener(const std::function<void(const NavigationEventArgs&)>& listener);
		static void RemoveNavigationListener(const winrt::event_token& token);

	private:
		static FrameContext* GetFrameContext(const winrt::Microsoft::UI::Xaml::Controls::Frame& targetFrame,
		                                     bool createIfNotFound = false);

		// 静态事件处理函数
		static void OnFrameNavigated(
			const winrt::Windows::Foundation::IInspectable& sender, // This will be the Frame
			const winrt::Microsoft::UI::Xaml::Navigation::NavigationEventArgs& e);

		static void OnFrameNavigationFailed(
			const winrt::Windows::Foundation::IInspectable& sender, // This will be the Frame
			const winrt::Microsoft::UI::Xaml::Navigation::NavigationFailedEventArgs& e);

		static void RaiseNavigatedEvent(
			const winrt::Microsoft::UI::Xaml::Controls::Frame& sourceFrame,
			const winrt::Windows::UI::Xaml::Interop::TypeName& sourcePageType,
			const winrt::Windows::UI::Xaml::Interop::TypeName& targetPageType,
			const winrt::Windows::Foundation::IInspectable& parameter);

		static void UpdateNavigationViewSelection(
			const winrt::Microsoft::UI::Xaml::Controls::Frame& targetFrame,
			const winrt::Windows::UI::Xaml::Interop::TypeName& targetPageType);

		// 静态成员变量
		static std::map<winrt::Microsoft::UI::Xaml::Controls::Frame, FrameContext> s_frameContexts;
		static std::map<winrt::hstring, winrt::Windows::UI::Xaml::Interop::TypeName> s_knownPageTypesForNavView;
		// 全局页面类型

		struct NavigatedListener
		{
			winrt::event_token token;
			std::function<void(const NavigationEventArgs&)> handler;
		};

		static std::vector<NavigatedListener> s_navigationListeners; // 全局监听器
		static int64_t s_nextTokenValue;
	};
}
