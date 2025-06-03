#pragma once
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include "Poco/Data/Session.h"
#include "Poco/Data/SQLite/Connector.h"
#include "Poco/Mutex.h"

namespace WordWizServices::Database
{
    struct WordSearchResult
    {
        std::string keyword;
        std::string definition_html;
    };

    struct DictionaryInfo
    {
        std::string uuid;
        std::string title;
    };

    class DatabaseManager
    {
    public:
        static DatabaseManager& getInstance();
        
        // Database lifecycle
        void initialize();
        void shutdown();
        bool isInitialized();
        
        // Database operations
        std::vector<WordSearchResult> searchWords(const std::string& query);
        std::vector<DictionaryInfo> getAvailableDictionaries();
        std::string getDictionaryHtmlContent(const std::string& word, const std::string& dictionaryId);
        std::string getDictionaryTitle(const std::string& uuid);
        std::pair<std::string, std::string> getCssReplacement(const std::string& uuid);

    private:
        DatabaseManager() = default;
        ~DatabaseManager() = default;
        DatabaseManager(const DatabaseManager&) = delete;
        DatabaseManager& operator=(const DatabaseManager&) = delete;

        std::unique_ptr<Poco::Data::Session> _pSession;
        std::string _dbPath;
        Poco::FastMutex _mutex;
        bool _isInitialized = false;
    };
}
