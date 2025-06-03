#include <pch.h>
#include "database.h"
#include "FilePathProvider.h"
#include "logger.h"
#include <Poco/Data/Session.h>
#include <Poco/Data/SQLite/Connector.h>
#include "Poco/Data/Statement.h"
#include "Poco/Data/RecordSet.h"
#include "Poco/Exception.h"
#include "Poco/String.h" // for Poco::toLower
#include "Poco/Path.h"   // For Poco::Path
#include "Poco/Nullable.h" // For Poco::Nullable

using namespace winrt;
using namespace ::WordWizServices::Data; // For FilePathProvider

namespace WordWizServices::Database
{
    DatabaseManager& DatabaseManager::getInstance()
    {
        static DatabaseManager instance;
        return instance;
    }

    void DatabaseManager::initialize()
    {
        if (!_isInitialized) {
            Poco::FastMutex::ScopedLock lock(_mutex);
            if (!_isInitialized) { // Double-checked locking
                try {
                    _dbPath = FilePathProvider::GetAppLocalFolderPath() + "\\Dictionaries\\output.db";
                    Poco::Data::SQLite::Connector::registerConnector();
                    _pSession = std::make_unique<Poco::Data::Session>("SQLite", _dbPath);
                    // No table creation needed here as we are reading existing tables
                    _isInitialized = true;
                }
                catch (const Poco::Exception& e) {
                    WordWizServices::Log::LogMessage(L"Failed to initialize database: " + 
                                                   winrt::to_hstring(e.displayText()));
                    throw;
                }
            }
        }
    }

    void DatabaseManager::shutdown()
    {
        Poco::FastMutex::ScopedLock lock(_mutex);
        if (_isInitialized && _pSession && _pSession->isConnected()) {
            try {
                _pSession->close();
            }
            catch (const Poco::Exception& e) {
                WordWizServices::Log::LogMessage(L"Error shutting down database: " + 
                                               winrt::to_hstring(e.displayText()));
            }
        }
        _pSession.reset(); // Clear the unique_ptr
        _isInitialized = false;
    }

    bool DatabaseManager::isInitialized()
    {
        Poco::FastMutex::ScopedLock lock(_mutex);
        return _isInitialized && _pSession && _pSession->isConnected();
    }

    std::vector<WordSearchResult> DatabaseManager::searchWords(const std::string& query)
    {
        Poco::FastMutex::ScopedLock lock(_mutex);
        std::vector<WordSearchResult> results;
        
        if (query.empty() || !isInitialized()) {
            return results;
        }

        try {
            // Prepare the SQL statement
            std::string searchQuery = "%" + query + "%";
            
            // Use RecordSet instead of direct execution with into/bind
            Poco::Data::Statement select(*_pSession);
            select << "SELECT keyword, definition_html FROM word WHERE keyword LIKE ?",
                Poco::Data::Keywords::bind(searchQuery);
                
            // Execute the statement and create a RecordSet
            select.execute();
            Poco::Data::RecordSet rs(select);
            
            // Verify if we have results
            if (rs.rowCount() == 0) {
                WordWizServices::Log::LogMessage(L"Search for '" + winrt::to_hstring(query) + L"': No records found in database.");
                return results;
            }
            
            // Navigate through results using RecordSet
            bool more = rs.moveFirst();
            while (more) {
                if (!rs["keyword"].isEmpty()) {
                    WordSearchResult result;
                    result.keyword = rs["keyword"].convert<std::string>();
                    if (!rs["definition_html"].isEmpty()) {
                        result.definition_html = rs["definition_html"].convert<std::string>();
                    }
                    results.push_back(result);
                }
                more = rs.moveNext();
            }
        }
        catch (const Poco::Exception& e) {
            WordWizServices::Log::LogMessage(L"Search error: " + winrt::to_hstring(e.displayText()) + 
                                            L" (Class: " + winrt::to_hstring(e.className()) + 
                                            L", Code: " + winrt::to_hstring(std::to_string(e.code())) + L")");
        }
        catch (const std::exception& e) {
            WordWizServices::Log::LogMessage(L"Standard exception in searchWords: " + winrt::to_hstring(e.what()));
        }
        catch (...) {
            WordWizServices::Log::LogMessage(L"Unknown exception in searchWords");
        }

        return results;
    }

    std::vector<DictionaryInfo> DatabaseManager::getAvailableDictionaries()
    {
        Poco::FastMutex::ScopedLock lock(_mutex);
        std::vector<DictionaryInfo> dictionaries;
        
        if (!isInitialized()) {
            return dictionaries;
        }

        try {
            Poco::Data::Statement select(*_pSession);
            Poco::Nullable<std::string> uuid_str_nullable;

            select << "SELECT DISTINCT uuid FROM info WHERE AttributeName = 'Title'";
            
            Poco::Data::RecordSet rs(select);
            bool more = rs.moveFirst();
            while(more) {
                if (!rs["uuid"].isEmpty()) {
                    uuid_str_nullable = rs["uuid"].convert<std::string>();
                    if (!uuid_str_nullable.isNull()) {
                        std::string uuid_str = uuid_str_nullable.value();
                        std::string title_str = getDictionaryTitle(uuid_str); 
                        if (!title_str.empty()){
                            DictionaryInfo dict;
                            dict.uuid = uuid_str;
                            dict.title = title_str;
                            dictionaries.push_back(dict);
                        }
                    }
                }
                more = rs.moveNext();
            }
        }
        catch (const Poco::Exception& e) {
            WordWizServices::Log::LogMessage(L"getAvailableDictionaries error: " + winrt::to_hstring(e.displayText()));
        }
        return dictionaries;
    }

