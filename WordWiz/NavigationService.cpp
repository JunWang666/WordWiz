#include "pch.h"
#include "NavigationService.h"
#include <sstream>

// 静态成员变量定义
std::map<winrt::Microsoft::UI::Xaml::Controls::Frame, WordWizServices::NavigationService::FrameContext> WordWizServices::NavigationService::s_frameContexts;
std::map<winrt::hstring, winrt::Windows::UI::Xaml::Interop::TypeName> WordWizServices::NavigationService::s_knownPageTypesForNavView;
std::vector<WordWizServices::NavigationService::NavigatedListener> WordWizServices::NavigationService::s_navigationListeners;
int64_t WordWizServices::NavigationService::s_nextTokenValue{ 1 };

namespace WordWizServices
{
    // 辅助函数获取或创建 Frame 上下文
    NavigationService::FrameContext* NavigationService::GetFrameContext(winrt::Microsoft::UI::Xaml::Controls::Frame const& targetFrame, bool createIfNotFound)
    {
        if (!targetFrame) return nullptr;

        auto it = s_frameContexts.find(targetFrame);
        if (it != s_frameContexts.end())
        {
            return &it->second;
        }
        else if (createIfNotFound)
        {
            s_frameContexts[targetFrame] = FrameContext{}; // 使用默认构造函数
            Log::LogMessage(L"NavigationService: Created new context for a Frame.");
            return &s_frameContexts[targetFrame];
        }
        return nullptr;
    }

    void NavigationService::Initialize(
        winrt::Microsoft::UI::Xaml::Controls::Frame const& targetFrame,
        winrt::Microsoft::UI::Xaml::Controls::NavigationView const& associatedNavView)
    {
        if (!targetFrame)
        {
            Log::LogMessage(L"NavigationService: targetFrame cannot be null for initialization.");
            return;
        }

        FrameContext* context = GetFrameContext(targetFrame, true); // 获取或创建上下文
        if (!context) {
            Log::LogMessage(L"NavigationService: Failed to get or create frame context during Initialize.");
            return;
        }

        // 撤销旧的事件处理器 (如果重复初始化)
        if (context->FrameNavigatedRevoker) context->FrameNavigatedRevoker.revoke();
        if (context->FrameNavigationFailedRevoker) context->FrameNavigationFailedRevoker.revoke();

        context->NavigationView = associatedNavView; // 可以为 nullptr
        context->CurrentPageType = {}; // 重置当前页面

        // 订阅 Frame 事件
        context->FrameNavigatedRevoker = targetFrame.Navigated(winrt::auto_revoke, &NavigationService::OnFrameNavigated);
        context->FrameNavigationFailedRevoker = targetFrame.NavigationFailed(winrt::auto_revoke, &NavigationService::OnFrameNavigationFailed);

        Log::LogMessage(L"NavigationService: Initialized for a Frame.");
    }

    void NavigationService::Uninitialize(winrt::Microsoft::UI::Xaml::Controls::Frame const& targetFrame)
    {
        if (!targetFrame) return;

        auto it = s_frameContexts.find(targetFrame);
        if (it != s_frameContexts.end())
        {
            // 撤销事件处理器
            if (it->second.FrameNavigatedRevoker) it->second.FrameNavigatedRevoker.revoke();
            if (it->second.FrameNavigationFailedRevoker) it->second.FrameNavigationFailedRevoker.revoke();

            s_frameContexts.erase(it);
            Log::LogMessage(L"NavigationService: Uninitialized and context removed for a Frame.");
        }
        else
        {
            Log::LogMessage(L"NavigationService: Frame not found for Uninitialize.");
        }
    }

    bool NavigationService::IsInitialized(winrt::Microsoft::UI::Xaml::Controls::Frame const& targetFrame)
    {
        return GetFrameContext(targetFrame) != nullptr;
    }

