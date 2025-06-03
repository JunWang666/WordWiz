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
{
    WordSearch::WordSearch()
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
    }

    Windows::Foundation::Collections::IVector<WordWiz::WordItem> WordSearch::Search(winrt::hstring const& query)
    {
        auto results = winrt::single_threaded_observable_vector<WordWiz::WordItem>();
        if (query.empty())
        {
            return results;
        }

        std::string sQuery = winrt::to_string(query);
        auto& dbManager = WordWizServices::Database::DatabaseManager::getInstance();
        
        try {
            auto searchResults = dbManager.searchWords(sQuery);
            
            for (const auto& result : searchResults) {
                auto item = winrt::make<WordWiz::implementation::WordItem>(
                    winrt::to_hstring(result.keyword), 
                    winrt::to_hstring("Definition for " + result.keyword)
                );
                results.Append(item);
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
    }     Windows::Foundation::Collections::IVector<WordWiz::DictionaryItemInWordDetail> WordSearch::GetAvailableDictionaries(winrt::hstring const& word)
    {
        auto dictionaries = winrt::single_threaded_vector<WordWiz::DictionaryItemInWordDetail>();
        auto& dbManager = WordWizServices::Database::DatabaseManager::getInstance();
        
        try {
            auto dictionaryInfos = dbManager.getAvailableDictionaries();
            
            for (const auto& info : dictionaryInfos) {
                dictionaries.Append(WordWiz::DictionaryItemInWordDetail{ 
                    winrt::to_hstring(info.uuid),      
                    winrt::to_hstring(info.uuid),      
                    winrt::to_hstring(info.title)      
                });
            }
        }
        catch (const std::exception& e) {
            WordWizServices::Log::LogMessage(L"Exception in GetAvailableDictionaries: " + winrt::to_hstring(e.what()));
        }
        catch (...) {
            WordWizServices::Log::LogMessage(L"Unknown exception in GetAvailableDictionaries");
        }
        
        return dictionaries;
    }     winrt::hstring WordSearch::GetDictionaryHtmlContent(winrt::hstring const& word, winrt::hstring const& dictionaryId)
    {
        if (word.empty() || dictionaryId.empty()) {
            return L"Error: Invalid parameters.";
        }

        std::string sWord = winrt::to_string(word);
        std::string sDictionaryId = winrt::to_string(dictionaryId);
        auto& dbManager = WordWizServices::Database::DatabaseManager::getInstance();
        
        try {
            std::string htmlContent = dbManager.getDictionaryHtmlContent(sWord, sDictionaryId);
            return winrt::to_hstring(htmlContent);
        }
        catch (const std::exception& e) {
            WordWizServices::Log::LogMessage(L"Exception in GetDictionaryHtmlContent: " + winrt::to_hstring(e.what()));
            return L"<p>Error retrieving definition: " + winrt::to_hstring(e.what()) + L"</p>";
        }
        catch (...) {
            WordWizServices::Log::LogMessage(L"Unknown exception in GetDictionaryHtmlContent");
            return L"<p>Unknown error retrieving definition</p>";
        }
    }

}