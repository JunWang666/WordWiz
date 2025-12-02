// WordDetails.xaml.cpp
#include "pch.h"
#include "WordDetails.xaml.h"
#include "WordDetails.g.cpp"
#include "WordItem.h"
#include "DictionaryItemInWordDetail.h"
#include "NavigationService.h"
#include "WordFavorite.h"

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls; // For SelectorBar related types
using namespace Windows::Foundation::Collections;
using namespace WordWiz; // 用于 DictionaryItemInWordDetal
using namespace WordWizServices; // 新增：使用NavigationService命名空间


namespace winrt::WordWiz::implementation
{
	// ItemToDisplayProperty (已存在)
	Microsoft::UI::Xaml::DependencyProperty WordDetails::m_itemToDisplayProperty =
		Microsoft::UI::Xaml::DependencyProperty::Register(
			L"ItemToDisplay",
			xaml_typename<WordWiz::WordItem>(),
			xaml_typename<WordWiz::WordDetails>(),
			Microsoft::UI::Xaml::PropertyMetadata{
				nullptr, PropertyChangedCallback(&WordDetails::OnItemToDisplayChanged)
			} // 添加回调
		);

	// 新增 DictionaryItemsProperty
	Microsoft::UI::Xaml::DependencyProperty WordDetails::m_dictionaryItemsProperty =
		Microsoft::UI::Xaml::DependencyProperty::Register(
			L"DictionaryItems",
			xaml_typename<IObservableVector<WordWiz::DictionaryItemInWordDetail>>(),
			xaml_typename<WordWiz::WordDetails>(),
			Microsoft::UI::Xaml::PropertyMetadata{nullptr}
		);

	// 新增 SelectedDictionaryHtmlProperty
	Microsoft::UI::Xaml::DependencyProperty WordDetails::m_selectedDictionaryHtmlProperty =
		Microsoft::UI::Xaml::DependencyProperty::Register(
			L"SelectedDictionaryHtml",
			xaml_typename<winrt::hstring>(),
			xaml_typename<WordWiz::WordDetails>(),
			Microsoft::UI::Xaml::PropertyMetadata{
				winrt::box_value(L""), PropertyChangedCallback(&WordDetails::OnSelectedDictionaryHtmlChanged)
			} // 初始值为空字符串，并添加回调
		);

	WordDetails::WordDetails()
	{
		InitializeComponent();
		// 初始化 DictionaryItems
		SetValue(m_dictionaryItemsProperty,
		         winrt::single_threaded_observable_vector<WordWiz::DictionaryItemInWordDetail>());
	}

	void WordDetails::OnLoaded(IInspectable const& /*sender*/, RoutedEventArgs const& /*args*/)
	{
		InitializeWebView2Async(); // 控件加载后开始初始化 WebView2
		UpdateDetailVisibility(); // 更新可见性
	}

