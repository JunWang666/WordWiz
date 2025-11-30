#include "pch.h"
#include "FavoritePage.xaml.h"
#if __has_include("FavoritePage.g.cpp")
#include "FavoritePage.g.cpp"
#endif
#if __has_include("FavoriteWordItem.g.cpp")
#include "FavoriteWordItem.g.cpp"
#endif
#include "WordFavorite.h"
#include "NavigationService.h"
#include "MainWindow.xaml.h"
#include "WordSearchResultPage.xaml.h"
#include <set>

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Windows::Foundation::Collections;

namespace winrt::WordWiz::implementation
{
	FavoritePage::FavoritePage()
	{
		InitializeComponent();
		m_favoriteItems = winrt::single_threaded_observable_vector<WordWiz::FavoriteWordItem>();
	}

	IObservableVector<WordWiz::FavoriteWordItem> FavoritePage::FavoriteItems()
	{
		return m_favoriteItems;
	}

	void FavoritePage::Page_Loaded(IInspectable const&, RoutedEventArgs const&)
	{
		LoadFavorites();
		UpdateTagFilter();

		// Connect slider value changed to update preview
		EditImportanceSlider().ValueChanged([this](auto&&, auto&&) {
			int importance = static_cast<int>(EditImportanceSlider().Value());
			ImportancePreviewText().Text(GetImportanceStars(importance));
		});
	}

	void FavoritePage::SearchTextBox_TextChanged(IInspectable const&, TextChangedEventArgs const&)
	{
		RefreshFavorites();
	}

	void FavoritePage::TagFilterComboBox_SelectionChanged(IInspectable const&, SelectionChangedEventArgs const&)
	{
		RefreshFavorites();
	}

	void FavoritePage::ImportanceFilterComboBox_SelectionChanged(IInspectable const&, SelectionChangedEventArgs const&)
	{
		RefreshFavorites();
	}

	void FavoritePage::FavoritesListView_SelectionChanged(IInspectable const&, SelectionChangedEventArgs const&)
	{
		auto selectedItem = FavoritesListView().SelectedItem();
		if (selectedItem)
		{
			auto favoriteItem = selectedItem.try_as<WordWiz::FavoriteWordItem>();
			if (favoriteItem)
			{
				// Navigate to word details
				auto mainFrame = winrt::WordWiz::implementation::MainWindow::GetMainFrame();
				if (mainFrame)
				{
					auto map = winrt::single_threaded_map<winrt::hstring, winrt::Windows::Foundation::IInspectable>();
					map.Insert(L"search_word", winrt::box_value(favoriteItem.Word()));

					WordWizServices::NavigationService::NavigateTo<WordWiz::WordSearchResultPage>(
						mainFrame,
						winrt::box_value(map)
					);
				}
			}
		}
	}

	void FavoritePage::EditButton_Click(IInspectable const& sender, RoutedEventArgs const&)
	{
		auto button = sender.try_as<Button>();
		if (button && button.Tag())
		{
			m_currentEditWord = unbox_value<hstring>(button.Tag());

			// Load current details
			auto details = WordWizModules::WordFavorite::getFavoriteWordDetails(m_currentEditWord);

			EditDialogWordText().Text(m_currentEditWord);
			EditTagTextBox().Text(winrt::hstring(details.tag));
			EditImportanceSlider().Value(static_cast<double>(details.importance));
			ImportancePreviewText().Text(GetImportanceStars(details.importance));

			EditFavoriteDialog().XamlRoot(this->XamlRoot());
			EditFavoriteDialog().ShowAsync();
		}
	}

	void FavoritePage::EditFavoriteDialog_PrimaryButtonClick(ContentDialog const&, ContentDialogButtonClickEventArgs const&)
	{
		if (!m_currentEditWord.empty())
		{
			hstring newTag = EditTagTextBox().Text();
			int newImportance = static_cast<int>(EditImportanceSlider().Value());

			WordWizModules::WordFavorite::updateFavoriteWordDetails(m_currentEditWord, newImportance, newTag);

			// Refresh list
			LoadFavorites();
			UpdateTagFilter();
		}
	}

	void FavoritePage::DeleteFavoriteButton_Click(IInspectable const&, RoutedEventArgs const&)
	{
		if (!m_currentEditWord.empty())
		{
			WordWizModules::WordFavorite::setWordFavorite(m_currentEditWord, false);
			EditFavoriteDialog().Hide();

			// Refresh list
			LoadFavorites();
			UpdateTagFilter();
		}
	}

	Microsoft::UI::Xaml::Visibility FavoritePage::GetTagVisibility(hstring const& tag)
	{
		return tag.empty() ? Visibility::Collapsed : Visibility::Visible;
	}

