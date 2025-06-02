// WordSearch.cpp
#include "pch.h"
#include "WordSearch.h"
#include "WordSearch.g.cpp"
#include "WordItem.h"
#include "DictionaryItemInWordDetail.h"
#include "FilePathProvider.h" // For FilePathProvider
#include "Poco/Data/Statement.h"
#include "Poco/Data/RecordSet.h"
#include "Poco/Exception.h"
#include "Poco/String.h" // for Poco::toLower
#include "Poco/Path.h"   // For Poco::Path
#include "Poco/Nullable.h" // For Poco::Nullable

using namespace winrt;
using namespace Windows::Foundation::Collections;
using namespace ::WordWiz::Data; // For FilePathProvider

namespace winrt::WordWiz::implementation
{
    // Static member definitions
    std::unique_ptr<Poco::Data::Session> WordSearch::_pSession;
    std::string WordSearch::_dbPath = FilePathProvider::GetAppLocalFolderPath() + "\\Dictionaries\\output.db";
    Poco::FastMutex WordSearch::_mutex;
    bool WordSearch::_isInitialized = false;

    WordSearch::WordSearch()
    {
        initialize();
    }

    WordSearch::~WordSearch()
    {
        // Destructor doesn't need to explicitly call shutdown if unique_ptr manages session
        // However, if explicit cleanup is desired for other resources or to unregister connectors:
        // shutdown(); // Consider if this is needed or if RAII is sufficient
    }
    
    void WordSearch::initialize() {
        if (!_isInitialized) {
            Poco::FastMutex::ScopedLock lock(_mutex);
            if (!_isInitialized) { // Double-checked locking
                try {
                    Poco::Data::SQLite::Connector::registerConnector();
                    _pSession = std::make_unique<Poco::Data::Session>("SQLite", _dbPath);
                    // No table creation needed here as we are reading existing tables
                    _isInitialized = true;
                }
                catch (const Poco::Exception& e) {
                    // Log error: Failed to initialize WordSearch database session
                    // For now, rethrow or handle appropriately
                    throw;
                }
            }
        }
    }

    void WordSearch::shutdown() {
        Poco::FastMutex::ScopedLock lock(_mutex);
        if (_isInitialized && _pSession && _pSession->isConnected()) {
            try {
                _pSession->close();
            }
            catch (const Poco::Exception& e) {
                // Log error: Error shutting down SQLite session
            }
        }
        _pSession.reset(); // Clear the unique_ptr
        _isInitialized = false;
        // Poco::Data::SQLite::Connector::unregisterConnector(); // Consider if this should be here or managed globally
    }

    bool WordSearch::isInitialized() {
        Poco::FastMutex::ScopedLock lock(_mutex);
        return _isInitialized && _pSession && _pSession->isConnected();
    }

    std::string WordSearch::getDictionaryTitle(const std::string& uuid) {
        Poco::FastMutex::ScopedLock lock(_mutex);
        if (!isInitialized()) {
            // Log warning or throw: WordSearch is not initialized.
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
            // Enhanced error logging
            WordWizServices::Log::LogMessage(L"getDictionaryTitle error: " + 
                                          winrt::to_hstring(e.displayText()));
            return "";
        }
    }
    
