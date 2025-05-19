// WordDetails.xaml.cpp
#include "pch.h"
#include "WordDetails.xaml.h"
#include "WordDetails.g.cpp"
#include "WordItem.h" 
#include "NavigationService.h" // 新增：引入NavigationService实现

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls; // For SelectorBar related types
using namespace Windows::Foundation::Collections;
using namespace WordWizServices; // 新增：使用NavigationService命名空间

namespace winrt::WordWiz::implementation
{
    // ItemToDisplayProperty (已存在)
    Microsoft::UI::Xaml::DependencyProperty WordDetails::m_itemToDisplayProperty =
        Microsoft::UI::Xaml::DependencyProperty::Register(
            L"ItemToDisplay",
            xaml_typename<WordWiz::WordItem>(),
            xaml_typename<WordWiz::WordDetails>(),
            Microsoft::UI::Xaml::PropertyMetadata{ nullptr, PropertyChangedCallback(&WordDetails::OnItemToDisplayChanged) } // 添加回调
        );

    // 新增 DictionaryNamesProperty
    Microsoft::UI::Xaml::DependencyProperty WordDetails::m_dictionaryNamesProperty =
        Microsoft::UI::Xaml::DependencyProperty::Register(
            L"DictionaryNames",
            xaml_typename<IObservableVector<winrt::hstring>>(),
            xaml_typename<WordWiz::WordDetails>(),
            Microsoft::UI::Xaml::PropertyMetadata{ nullptr }
        );

    // 新增 SelectedDictionaryHtmlProperty
    Microsoft::UI::Xaml::DependencyProperty WordDetails::m_selectedDictionaryHtmlProperty =
        Microsoft::UI::Xaml::DependencyProperty::Register(
            L"SelectedDictionaryHtml",
            xaml_typename<winrt::hstring>(),
            xaml_typename<WordWiz::WordDetails>(),
            Microsoft::UI::Xaml::PropertyMetadata{ winrt::box_value(L""), PropertyChangedCallback(&WordDetails::OnSelectedDictionaryHtmlChanged) } // 初始值为空字符串，并添加回调
        );

    WordDetails::WordDetails()
    {
        InitializeComponent();
        // 初始化 WordSearch 服务
        m_wordSearchService = winrt::make<WordWiz::implementation::WordSearch>();
        // 初始化 DictionaryNames 为一个空的 observable vector
        SetValue(m_dictionaryNamesProperty, winrt::single_threaded_observable_vector<winrt::hstring>());
    }

    void WordDetails::OnLoaded(IInspectable const& /*sender*/, RoutedEventArgs const& /*args*/)
    {
        InitializeWebView2Async(); // 控件加载后开始初始化 WebView2
        UpdateDetailVisibility(); // 更新可见性
    }

    // 新增：异步初始化 WebView2 的方法
    winrt::fire_and_forget WordDetails::InitializeWebView2Async()
    {
        auto strong_this{ get_strong() }; // 在协程中安全使用 this
        try
        {
            if (strong_this->DictionaryWebView()) // 确保 DictionaryWebView 控件存在
            {
                // Debug输出，确认调用
                // OutputDebugString(L"InitializeWebView2Async: Calling EnsureCoreWebView2Async...\n");
                co_await strong_this->DictionaryWebView().EnsureCoreWebView2Async();
                
                
                strong_this->m_isCoreWebView2Initialized = true;
                // OutputDebugString(L"InitializeWebView2Async: CoreWebView2 Initialized.\n");

                // 如果有待处理的HTML，现在加载它
                if (strong_this->m_isCoreWebView2Initialized && !strong_this->m_pendingHtmlToNavigate.empty())
                {
                    // OutputDebugString((L"InitializeWebView2Async: Navigating to pending HTML: " + strong_this->m_pendingHtmlToNavigate + L"\n").c_str());
                    strong_this->DictionaryWebView().NavigateToString(strong_this->m_pendingHtmlToNavigate);
                    strong_this->m_pendingHtmlToNavigate = L""; // 清除待处理的HTML
                }
            }
        }
        catch (winrt::hresult_error const& ex)
        {
            // 记录或处理 CoreWebView2 初始化失败的情况
            winrt::hstring errorMessage = ex.message();
            // OutputDebugString((L"InitializeWebView2Async: CoreWebView2 Init Error: " + errorMessage + L"\n").c_str());
            // 在UI上显示错误，或者进行其他错误处理
        }
    }