    void NavigationService::SetNavigationViewForFrame(
        winrt::Microsoft::UI::Xaml::Controls::Frame const& targetFrame,
        winrt::Microsoft::UI::Xaml::Controls::NavigationView const& navView)
    {
        if (!targetFrame)
        {
            Log::LogMessage(L"NavigationService: targetFrame cannot be null in SetNavigationViewForFrame.");
            return;
        }
        FrameContext* context = GetFrameContext(targetFrame, true); // 获取或创建
        if (context)
        {
            context->NavigationView = navView;
            Log::LogMessage(L"NavigationService: NavigationView updated for a Frame.");
        }
        else {
            Log::LogMessage(L"NavigationService: Failed to get/create context in SetNavigationViewForFrame.");
        }
    }


    bool NavigationService::NavigateFromTag(
        winrt::Microsoft::UI::Xaml::Controls::Frame const& targetFrame,
        winrt::hstring const& pageTypeNameString,
        winrt::Windows::Foundation::IInspectable const& parameter,
        winrt::Microsoft::UI::Xaml::Media::Animation::NavigationTransitionInfo const& infoOverride)
    {
        if (!targetFrame)
        {
            Log::LogMessage(L"NavigationService: targetFrame cannot be null in NavigateFromTag.");
            return false;
        }
        FrameContext* context = GetFrameContext(targetFrame);
        if (!context)
        {
            Log::LogMessage(L"NavigationService: Target Frame not initialized for NavigateFromTag.");
            return false;
        }
        if (pageTypeNameString.empty())
        {
            Log::LogMessage(L"NavigationService: pageTypeNameString is empty in NavigateFromTag.");
            return false;
        }

        auto it = s_knownPageTypesForNavView.find(pageTypeNameString); // 全局查找
        if (it != s_knownPageTypesForNavView.end())
        {
            Log::LogMessage(L"NavigationService: Navigating from tag to " + std::wstring(pageTypeNameString.c_str()) + L" in specified Frame.");
            return targetFrame.Navigate(it->second, parameter, infoOverride);
        }

        Log::LogMessage(L"NavigationService: PageTypeNameString not registered globally for NavView navigation: " + std::wstring(pageTypeNameString.c_str()));
        return false;
    }

    bool NavigationService::AddCurrentPageToHistoryWithData(
        winrt::Microsoft::UI::Xaml::Controls::Frame const& targetFrame,
        winrt::Windows::Foundation::IInspectable const& dataPacket,
        winrt::Microsoft::UI::Xaml::Media::Animation::NavigationTransitionInfo const& infoOverride)
    {
        if (!targetFrame) return false;
        FrameContext* context = GetFrameContext(targetFrame);
        if (!context)
        {
            Log::LogMessage(L"NavigationService: Target Frame not initialized in AddCurrentPageToHistoryWithData.");
            return false;
        }
        if (context->CurrentPageType.Name.empty())
        {
            Log::LogMessage(L"NavigationService: CurrentPageType is invalid for specified Frame in AddCurrentPageToHistoryWithData.");
            return false;
        }

        Log::LogMessage(L"NavigationService: Adding current page to history with data in specified Frame - " + std::wstring(context->CurrentPageType.Name.c_str()));
        return targetFrame.Navigate(context->CurrentPageType, dataPacket, infoOverride);
    }

    bool NavigationService::GoBack(winrt::Microsoft::UI::Xaml::Controls::Frame const& targetFrame, winrt::Microsoft::UI::Xaml::Media::Animation::NavigationTransitionInfo const& infoOverride)
    {
        if (!targetFrame || !targetFrame.CanGoBack())
        {
            Log::LogMessage(L"NavigationService: Cannot GoBack for the specified Frame.");
            return false;
        }
        Log::LogMessage(L"NavigationService: Going back in specified Frame.");
        targetFrame.GoBack();
        return true;
    }

    bool NavigationService::GoForward(winrt::Microsoft::UI::Xaml::Controls::Frame const& targetFrame)
    {
        if (!targetFrame || !targetFrame.CanGoForward())
        {
            Log::LogMessage(L"NavigationService: Cannot GoForward for the specified Frame.");
            return false;
        }
        Log::LogMessage(L"NavigationService: Going forward in specified Frame.");
        targetFrame.GoForward();
        return true;
    }

