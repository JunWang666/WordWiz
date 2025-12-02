#pragma once
#include <vector>
#include <string>

namespace WordWizModules::WordFavorite {
	// Structure to hold favorite word data
	struct FavoriteWordData {
		std::wstring word;
		int importance;      // 1-5 scale
		std::wstring tag;
		std::wstring time;
	};

	bool isWordFavorite(winrt::hstring const& word);
	void setWordFavorite(winrt::hstring const& word, bool target);
	void setWordFavoriteWithDetails(winrt::hstring const& word, int importance, winrt::hstring const& tag);
	void switchWordFavorite(winrt::hstring const& word);
	
	// Get favorite word details
	FavoriteWordData getFavoriteWordDetails(winrt::hstring const& word);
	
	// Update importance and tag for a favorite word
	void updateFavoriteWordDetails(winrt::hstring const& word, int importance, winrt::hstring const& tag);
	
	// Get all favorite words
	std::vector<FavoriteWordData> getAllFavoriteWords();
	
	// Search and filter favorites
	std::vector<FavoriteWordData> searchFavoriteWords(winrt::hstring const& searchQuery, 
		winrt::hstring const& tagFilter, int minImportance);
}