    // ItemToDisplay Getter/Setter (已存在)
    WordWiz::WordItem WordDetails::ItemToDisplay()
    {
        return GetValue(m_itemToDisplayProperty).try_as<WordWiz::WordItem>();
    }
    void WordDetails::ItemToDisplay(WordWiz::WordItem const& value)
    {
        SetValue(m_itemToDisplayProperty, value);
    }

    // DictionaryNames Getter
    IObservableVector<winrt::hstring> WordDetails::DictionaryNames()
    {
        return GetValue(m_dictionaryNamesProperty).try_as<IObservableVector<winrt::hstring>>();
    }

    // SelectedDictionaryHtml Getter/Setter
    winrt::hstring WordDetails::SelectedDictionaryHtml()
    {
        return unbox_value<winrt::hstring>(GetValue(m_selectedDictionaryHtmlProperty));
    }
    void WordDetails::SelectedDictionaryHtml(winrt::hstring const& value)
    {
        SetValue(m_selectedDictionaryHtmlProperty, box_value(value));
    }

    void WordDetails::OnItemToDisplayChanged(Microsoft::UI::Xaml::DependencyObject const& d, Microsoft::UI::Xaml::DependencyPropertyChangedEventArgs const& e)
    {
        if (auto SendersThis{ d.try_as<WordDetails>() }) // 将 d 转换为 WordDetails 实例指针
        {
            auto newItem = e.NewValue().try_as<WordWiz::WordItem>(); // 获取新的 WordItem

            // 新增：通过NavigationService记录历史
            if (SendersThis->m_hostFrame && newItem && !newItem.Word().empty())
            {
                // 这里将WordItem作为参数传递，infoOverride可用默认
                NavigationService::AddCurrentPageToHistoryWithData(
                    SendersThis->m_hostFrame,
                    newItem,
                    nullptr // 可根据需要传递NavigationTransitionInfo
                );
            }

            // 清理 SelectorBar 中的旧项目
            // 假设你的 SelectorBar 在 XAML 中的 x:Name 是 DictionarySelectorBar
            // 并且 C++/WinRT 为你生成了 DictionarySelectorBar() 成员函数来访问它
            if (SendersThis->DictionarySelectorBar()) // 检查控件是否有效
            {
                SendersThis->DictionarySelectorBar().Items().Clear();
            }

            // 清理可能存在的旧的字典名列表（如果你的 DictionaryNames 属性仍在使用）
            // if (SendersThis->DictionaryNames()) SendersThis->DictionaryNames().Clear();


            if (newItem) // 如果新的 WordItem 有效
            {
                winrt::hstring word = newItem.Word(); // 获取单词

                // 调用 WordSearch 服务获取可用词典列表
                // !!! 再次确认 m_wordSearchService 已初始化，并且 GetAvailableDictionaries 方法已在 WordSearch.idl, .h, .cpp 中正确声明和实现 !!!
                auto availableDictionaries = SendersThis->m_wordSearchService.GetAvailableDictionaries(word);

                if (SendersThis->DictionarySelectorBar()) // 再次检查
                {
                    for (auto const& dictName : availableDictionaries)
                    {
                        SelectorBarItem sbItem;          // 创建新的 SelectorBarItem
                        sbItem.Text(dictName);           // 设置其文本
                        SendersThis->DictionarySelectorBar().Items().Append(sbItem); // 添加到 SelectorBar
                    }
                }

                if (SendersThis->DictionarySelectorBar() && SendersThis->DictionarySelectorBar().Items().Size() > 0)
                {
                    // 自动选中第一个词典项
                    // 注意：直接设置 SelectedItem 通常会触发 SelectionChanged 事件，
                    // 在该事件的处理函数中我们会加载HTML内容到 WebView2
                    SendersThis->DictionarySelectorBar().SelectedItem(
                        SendersThis->DictionarySelectorBar().Items().GetAt(0).try_as<SelectorBarItem>()
                    );
                }
                else // 没有可用词典
                {
                    // 设置一个提示信息到 WebView2
                    SendersThis->SelectedDictionaryHtml(L"<html><body><p>没有找到该词的词典信息。</p></body></html>");
                }
            }
            else // 如果新的 WordItem 为空 (例如，取消选择)
            {
                // 清理 WebView2 或显示提示信息
                SendersThis->SelectedDictionaryHtml(L"<html><body><p>请选择一个单词查看详情。</p></body></html>");
            }

            // 更新可见性
            SendersThis->UpdateDetailVisibility();
        }
    }