    bool NavigationService::CanGoBack(winrt::Microsoft::UI::Xaml::Controls::Frame const& targetFrame)
    {
        return targetFrame && targetFrame.CanGoBack();
    }

    bool NavigationService::CanGoForward(winrt::Microsoft::UI::Xaml::Controls::Frame const& targetFrame)
    {
        return targetFrame && targetFrame.CanGoForward();
    }

    void NavigationService::ClearNavigationHistory(winrt::Microsoft::UI::Xaml::Controls::Frame const& targetFrame)
    {
        if (targetFrame)
        {
            Log::LogMessage(L"NavigationService: Clearing navigation history for specified Frame.");
            auto originalCacheSize = targetFrame.CacheSize();
            targetFrame.CacheSize(0);
            targetFrame.CacheSize(originalCacheSize);
        }
        else
        {
            Log::LogMessage(L"NavigationService: Target Frame is null, cannot clear history.");
        }
    }

    winrt::Windows::UI::Xaml::Interop::TypeName NavigationService::GetCurrentPageType(winrt::Microsoft::UI::Xaml::Controls::Frame const& targetFrame)
    {
        FrameContext* context = GetFrameContext(targetFrame);
        return (context) ? context->CurrentPageType : winrt::Windows::UI::Xaml::Interop::TypeName{};
    }

    winrt::event_token NavigationService::AddNavigationListener(std::function<void(NavigationEventArgs const&)> const& listener)
    {
        winrt::event_token token{ s_nextTokenValue++ };
        s_navigationListeners.push_back({ token, listener });
        Log::LogMessage(L"NavigationService: Added global navigation listener with token " + std::to_wstring(token.value));
        return token;
    }

    void NavigationService::RemoveNavigationListener(winrt::event_token const& token)
    {
        s_navigationListeners.erase(
            std::remove_if(s_navigationListeners.begin(), s_navigationListeners.end(),
                [&](NavigatedListener const& listener_item) { return listener_item.token == token; }),
            s_navigationListeners.end());
        Log::LogMessage(L"NavigationService: Removed global navigation listener with token " + std::to_wstring(token.value));
    }

    void NavigationService::OnFrameNavigated(
        winrt::Windows::Foundation::IInspectable const& sender,
        winrt::Microsoft::UI::Xaml::Navigation::NavigationEventArgs const& e)
    {
        auto navigatedFrame = sender.try_as<winrt::Microsoft::UI::Xaml::Controls::Frame>();
        if (!navigatedFrame) return;

        FrameContext* context = GetFrameContext(navigatedFrame);
        if (!context)
        {
            Log::LogMessage(L"NavigationService: OnFrameNavigated - Could not find context for the navigated Frame.");
            return;
        }

        winrt::Windows::UI::Xaml::Interop::TypeName previousPageType = context->CurrentPageType;
        context->CurrentPageType = e.SourcePageType();

        std::wstringstream logStream;
        logStream << L"NavigationService: Frame navigated. Mode=" << static_cast<int>(e.NavigationMode());
        logStream << L", TargetPage=" << context->CurrentPageType.Name.c_str();
        if (!previousPageType.Name.empty()) {
            logStream << L", FromPage=" << previousPageType.Name.c_str();
        }
        Log::LogMessage(logStream.str());

        UpdateNavigationViewSelection(navigatedFrame, context->CurrentPageType);
        RaiseNavigatedEvent(navigatedFrame, previousPageType, context->CurrentPageType, e.Parameter());
    }

    void NavigationService::OnFrameNavigationFailed(
        winrt::Windows::Foundation::IInspectable const& sender,
        winrt::Microsoft::UI::Xaml::Navigation::NavigationFailedEventArgs const& e)
    {
        auto failedFrame = sender.try_as<winrt::Microsoft::UI::Xaml::Controls::Frame>();
        std::wstringstream ss;
        ss << L"NavigationService: Frame navigation FAILED";
        if (failedFrame) ss << L" (Frame hash: " << std::hash<winrt::Windows::Foundation::IInspectable>{}(failedFrame) << L")"; // 简单标识Frame
        ss << L". Page: " << e.SourcePageType().Name.c_str()
            << L", Exception HRESULT: 0x" << std::hex << e.Exception().value;
        Log::LogMessage(ss.str());
    }