	// 新增：异步初始化 WebView2 的方法
	winrt::fire_and_forget WordDetails::InitializeWebView2Async()
	{
		auto strong_this{get_strong()}; // 在协程中安全使用 this
		try
		{
			if (strong_this->DictionaryWebView()) // 确保 DictionaryWebView 控件存在
			{
				co_await strong_this->DictionaryWebView().EnsureCoreWebView2Async();

				strong_this->m_isCoreWebView2Initialized = true;

				// 如果有待处理的HTML，现在加载它
				if (strong_this->m_isCoreWebView2Initialized && !strong_this->m_pendingHtmlToNavigate.empty())
				{
					strong_this->DictionaryWebView().NavigateToString(strong_this->m_pendingHtmlToNavigate);
					strong_this->m_pendingHtmlToNavigate = L""; // 清除待处理的HTML
				}
			}
		}
		catch (winrt::hresult_error const& ex)
		{
			winrt::hstring errorMessage = ex.message();
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

	// DictionaryItems Getter
	IObservableVector<WordWiz::DictionaryItemInWordDetail> WordDetails::DictionaryItems()
	{
		return GetValue(m_dictionaryItemsProperty).try_as<IObservableVector<WordWiz::DictionaryItemInWordDetail>>();
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

	void WordDetails::OnItemToDisplayChanged(Microsoft::UI::Xaml::DependencyObject const& d,
	                                         Microsoft::UI::Xaml::DependencyPropertyChangedEventArgs const& e)
	{
		if (auto SendersThis{d.try_as<WordDetails>()}) // 将 d 转换为 WordDetails 实例指针
		{
			auto newItem = e.NewValue().try_as<WordWiz::WordItem>(); // 获取新的 WordItem

			// 新增：通过NavigationService记录历史
			if (SendersThis->m_hostFrame && newItem && !newItem.Word().empty())
			{
				NavigationService::AddCurrentPageToHistoryWithData(
					SendersThis->m_hostFrame,
					newItem,
					nullptr // 可根据需要传递NavigationTransitionInfo
				);
			}

			// 清理 SelectorBar 中的旧项目
			if (SendersThis->DictionarySelectorBar()) // 检查控件是否有效
			{
				SendersThis->DictionarySelectorBar().Items().Clear();
			}

			// 清理可能存在的旧的字典项列表
			if (SendersThis->DictionaryItems())
				SendersThis->DictionaryItems().Clear();

			if (newItem) // 如果新的 WordItem 有效
			{
				winrt::hstring word = newItem.Word(); // 获取单词

				// 调用 WordSearch 服务获取可用词典列表 (现在返回DictionaryItemInWordDetal对象)
				Windows::Foundation::Collections::IVector<WordWiz::DictionaryItemInWordDetail> availableDictionaries =
					WordWiz::WordSearch().GetAvailableDictionaries(word);

				// 存储所有字典项
				for (auto const& dictItem : availableDictionaries)
				{
					SendersThis->DictionaryItems().Append(dictItem);
				}

				if (SendersThis->DictionarySelectorBar()) // 再次检查
				{
					for (auto const& dictItem : availableDictionaries)
					{
						SelectorBarItem sbItem; // 创建新的 SelectorBarItem
						sbItem.Text(dictItem.DisplayName()); // 设置其显示名称
						sbItem.Tag(box_value(dictItem.Id())); // 使用Tag存储字典ID（hstring）
						SendersThis->DictionarySelectorBar().Items().Append(sbItem); // 添加到 SelectorBar
					}
				}

				if (SendersThis->DictionarySelectorBar() && SendersThis->DictionarySelectorBar().Items().Size() > 0)
				{
					// 自动选中第一个词典项
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
	void WordDetails::OnSelectedDictionaryHtmlChanged(DependencyObject const& d,
	                                                  DependencyPropertyChangedEventArgs const& e)
	{
		if (auto SendersThis{d.try_as<WordDetails>()})
		{
			winrt::hstring actualHtmlString = winrt::unbox_value<winrt::hstring>(e.NewValue());
			if (SendersThis->DictionaryWebView()) // 确保 DictionaryWebView 控件存在
			{
				if (SendersThis->m_isCoreWebView2Initialized)
				{
					SendersThis->DictionaryWebView().NavigateToString(actualHtmlString);
				}
				else
				{
					SendersThis->m_pendingHtmlToNavigate = actualHtmlString;
				}
			}
		}
	}

	// DictionarySelectorBar_SelectionChanged 更新以使用字符串ID标识字典
	void WordDetails::DictionarySelectorBar_SelectionChanged(SelectorBar const& sender,
	                                                         SelectorBarSelectionChangedEventArgs const& /*args*/)
	{
		auto selectedBarItem = sender.SelectedItem().try_as<SelectorBarItem>();

		if (selectedBarItem && ItemToDisplay())
		{
			// 从Tag中获取字典ID（hstring）
			if (selectedBarItem.Tag())
			{
				auto dictionaryId = unbox_value<winrt::hstring>(selectedBarItem.Tag());
				winrt::hstring word = ItemToDisplay().Word();

				// 使用字符串ID调用获取字典内容
				winrt::hstring htmlContent = WordWiz::WordSearch().GetDictionaryHtmlContent(word, dictionaryId);
				SelectedDictionaryHtml(htmlContent);
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

	void WordDetails::Favorite_Click(IInspectable const& sender, RoutedEventArgs const& e)
	{
		// 确保当前有一个单词被选中
		if (ItemToDisplay())
		{
			winrt::hstring currentWord = ItemToDisplay().Word();
			bool isFavorite = WordWizModules::WordFavorite::isWordFavorite(currentWord);

			if (isFavorite)
			{
				// If already favorite, just toggle off
				WordWizModules::WordFavorite::setWordFavorite(currentWord, false);
				favoriteButton().Content().try_as<FontIcon>().Glyph(GetFavoriteIconGlyph(currentWord));
			}
			else
			{
				// If not favorite, show flyout to set details
				auto details = WordWizModules::WordFavorite::getFavoriteWordDetails(currentWord);
				FavoriteTagTextBox().Text(winrt::hstring(details.tag));
				FavoriteImportanceSlider().Value(static_cast<double>(details.importance > 0 ? details.importance : 3));
				ImportancePreviewText().Text(GetImportanceStars(static_cast<int>(FavoriteImportanceSlider().Value())));
				
				FavoriteDetailsFlyout().ShowAt(favoriteButton());
			}
		}
	}

	void WordDetails::FavoriteImportanceSlider_ValueChanged(IInspectable const&, Microsoft::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs const&)
	{
		int importance = static_cast<int>(FavoriteImportanceSlider().Value());
		ImportancePreviewText().Text(GetImportanceStars(importance));
	}

	void WordDetails::SaveFavoriteDetails_Click(IInspectable const&, RoutedEventArgs const&)
	{
		if (ItemToDisplay())
		{
			winrt::hstring currentWord = ItemToDisplay().Word();
			winrt::hstring tag = FavoriteTagTextBox().Text();
			int importance = static_cast<int>(FavoriteImportanceSlider().Value());

			WordWizModules::WordFavorite::setWordFavoriteWithDetails(currentWord, importance, tag);
			
			// Update the button icon
			favoriteButton().Content().try_as<FontIcon>().Glyph(GetFavoriteIconGlyph(currentWord));
			
			// Close the flyout
			FavoriteDetailsFlyout().Hide();
		}
	}

	winrt::hstring WordDetails::GetImportanceStars(int importance)
	{
		// Pre-defined star strings for efficiency (avoiding loop concatenation)
		// Using Unicode escape sequence \u2605 for BLACK STAR character
		static const wchar_t* starStrings[] = {
			L"",
			L"\u2605",
			L"\u2605\u2605",
			L"\u2605\u2605\u2605",
			L"\u2605\u2605\u2605\u2605",
			L"\u2605\u2605\u2605\u2605\u2605"
		};
		
		if (importance >= 1 && importance <= 5)
		{
			return winrt::hstring(starStrings[importance]);
		}
		return winrt::hstring(starStrings[0]);
	}

	winrt::hstring WordDetails::GetFavoriteIconGlyph(hstring const& item)
	{
		try{
			bool isFavorite = WordWizModules::WordFavorite::isWordFavorite(item);
			//WordWizServices::Log::LogMessage(L"单词" + item + L"的收藏状态为：" + winrt::to_hstring(isFavorite));
			// isFavorite 为 true 时返回实心星星 (E735)
			// isFavorite 为 false 时返回空心星星 (E734)
			return isFavorite ? L"\uE735" : L"\uE734";
		}
		catch(...){// 默认情况下返回空心星星
			return L"\uE734";
		}
	}
}
