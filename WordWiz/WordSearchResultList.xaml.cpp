// WordSearchResultList.xaml.cpp
#include "pch.h"
#include "WordSearchResultList.xaml.h"
#include "WordSearchResultList.g.cpp" // 由 MIDL 生成
#include "WordItem.h"                 // 创建 WordItem 实例需要
#include <winrt/Microsoft.UI.Dispatching.h> // 添加此头文件以支持 Microsoft::UI::Dispatching::DispatcherQueue
#include <vector> // 添加 std::vector 支持
// #include "WordSearch.h" // 已经在 pch.h 或 WordSearchResultList.xaml.h 中包含了

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Windows::Foundation::Collections;
using namespace Windows::System; // For VirtualKey
using namespace Microsoft::UI::Dispatching; // For DispatcherQueue

namespace winrt::WordWiz::implementation
{
	// 静态依赖属性 m_currentDetailItemProperty 的定义和注册
	Microsoft::UI::Xaml::DependencyProperty WordSearchResultList::m_currentDetailItemProperty =
		Microsoft::UI::Xaml::DependencyProperty::Register(
			L"CurrentDetailItem",
			xaml_typename<WordWiz::WordItem>(),
			xaml_typename<WordWiz::WordSearchResultList>(),
			Microsoft::UI::Xaml::PropertyMetadata{nullptr}
		);


	// 构造函数
	WordSearchResultList::WordSearchResultList()
	{
		m_items = winrt::single_threaded_observable_vector<WordWiz::WordItem>();

		// 初始化 WordSearch 服务
		m_wordSearchService = winrt::make<WordWiz::implementation::WordSearch>();

		// 初始化防抖计时器
		m_debounceTimer = Microsoft::UI::Xaml::DispatcherTimer();
		m_debounceTimer.Interval(std::chrono::milliseconds{500}); // 设置0.5秒间隔
		m_debounceTimer.Tick({this, &WordSearchResultList::OnDebounceTimerTick});

		InitializeComponent(); // XAML 生成的代码，必须调用

		// 你原有的示例数据添加代码
		// AddSampleItems(); 
		// 建议在实际使用时，列表初始为空，通过搜索填充
	}

	// Items 属性的 getter 实现
	IObservableVector<WordWiz::WordItem> WordSearchResultList::Items()
	{
		return m_items;
	}

	// CurrentDetailItem 依赖属性的 getter 实现
	WordWiz::WordItem WordSearchResultList::CurrentDetailItem()
	{
		return GetValue(m_currentDetailItemProperty).try_as<WordWiz::WordItem>();
	}

	// CurrentDetailItem 依赖属性的 setter 实现
	void WordSearchResultList::CurrentDetailItem(WordWiz::WordItem const& value)
	{
		SetValue(m_currentDetailItemProperty, value);
	}

	// ListView 的 SelectionChanged 事件处理函数实现
	void WordSearchResultList::ResultsListView_SelectionChanged(IInspectable const& sender,
	                                                            Controls::SelectionChangedEventArgs const& /*args*/)
	{
		auto listView = sender.as<Microsoft::UI::Xaml::Controls::ListView>();
		auto selectedListViewItem = listView.SelectedItem(); // 获取当前选中的原始项

		if (selectedListViewItem) // 关键：仅当确实有一个选中项时才更新
		{
			// 将选中的项转换为 WordItem 并更新 CurrentDetailItem
			this->CurrentDetailItem(selectedListViewItem.try_as<WordWiz::WordItem>());
		}
		// else (如果 selectedListViewItem 是 nullptr，例如列表被清空或用户取消选择)
		// {
		//     我们不执行任何操作，CurrentDetailItem 保持其之前的值。
		//     这样 WordDetails 控件就不会“复原”。        // }
	}

	// SearchTextBox 的 TextChanged 事件处理函数 (防抖逻辑)
	void WordSearchResultList::SearchTextBox_TextChanged(Windows::Foundation::IInspectable const& /*sender*/,
	                                                     Microsoft::UI::Xaml::Controls::TextChangedEventArgs const&
	                                                     /*args*/)
	{
		//m_debounceTimer.Stop(); // 每次输入变化时，重置计时器
		m_debounceTimer.Start();
	} // SearchTextBox 的 KeyDown 事件处理函数 (回车立即搜索)
	void WordSearchResultList::SearchTextBox_KeyDown(Windows::Foundation::IInspectable const& sender,
	                                                 Microsoft::UI::Xaml::Input::KeyRoutedEventArgs const& e)
	{
		if (e.Key() == VirtualKey::Enter)
		{
			m_debounceTimer.Stop(); // 停止可能正在计时的防抖操作
			auto textBox = sender.as<Controls::TextBox>();
			// 使用 fire-and-forget 调用异步方法
			ExecuteSearchAsync(textBox.Text());
		}
	}

