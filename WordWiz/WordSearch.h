// WordSearch.h
#pragma once
#include "WordSearch.g.h"
#include "WordItem.h"
#include "DictionaryItemInWordDetail.g.h" // ֻ include g.h

#include "Poco/Data/Session.h"
#include "Poco/Data/SQLite/Connector.h"
#include "Poco/Mutex.h"
#include <memory> // For std::unique_ptr
#include <string> // For std::string

using namespace winrt;
using namespace Windows::Foundation::Collections;

namespace winrt::WordWiz::implementation
{
    struct WordSearch : WordSearchT<WordSearch>
    {
        WordSearch();
        ~WordSearch();

        IVector<WordWiz::WordItem> Search(winrt::hstring const& query);
        IVector<WordWiz::DictionaryItemInWordDetail> GetAvailableDictionaries(winrt::hstring const& word);
        winrt::hstring GetDictionaryHtmlContent(winrt::hstring const& word, winrt::hstring const& dictionaryId);
        
        static void shutdown();

    private:
        void initialize();
        bool isInitialized();
        
        // Database related members
        static std::unique_ptr<Poco::Data::Session> _pSession;
        static std::string _dbPath;
        static Poco::FastMutex _mutex;
        static bool _isInitialized;

        // Helper methods for database interaction
        std::string getDictionaryTitle(const std::string& uuid);
        std::pair<std::string, std::string> getCssReplacement(const std::string& uuid);
    };
}

namespace winrt::WordWiz::factory_implementation
{
    struct WordSearch : WordSearchT<WordSearch, implementation::WordSearch>
    {
    };
}