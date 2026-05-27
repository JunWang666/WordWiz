#include "pch.h"
#include "FavoritePage.xaml.h"
#if __has_include("FavoritePage.g.cpp")
#include "FavoritePage.g.cpp"
#endif
#include "WordFavorite.h"
#include <algorithm>

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::WordWiz::implementation
{
	int32_t FavoritePage::MyProperty()
	{
		throw hresult_not_implemented();
	}

	void FavoritePage::MyProperty(int32_t /* value */)
	{
		throw hresult_not_implemented();
	}

	void FavoritePage::myButton_Click(IInspectable const&, RoutedEventArgs const&)
	{
		RefreshFavorites();
	}

	void FavoritePage::OnNavigatedTo(winrt::Microsoft::UI::Xaml::Navigation::NavigationEventArgs const&)
	{
		RefreshFavorites();
	}

	void FavoritePage::RefreshButton_Click(IInspectable const&, RoutedEventArgs const&)
	{
		RefreshFavorites();
	}

	void FavoritePage::SortComboBox_SelectionChanged(winrt::Windows::Foundation::IInspectable const&,
	                                                 winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const&)
	{
		RefreshFavorites();
	}

	void FavoritePage::SearchTextBox_TextChanged(winrt::Windows::Foundation::IInspectable const&,
	                                             winrt::Microsoft::UI::Xaml::Controls::TextChangedEventArgs const&)
	{
		RefreshFavorites();
	}

	void FavoritePage::RemoveFavoriteButton_Click(winrt::Windows::Foundation::IInspectable const& sender,
	                                              RoutedEventArgs const&)
	{
		auto btn = sender.try_as<Button>();
		if (btn && btn.Tag())
		{
			auto word = unbox_value<winrt::hstring>(btn.Tag());
			WordWizModules::WordFavorite::setWordFavorite(word, false);
			RefreshFavorites();
		}
	}

	void FavoritePage::RefreshFavorites()
	{
		const bool newestFirst = SortComboBox() ? SortComboBox().SelectedIndex() == 0 : true;
		m_allFavorites = WordWizModules::WordFavorite::getFavoriteWords(newestFirst);

		auto listItems = winrt::single_threaded_observable_vector<winrt::IInspectable>();
		const auto filter = SearchTextBox() ? SearchTextBox().Text() : L"";

		for (auto const& word : m_allFavorites)
		{
			if (!filter.empty() && word.find(filter) == winrt::hstring::npos)
			{
				continue;
			}
			listItems.Append(winrt::box_value(word));
		}

		FavoriteListView().ItemsSource(listItems);
		StatusTextBlock().Text(L"共 " + winrt::to_hstring(listItems.Size()) + L" 个收藏词条");
	}
}