	// 防抖计时器触发事件
	void WordSearchResultList::OnDebounceTimerTick(Windows::Foundation::IInspectable const& /*sender*/,
	                                               Windows::Foundation::IInspectable const& /*e*/)
	{
		m_debounceTimer.Stop(); // 计时器触发后先停止
		// 使用 fire-and-forget 调用异步方法
		ExecuteSearchAsync(SearchTextBox().Text());
	} // 执行搜索的异步方法 - 使用值传递确保协程安全
	winrt::Windows::Foundation::IAsyncAction WordSearchResultList::ExecuteSearchAsync(winrt::hstring query)
	{
		// 如果查询为空，显示无输入状态并清空列表
		if (query.empty())
		{
			HideLoadingState();
			HideNoResultsState();
			ShowNoInputState();
			m_items.Clear();
			co_return;
		}

		// 显示加载状态
		ShowLoadingState();
		HideNoResultsState();
		HideNoInputState();

		// 获取强引用，确保对象生命周期
		auto strongThis = get_strong();

		try
		{
			// 在后台线程执行搜索
			co_await winrt::resume_background();

			// 执行搜索（在后台线程）
			auto results = strongThis->m_wordSearchService.Search(query);

			// 简化的UI更新方式 - 使用简单的调度队列调用
			strongThis->DispatcherQueue().TryEnqueue([strongThis, results]()
			{
				try
				{
					// 隐藏加载状态
					strongThis->HideLoadingState();

					// 清空现有列表项
					strongThis->m_items.Clear();

					if (results != nullptr && results.Size() > 0)
					{
						// 有搜索结果，添加到列表
						for (auto const& item : results)
						{
							if (item != nullptr)
							{
								strongThis->m_items.Append(item);
							}
						}
						strongThis->HideNoResultsState();
					}
					else
					{
						// 没有搜索结果，显示无结果状态
						strongThis->ShowNoResultsState();
					}
				}
				catch (...)
				{
					// UI线程中的异常处理
					strongThis->HideLoadingState();
					strongThis->ShowNoResultsState();
				}
			});
		}
		catch (...)
		{
			// 后台线程异常处理
			strongThis->DispatcherQueue().TryEnqueue([strongThis]()
			{
				strongThis->HideLoadingState();
				strongThis->ShowNoResultsState();
			});
		}
	} // 显示加载状态
	void WordSearchResultList::ShowLoadingState()
	{
		if (LoadingProgressBar())
		{
			LoadingProgressBar().Visibility(Microsoft::UI::Xaml::Visibility::Visible);
		}
		HideNoInputState();
		HideNoResultsState();
	}

	// 隐藏加载状态
	void WordSearchResultList::HideLoadingState()
	{
		if (LoadingProgressBar())
		{
			LoadingProgressBar().Visibility(Microsoft::UI::Xaml::Visibility::Collapsed);
		}
	}

	// 显示无结果状态
	void WordSearchResultList::ShowNoResultsState()
	{
		if (NoResultsPanel())
		{
			NoResultsPanel().Visibility(Microsoft::UI::Xaml::Visibility::Visible);
		}
		HideNoInputState();
	}

	// 隐藏无结果状态
	void WordSearchResultList::HideNoResultsState()
	{
		if (NoResultsPanel())
		{
			NoResultsPanel().Visibility(Microsoft::UI::Xaml::Visibility::Collapsed);
		}
	}

	// 显示无输入状态
	void WordSearchResultList::ShowNoInputState()
	{
		if (NoInputPanel())
		{
			NoInputPanel().Visibility(Microsoft::UI::Xaml::Visibility::Visible);
		}
		HideNoResultsState();
	}

	// 隐藏无输入状态
	void WordSearchResultList::HideNoInputState()
	{
		if (NoInputPanel())
		{
			NoInputPanel().Visibility(Microsoft::UI::Xaml::Visibility::Collapsed);
		}
	}


	// 你已有的辅助方法
	void WordSearchResultList::AddSampleItems()
	{
		AddNewEntry(L"WinUI 3 Sample", L"Native UX platform from Microsoft.");
		AddNewEntry(L"C++/WinRT Sample", L"Standard C++ language projection aaaaaaaaaaa.");
	}

	void WordSearchResultList::AddNewEntry(winrt::hstring const& word, winrt::hstring const& explanation)
	{
		auto newItem = winrt::make<WordWiz::implementation::WordItem>(word, explanation);
		m_items.Append(newItem);
	}

	void WordSearchResultList::searchWord(winrt::hstring const& query)
	{
		SearchTextBox().Text(query); // 更新搜索框文本
		// 使用 fire-and-forget 调用异步方法
		ExecuteSearchAsync(query);
	}
}
