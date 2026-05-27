#pragma once
namespace WordWizModules::WordFavorite {
	bool isWordFavorite(winrt::hstring const& word);
	void setWordFavorite(winrt::hstring const& word, bool target);
	void switchWordFavorite(winrt::hstring const& word);
	std::vector<winrt::hstring> getFavoriteWords(bool newestFirst = true);
}