    std::string DatabaseManager::getDictionaryHtmlContent(const std::string& word, const std::string& dictionaryId)
    {
        Poco::FastMutex::ScopedLock lock(_mutex);
        if (!isInitialized() || word.empty() || dictionaryId.empty()) {
            return "Error: Database not initialized or invalid parameters.";
        }
        
        try {
            // Use the same RecordSet approach as in Search method
            Poco::Data::Statement select(*_pSession);
            select << "SELECT definition_html FROM word WHERE keyword = ?",
                Poco::Data::Keywords::bind(word);
                
            select.execute();
            Poco::Data::RecordSet rs(select);
            
            if (rs.rowCount() == 0) {
                return "<p>Definition not found for word: " + word + "</p>";
            }
            
            // Get the first row (should only be one for an exact keyword match)
            rs.moveFirst();
            
            if (rs["definition_html"].isEmpty()) {
                return "<p>No definition content available for word: " + word + "</p>";
            }
            
            std::string definition_html = rs["definition_html"].convert<std::string>();

            // Get CSS replacement info
            auto css_parts = getCssReplacement(dictionaryId);
            std::string css_target_href = css_parts.first;
            std::string css_replacement_content = css_parts.second;

            std::string final_html_content = definition_html;
            if (!css_target_href.empty() && !css_replacement_content.empty()) {
                size_t pos = final_html_content.find(css_target_href);
                if (pos != std::string::npos) {
                    final_html_content.replace(pos, css_target_href.length(), css_replacement_content);
                }
            }
            
            std::string html_wrapper_start = "<html><head><meta charset=\'UTF-8\'></head><body>";
            std::string html_wrapper_end = "</body></html>";
            
            if (Poco::toLower(final_html_content).find("<html") == std::string::npos) {
                 final_html_content = html_wrapper_start + final_html_content + html_wrapper_end;
            }

            return final_html_content;
        }
        catch (const Poco::Exception& e) {
            WordWizServices::Log::LogMessage(L"getDictionaryHtmlContent error: " + 
                                           winrt::to_hstring(e.displayText()) + 
                                           L" (Class: " + winrt::to_hstring(e.className()) + 
                                           L", Code: " + winrt::to_hstring(std::to_string(e.code())) + L")");
            return "<p>Error retrieving definition: " + e.displayText() + "</p>";
        }
        catch (const std::exception& e) {
            WordWizServices::Log::LogMessage(L"Standard exception in getDictionaryHtmlContent: " + winrt::to_hstring(e.what()));
            return "<p>Error retrieving definition: " + std::string(e.what()) + "</p>";
        }
        catch (...) {
            WordWizServices::Log::LogMessage(L"Unknown exception in getDictionaryHtmlContent");
            return "<p>Unknown error retrieving definition</p>";
        }
    }

    std::string DatabaseManager::getDictionaryTitle(const std::string& uuid)
    {
        // Note: This method doesn't need additional locking as it's called from already locked methods
        if (!isInitialized()) {
            return ""; 
        }
        try {
            Poco::Data::Statement select(*_pSession);
            select << "SELECT AttributeValue FROM info WHERE AttributeName = 'Title' AND uuid = ?",
                Poco::Data::Keywords::bind(uuid);
                
            select.execute();
            Poco::Data::RecordSet rs(select);
            
            if (rs.rowCount() == 0) {
                return ""; // No title found
            }
            
            rs.moveFirst();
            if (rs["AttributeValue"].isEmpty()) {
                return ""; // AttributeValue is empty or NULL
            }
            
            return rs["AttributeValue"].convert<std::string>();
        }
        catch (const Poco::Exception& e) {
            WordWizServices::Log::LogMessage(L"getDictionaryTitle error: " + 
                                          winrt::to_hstring(e.displayText()));
            return "";
        }
    }

    std::pair<std::string, std::string> DatabaseManager::getCssReplacement(const std::string& uuid)
    {
        // Note: This method doesn't need additional locking as it's called from already locked methods
        if (!isInitialized()) {
            return {"", ""};
        }
        
        std::string targetHref = "", replacementContent = "";
        
        try {
            // Query for CSS_TARGET_HREF
            Poco::Data::Statement selectTarget(*_pSession);
            selectTarget << "SELECT AttributeValue FROM info WHERE AttributeName = 'CSS_TARGET_HREF' AND uuid = ?",
                Poco::Data::Keywords::bind(uuid);
                
            selectTarget.execute();
            Poco::Data::RecordSet rsTarget(selectTarget);
            
            if (rsTarget.rowCount() > 0) {
                rsTarget.moveFirst();
                if (!rsTarget["AttributeValue"].isEmpty()) {
                    targetHref = rsTarget["AttributeValue"].convert<std::string>();
                }
            }

            // Query for CSS_REPLACEMENT_CONTENT
            Poco::Data::Statement selectReplacement(*_pSession);
            selectReplacement << "SELECT AttributeValue FROM info WHERE AttributeName = 'CSS_REPLACEMENT_CONTENT' AND uuid = ?",
                Poco::Data::Keywords::bind(uuid);
                
            selectReplacement.execute();
            Poco::Data::RecordSet rsReplacement(selectReplacement);
            
            if (rsReplacement.rowCount() > 0) {
                rsReplacement.moveFirst();
                if (!rsReplacement["AttributeValue"].isEmpty()) {
                    replacementContent = rsReplacement["AttributeValue"].convert<std::string>();
                }
            }
            
            return {targetHref, replacementContent};
        }
        catch (const Poco::Exception& e) {
            WordWizServices::Log::LogMessage(L"getCssReplacement error: " + 
                                          winrt::to_hstring(e.displayText()));
            return {"", ""};
        }
    }
}

