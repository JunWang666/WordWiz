// WordSearch.cpp
#include "pch.h"
#include "WordSearch.h"
#include "WordSearch.g.cpp"
#include "WordItem.h"
#include "DictionaryItemInWordDetail.h"
#include "database.h"
#include "logger.h"

using namespace winrt;
using namespace Windows::Foundation::Collections;

namespace winrt::WordWiz::implementation
{    WordSearch::WordSearch()
    {
        WordWizServices::Database::DatabaseManager::getInstance().initialize();
    }

    WordSearch::~WordSearch()
    {
        // Destructor doesn't need to explicitly call shutdown
        // DatabaseManager will handle cleanup
    }
    
    void WordSearch::shutdown() {
        WordWizServices::Database::DatabaseManager::getInstance().shutdown();
    }Windows::Foundation::Collections::IVector<WordWiz::WordItem> WordSearch::Search(winrt::hstring const& query)
    {
        auto results = winrt::single_threaded_observable_vector<WordWiz::WordItem>();
        if (query.empty())
        {
            return results;
        }        std::string sQuery = winrt::to_string(query);
        auto& dbManager = WordWizServices::Database::DatabaseManager::getInstance();
        
        try {
            std::string searchQuery = "%" + sQuery + "%";
            auto rs = dbManager.executeQuery("SELECT keyword, definition_html FROM word WHERE keyword LIKE ?", {searchQuery});
            
            if (rs.rowCount() == 0) {
                WordWizServices::Log::LogMessage(L"Search for '" + query + L"': No records found in database.");
                return results;
            }
            
            bool more = rs.moveFirst();
            while (more) {
                if (!rs["keyword"].isEmpty()) {
                    std::string keyword = rs["keyword"].convert<std::string>();
                    
                    auto item = winrt::make<WordWiz::implementation::WordItem>(
                        winrt::to_hstring(keyword), 
                        winrt::to_hstring("Definition for " + keyword)
                    );
                    results.Append(item);
                }
                more = rs.moveNext();
            }
        }
        catch (const std::exception& e) {
            WordWizServices::Log::LogMessage(L"Exception in Search: " + winrt::to_hstring(e.what()));
        }
        catch (...) {
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
        auto dictionaries = winrt::single_threaded_vector<WordWiz::DictionaryItemInWordDetail>();
        auto& dbManager = WordWizServices::Database::DatabaseManager::getInstance();
        
        try {
            // Query to get available dictionaries for a word
            std::string sWord = winrt::to_string(word);
            auto rs = dbManager.executeQuery("SELECT DISTINCT uuid FROM info WHERE keyword = ?", {sWord});
            
            if (rs.rowCount() > 0) {
                bool more = rs.moveFirst();
                while (more) {
                    if (!rs["uuid"].isEmpty()) {
                        std::string uuid = rs["uuid"].convert<std::string>();
                        std::string title = getDictionaryTitle(uuid);
                        
                        dictionaries.Append(WordWiz::DictionaryItemInWordDetail{ 
                            winrt::to_hstring(uuid),      
                            winrt::to_hstring(title),
                            winrt::to_hstring(title)      
                        });
                    }
                    more = rs.moveNext();
                }
            }
        }
        catch (const std::exception& e) {
            WordWizServices::Log::LogMessage(L"Exception in GetAvailableDictionaries: " + winrt::to_hstring(e.what()));
        }
        catch (...) {
            WordWizServices::Log::LogMessage(L"Unknown exception in GetAvailableDictionaries");
        }
        
        return dictionaries;
    }
      winrt::hstring WordSearch::GetDictionaryHtmlContent(winrt::hstring const& word, winrt::hstring const& dictionaryId)
    {
        if (word.empty() || dictionaryId.empty()) {
            return L"Error: Invalid parameters.";
        }

        std::string sWord = winrt::to_string(word);
        std::string sDictionaryId = winrt::to_string(dictionaryId);
        auto& dbManager = WordWizServices::Database::DatabaseManager::getInstance();
        
        try {
            // Get the definition HTML content from the database
            auto rs = dbManager.executeQuery("SELECT definition_html FROM entry WHERE keyword = ? AND uuid = ?", {sWord, sDictionaryId});
            
            if (rs.rowCount() == 0 || !rs.moveFirst()) {
                return L"<p>Definition not found for this dictionary.</p>";
            }
            
            std::string htmlContent = rs["definition_html"].convert<std::string>();
            
            // Apply CSS replacements
            auto cssReplacement = getCssReplacement(sDictionaryId);
            if (!cssReplacement.first.empty() && !cssReplacement.second.empty()) {
                // Replace CSS in HTML content
                size_t pos = htmlContent.find(cssReplacement.first);
                if (pos != std::string::npos) {
                    htmlContent.replace(pos, cssReplacement.first.length(), cssReplacement.second);
                }
            }
            
            return winrt::to_hstring(htmlContent);
        }
        catch (const std::exception& e) {
            WordWizServices::Log::LogMessage(L"Exception in GetDictionaryHtmlContent: " + winrt::to_hstring(e.what()));
            return L"<p>Error retrieving definition: " + winrt::to_hstring(e.what()) + L"</p>";
        }
        catch (...) {
            WordWizServices::Log::LogMessage(L"Unknown exception in GetDictionaryHtmlContent");
            return L"<p>Unknown error retrieving definition</p>";        }
    }

    // Private helper methods
    std::string WordSearch::getDictionaryTitle(const std::string& uuid)
    {
        auto& dbManager = WordWizServices::Database::DatabaseManager::getInstance();
        try {
            std::string title = dbManager.executeScalarQuery("SELECT title FROM dictionary WHERE uuid = ?", {uuid});
            return title.empty() ? "Unknown Dictionary" : title;
        }
        catch (...) {
            return "Unknown Dictionary";
        }
    }

    std::pair<std::string, std::string> WordSearch::getCssReplacement(const std::string& uuid)
    {
        auto& dbManager = WordWizServices::Database::DatabaseManager::getInstance();
        try {
            auto rs = dbManager.executeQuery("SELECT old_css, new_css FROM css_replacement WHERE uuid = ?", {uuid});
            
            if (rs.rowCount() > 0 && rs.moveFirst()) {
                std::string oldCss = rs["old_css"].convert<std::string>();
                std::string newCss = rs["new_css"].convert<std::string>();
                return std::make_pair(oldCss, newCss);
            }
        }
        catch (...) {
            // Return empty if no CSS replacement found or error occurs
        }
        return std::make_pair("", "");
    }

}