	hstring FavoritePage::GetImportanceStars(int32_t importance)
	{
		// Pre-defined star strings for efficiency (avoiding loop concatenation)
		static const wchar_t* starStrings[] = {
			L"",
			L"★",
			L"★★",
			L"★★★",
			L"★★★★",
			L"★★★★★"
		};
		
		if (importance >= 1 && importance <= 5)
		{
			return hstring(starStrings[importance]);
		}
		return hstring(starStrings[0]);
	}

	void FavoritePage::LoadFavorites()
	{
		m_favoriteItems.Clear();

		auto favorites = WordWizModules::WordFavorite::getAllFavoriteWords();
		for (const auto& fav : favorites)
		{
			auto item = winrt::make<FavoriteWordItem>(
				winrt::hstring(fav.word),
				fav.importance,
				winrt::hstring(fav.tag),
				winrt::hstring(fav.time)
			);
			m_favoriteItems.Append(item);
		}

		UpdateEmptyState();
		ResultCountTextBlock().Text(L"共 " + winrt::to_hstring(m_favoriteItems.Size()) + L" 个收藏单词");
	}

	void FavoritePage::RefreshFavorites()
	{
		m_favoriteItems.Clear();

		// Get filter values
		hstring searchQuery = SearchTextBox().Text();

		hstring tagFilter;
		if (auto selectedItem = TagFilterComboBox().SelectedItem())
		{
			if (auto comboItem = selectedItem.try_as<ComboBoxItem>())
			{
				if (comboItem.Tag())
				{
					tagFilter = unbox_value<hstring>(comboItem.Tag());
				}
			}
		}

		int minImportance = 0;
		if (auto selectedItem = ImportanceFilterComboBox().SelectedItem())
		{
			if (auto comboItem = selectedItem.try_as<ComboBoxItem>())
			{
				if (comboItem.Tag())
				{
					auto tagStr = unbox_value<hstring>(comboItem.Tag());
					minImportance = std::stoi(winrt::to_string(tagStr));
				}
			}
		}

		auto favorites = WordWizModules::WordFavorite::searchFavoriteWords(searchQuery, tagFilter, minImportance);
		for (const auto& fav : favorites)
		{
			auto item = winrt::make<FavoriteWordItem>(
				winrt::hstring(fav.word),
				fav.importance,
				winrt::hstring(fav.tag),
				winrt::hstring(fav.time)
			);
			m_favoriteItems.Append(item);
		}

		UpdateEmptyState();
		ResultCountTextBlock().Text(L"共 " + winrt::to_hstring(m_favoriteItems.Size()) + L" 个收藏单词");
	}

	void FavoritePage::UpdateTagFilter()
	{
		// Get all unique tags
		std::set<std::wstring> tags;
		auto favorites = WordWizModules::WordFavorite::getAllFavoriteWords();
		for (const auto& fav : favorites)
		{
			if (!fav.tag.empty())
			{
				tags.insert(fav.tag);
			}
		}

		// Remember current selection
		hstring currentSelection;
		if (auto selectedItem = TagFilterComboBox().SelectedItem())
		{
			if (auto comboItem = selectedItem.try_as<ComboBoxItem>())
			{
				if (comboItem.Tag())
				{
					currentSelection = unbox_value<hstring>(comboItem.Tag());
				}
			}
		}

		// Clear and rebuild
		TagFilterComboBox().Items().Clear();

		ComboBoxItem allItem;
		allItem.Content(box_value(L"全部标签"));
		allItem.Tag(box_value(L""));
		TagFilterComboBox().Items().Append(allItem);

		for (const auto& tag : tags)
		{
			ComboBoxItem item;
			item.Content(box_value(winrt::hstring(tag)));
			item.Tag(box_value(winrt::hstring(tag)));
			TagFilterComboBox().Items().Append(item);
		}

		// Restore selection
		TagFilterComboBox().SelectedIndex(0);
		for (uint32_t i = 0; i < TagFilterComboBox().Items().Size(); i++)
		{
			if (auto item = TagFilterComboBox().Items().GetAt(i).try_as<ComboBoxItem>())
			{
				if (item.Tag() && unbox_value<hstring>(item.Tag()) == currentSelection)
				{
					TagFilterComboBox().SelectedIndex(i);
					break;
				}
			}
		}
	}

	void FavoritePage::UpdateEmptyState()
	{
		if (m_favoriteItems.Size() == 0)
		{
			EmptyStatePanel().Visibility(Visibility::Visible);
			FavoritesListView().Visibility(Visibility::Collapsed);
		}
		else
		{
			EmptyStatePanel().Visibility(Visibility::Collapsed);
			FavoritesListView().Visibility(Visibility::Visible);
		}
	}
}