    std::pair<std::string, std::string> WordSearch::getCssReplacement(const std::string& uuid) {
        Poco::FastMutex::ScopedLock lock(_mutex);
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
            // Enhanced error logging
            WordWizServices::Log::LogMessage(L"getCssReplacement error: " + 
                                          winrt::to_hstring(e.displayText()));
            return {"", ""};
        }
    }

    Windows::Foundation::Collections::IVector<WordWiz::WordItem> WordSearch::Search(winrt::hstring const& query)
    {
        Poco::FastMutex::ScopedLock lock(_mutex);
        auto results = winrt::single_threaded_observable_vector<WordWiz::WordItem>();
        if (query.empty() || !isInitialized())
        {
            return results;
        }

        std::string sQuery = winrt::to_string(query);
        try {
            // Prepare the SQL statement
            std::string searchQuery = "%" + sQuery + "%";
            
            // Use RecordSet instead of direct execution with into/bind
            Poco::Data::Statement select(*_pSession);
            select << "SELECT keyword, definition_html FROM word WHERE keyword LIKE ?",
                Poco::Data::Keywords::bind(searchQuery);
                
            // Execute the statement and create a RecordSet
            select.execute();
            Poco::Data::RecordSet rs(select);
            
            // Verify if we have results
            if (rs.rowCount() == 0) {
                WordWizServices::Log::LogMessage(L"Search for '" + query + L"': No records found in database.");
                return results;
            }
            
            // Navigate through results using RecordSet
            bool more = rs.moveFirst();
            while (more) {
                if (!rs["keyword"].isEmpty()) {
                    std::string keyword = rs["keyword"].convert<std::string>();
                    
                    // Create and add a WordItem
                    auto item = winrt::make<WordWiz::implementation::WordItem>(
                        winrt::to_hstring(keyword), 
                        winrt::to_hstring("Definition for " + keyword)
                    );
                    results.Append(item);
                }
                more = rs.moveNext();
            }
        }
        catch (const Poco::Exception& e) {
            // Detailed error logging
            WordWizServices::Log::LogMessage(L"Search error: " + winrt::to_hstring(e.displayText()) + 
                                            L" (Class: " + winrt::to_hstring(e.className()) + 
                                            L", Code: " + winrt::to_hstring(std::to_string(e.code())) + L")");
        }
        catch (const std::exception& e) {
            // Catch any standard exceptions that might occur
            WordWizServices::Log::LogMessage(L"Standard exception in Search: " + winrt::to_hstring(e.what()));
        }
        catch (...) {
            // Catch any other exceptions
            WordWizServices::Log::LogMessage(L"Unknown exception in Search");
        }

        // Log search results
        winrt::hstring logMessage = L"Search for '" + query + L"': " + winrt::to_hstring(results.Size()) + L" results found.";
        if (results.Size() > 0)
        {
            logMessage = logMessage + L" Keywords: [";
            unsigned int count = 0;
            for (auto const& item : results)
            {
                if (count > 0)
                {
                    logMessage = logMessage + L", ";
                }
                logMessage = logMessage + item.Word();
                count++;
                if (count >= 3 && results.Size() > 3) 
                {
                    logMessage = logMessage + L", ...";
                    break;
                }
            }
            logMessage = logMessage + L"]";
        }
        WordWizServices::Log::LogMessage(logMessage);

        return results;
    }

     Windows::Foundation::Collections::IVector<WordWiz::DictionaryItemInWordDetail> WordSearch::GetAvailableDictionaries(winrt::hstring const& word)
    {
        Poco::FastMutex::ScopedLock lock(_mutex);

        auto dictionaries = winrt::single_threaded_vector<WordWiz::DictionaryItemInWordDetail>();
        if (!isInitialized()) {
            return dictionaries;
        }

        try {
            Poco::Data::Statement select(*_pSession);
            Poco::Nullable<std::string> uuid_str_nullable;
            // title_str will be fetched by getDictionaryTitle which already handles nullability

            select << "SELECT DISTINCT uuid FROM info WHERE AttributeName = 'Title'";
            
            Poco::Data::RecordSet rs(select);
            bool more = rs.moveFirst();
            while(more) {
                if (!rs["uuid"].isEmpty()) // Check if the variant is empty before converting
                {
                    uuid_str_nullable = rs["uuid"].convert<std::string>();
                    if (!uuid_str_nullable.isNull()) {
                        std::string uuid_str = uuid_str_nullable.value();
                        std::string title_str = getDictionaryTitle(uuid_str); 
                        if (!title_str.empty()){
                            dictionaries.Append(WordWiz::DictionaryItemInWordDetail{ 
                                winrt::to_hstring(uuid_str),      
                                winrt::to_hstring(uuid_str),      
                                winrt::to_hstring(title_str)      
                            });
                        }
                    }
                }
                more = rs.moveNext();
            }
        }
        catch (const Poco::Exception& e) {
            // Log error
            WordWizServices::Log::LogMessage(L"GetAvailableDictionaries error: " + winrt::to_hstring(e.displayText()));
        }
        return dictionaries;
    }


     winrt::hstring WordSearch::GetDictionaryHtmlContent(winrt::hstring const& word, winrt::hstring const& dictionaryId)
    {
        Poco::FastMutex::ScopedLock lock(_mutex);
        if (!isInitialized() || word.empty() || dictionaryId.empty()) {
            return L"Error: WordSearch not initialized or invalid parameters.";
        }

        std::string sWord = winrt::to_string(word);
        std::string sDictionaryId = winrt::to_string(dictionaryId);
        
        try {
            // Use the same RecordSet approach as in Search method
            Poco::Data::Statement select(*_pSession);
            select << "SELECT definition_html FROM word WHERE keyword = ?",
                Poco::Data::Keywords::bind(sWord);
                
            select.execute();
            Poco::Data::RecordSet rs(select);
            
            if (rs.rowCount() == 0) {
                return L"<p>Definition not found for word: " + word + L"</p>";
            }
            
            // Get the first row (should only be one for an exact keyword match)
            rs.moveFirst();
            
            if (rs["definition_html"].isEmpty()) {
                return L"<p>No definition content available for word: " + word + L"</p>";
            }
            
            std::string definition_html = rs["definition_html"].convert<std::string>();

            // Get CSS replacement info
            auto css_parts = getCssReplacement(sDictionaryId);
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

            return winrt::to_hstring(final_html_content);
        }
        catch (const Poco::Exception& e) {
            // Enhanced error logging
            WordWizServices::Log::LogMessage(L"GetDictionaryHtmlContent error: " + 
                                           winrt::to_hstring(e.displayText()) + 
                                           L" (Class: " + winrt::to_hstring(e.className()) + 
                                           L", Code: " + winrt::to_hstring(std::to_string(e.code())) + L")");
            return L"<p>Error retrieving definition: " + winrt::to_hstring(e.displayText()) + L"</p>";
        }
        catch (const std::exception& e) {
            WordWizServices::Log::LogMessage(L"Standard exception in GetDictionaryHtmlContent: " + winrt::to_hstring(e.what()));
            return L"<p>Error retrieving definition: " + winrt::to_hstring(e.what()) + L"</p>";
        }
        catch (...) {
            WordWizServices::Log::LogMessage(L"Unknown exception in GetDictionaryHtmlContent");
            return L"<p>Unknown error retrieving definition</p>";
        }
    }

}