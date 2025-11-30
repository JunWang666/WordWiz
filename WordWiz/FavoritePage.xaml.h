#pragma once

#include "FavoritePage.g.h"
#include "FavoriteWordItem.g.h"
#include "WordFavorite.h"

namespace winrt::WordWiz::implementation
{
	struct FavoriteWordItem : FavoriteWordItemT<FavoriteWordItem>
	{
		FavoriteWordItem(hstring const& word, int32_t importance, hstring const& tag, hstring const& time)
			: m_word(word), m_importance(importance), m_tag(tag), m_time(time) {}

		hstring Word() { return m_word; }
		int32_t Importance() { return m_importance; }
		hstring Tag() { return m_tag; }
		hstring Time() { return m_time; }

	private:
		hstring m_word;
		int32_t m_importance;
		hstring m_tag;
		hstring m_time;
	};

	struct FavoritePage : FavoritePageT<FavoritePage>
	{
		FavoritePage();

		Windows::Foundation::Collections::IObservableVector<WordWiz::FavoriteWordItem> FavoriteItems();

		void Page_Loaded(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& e);
		void SearchTextBox_TextChanged(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::Controls::TextChangedEventArgs const& e);
		void TagFilterComboBox_SelectionChanged(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e);
		void ImportanceFilterComboBox_SelectionChanged(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e);
		void FavoritesListView_SelectionChanged(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e);
		void EditButton_Click(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& e);
		void EditFavoriteDialog_PrimaryButtonClick(Microsoft::UI::Xaml::Controls::ContentDialog const& sender, Microsoft::UI::Xaml::Controls::ContentDialogButtonClickEventArgs const& args);
		void DeleteFavoriteButton_Click(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& e);

		// Static helper functions for XAML binding
		static Microsoft::UI::Xaml::Visibility GetTagVisibility(hstring const& tag);
		static hstring GetImportanceStars(int32_t importance);

	private:
		Windows::Foundation::Collections::IObservableVector<WordWiz::FavoriteWordItem> m_favoriteItems;
		hstring m_currentEditWord;

		void LoadFavorites();
		void RefreshFavorites();
		void UpdateTagFilter();
		void UpdateEmptyState();
	};
}

namespace winrt::WordWiz::factory_implementation
{
	struct FavoriteWordItem : FavoriteWordItemT<FavoriteWordItem, implementation::FavoriteWordItem>
	{
	};

	struct FavoritePage : FavoritePageT<FavoritePage, implementation::FavoritePage>
	{
	};
}