    void NavigationService::RaiseNavigatedEvent(
        winrt::Microsoft::UI::Xaml::Controls::Frame const& sourceFrame,
        winrt::Windows::UI::Xaml::Interop::TypeName const& sourcePageType,
        winrt::Windows::UI::Xaml::Interop::TypeName const& targetPageType,
        winrt::Windows::Foundation::IInspectable const& parameter)
    {
        NavigationEventArgs args;
        args.SourceFrame = sourceFrame;
        args.SourcePageType = sourcePageType;
        args.TargetPageType = targetPageType;
        args.Parameter = parameter;

        for (auto const& listener_item : s_navigationListeners)
        {
            if (listener_item.handler)
            {
                listener_item.handler(args);
            }
        }
    }

    void NavigationService::UpdateNavigationViewSelection(
        winrt::Microsoft::UI::Xaml::Controls::Frame const& targetFrame,
        winrt::Windows::UI::Xaml::Interop::TypeName const& targetPageType)
    {
        FrameContext* context = GetFrameContext(targetFrame);
        if (!context || !context->NavigationView || targetPageType.Name.empty()) // 检查 NavigationView 是否存在
        {
            // 如果没有关联的 NavigationView，或者页面类型无效，则不执行操作
            // 或者，如果 NavigationView 存在但没有匹配项，则清除其选择
            if (context && context->NavigationView) context->NavigationView.SelectedItem(nullptr);
            return;
        }

        winrt::hstring targetPageTypeNameStr = targetPageType.Name;
        winrt::Microsoft::UI::Xaml::Controls::NavigationViewItemBase selectedItem = nullptr;

        auto findAndSetItem = [&](auto const& items) -> bool {
            for (auto const& item : items) {
                if (auto navItem = item.try_as<winrt::Microsoft::UI::Xaml::Controls::NavigationViewItem>()) {
                    if (auto tagInspectable = navItem.Tag()) {
                        if (auto tag = tagInspectable.try_as<winrt::hstring>()) {
                            if (tag == targetPageTypeNameStr) { // 比较 Tag 和全局注册的页面类型名
                                selectedItem = navItem;
                                return true;
                            }
                        }
                    }
                }
            }
            return false;
            };

        bool found = false;
        if (context->NavigationView.MenuItems().Size() > 0) {
            if (findAndSetItem(context->NavigationView.MenuItems())) {
                found = true;
            }
        }
        if (!found && context->NavigationView.FooterMenuItems().Size() > 0) {
            if (findAndSetItem(context->NavigationView.FooterMenuItems())) {
                found = true;
            }
        }
        // 修复 SettingsPage 匹配问题，确保 SettingsItem 被正确选中
        if (!found && context->NavigationView.SettingsItem()) {
            if (auto settingsItem = context->NavigationView.SettingsItem().try_as<winrt::Microsoft::UI::Xaml::Controls::NavigationViewItem>()) {
                // 支持 SettingsPage 名称的多种写法
                if (targetPageTypeNameStr == L"SettingsPage" ||
                    targetPageTypeNameStr == L"WordWiz.SettingsPage" ||
                    (settingsItem.Tag() && settingsItem.Tag().try_as<winrt::hstring>() == targetPageTypeNameStr)) {
                    selectedItem = settingsItem;
                    found = true;
                }
            }
        }

        if (found) {
            // 先清空再设置，避免残影
            context->NavigationView.SelectedItem(selectedItem);
            Log::LogMessage(L"NavigationService: Setting NavigationView selected item for " + std::wstring(targetPageTypeNameStr.c_str()) + L" associated with the Frame.");
        } else {
            context->NavigationView.SelectedItem(nullptr);
            Log::LogMessage(L"NavigationService: No matching NavigationViewItem found for " + std::wstring(targetPageTypeNameStr.c_str()) + L" in NavigationView associated with the Frame. Clearing selection.");
        }
    }
} // namespace WordWizServices