    // ItemToDisplay 属性更改时的回调
    // 在 WordDetails::OnItemToDisplayChanged 方法内部，获取到 availableDictionaries 之后：
    void WordDetails::OnSelectedDictionaryHtmlChanged(DependencyObject const& d, DependencyPropertyChangedEventArgs const& e)
    {
        if (auto SendersThis{ d.try_as<WordDetails>() })
        {
            winrt::hstring actualHtmlString = winrt::unbox_value<winrt::hstring>(e.NewValue());
            // OutputDebugString((L"OnSelectedDictionaryHtmlChanged: Received HTML. CoreWebView2 initialized: " + (SendersThis->m_isCoreWebView2Initialized ? L"true" : L"false") + L"\n").c_str());
            // OutputDebugString((L"HTML to load: " + actualHtmlString + L"\n").c_str());


            if (SendersThis->DictionaryWebView()) // 确保 DictionaryWebView 控件存在
            {
                if (SendersThis->m_isCoreWebView2Initialized)
                {
                    // OutputDebugString(L"OnSelectedDictionaryHtmlChanged: CoreWebView2 is ready, navigating.\n");
                    SendersThis->DictionaryWebView().NavigateToString(actualHtmlString);
                }
                else
                {
                    // OutputDebugString(L"OnSelectedDictionaryHtmlChanged: CoreWebView2 not ready, storing HTML as pending.\n");
                    SendersThis->m_pendingHtmlToNavigate = actualHtmlString;
                    // InitializeWebView2Async 应该已经由 Loaded 事件触发了，
                    // 如果担心 Loaded 可能晚于第一次 SelectedDictionaryHtml 变化（不太可能但极端情况），
                    // 可以考虑再次调用，但要注意避免多次不必要的 EnsureCoreWebView2Async 调用。
                    // 通常，依赖 Loaded 触发一次初始化即可。
                }
            }
        }
    }

    // DictionarySelectorBar_SelectionChanged 可能需要调整以从 SelectorBarItem 获取文本
    void WordDetails::DictionarySelectorBar_SelectionChanged(SelectorBar const& sender, SelectorBarSelectionChangedEventArgs const& /*args*/)
    {
        // args 参数实际更有用，因为它直接包含 SelectedItem
        // auto selectedBarItem = args.SelectedItem().try_as<SelectorBarItem>();
        // 如果用 sender:
        auto selectedBarItem = sender.SelectedItem().try_as<SelectorBarItem>();

        if (selectedBarItem && ItemToDisplay())
        {
            winrt::hstring dictionaryName = selectedBarItem.Text(); // 从 SelectorBarItem 获取文本
            if (!dictionaryName.empty())
            {
                winrt::hstring word = ItemToDisplay().Word();
                winrt::hstring htmlContent = m_wordSearchService.GetDictionaryHtmlContent(word, dictionaryName);
                SelectedDictionaryHtml(htmlContent); // 这个DP仍然用来驱动WebView2
            }
        }
    }

    // 新增：更新详情可见性的方法
    void WordDetails::UpdateDetailVisibility()
    {
        // Assume ItemToDisplay is nullptr or has an empty Word when nothing is selected
        bool hasWord = (ItemToDisplay() != nullptr) && !ItemToDisplay().Word().empty();

        if (hasWord)
        {
			DetalPanel().Visibility(Visibility::Visible);
            PlaceholderPanel().Visibility(Visibility::Collapsed);
        }
        else
        {
            DetalPanel().Visibility(Visibility::Collapsed);
            PlaceholderPanel().Visibility(Visibility::Visible);
        }
    }
}