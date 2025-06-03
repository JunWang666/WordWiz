// WordSearch.cpp
#include "pch.h"
#include "WordSearch.h"
#include "WordSearch.g.cpp"
#include "WordItem.h"
#include "DictionaryItemInWordDetail.h"
#include "database.h"
#include "logger.h"
#include "FilePathProvider.h"

using namespace winrt;
using namespace Windows::Foundation::Collections;

namespace winrt::WordWiz::implementation
{   
    WordSearch::WordSearch()
    {

    }

    WordSearch::~WordSearch()
    {
        
    }
    
    Windows::Foundation::Collections::IVector<WordWiz::WordItem> WordSearch::Search(winrt::hstring const& query)
    {
        auto results = winrt::single_threaded_observable_vector<WordWiz::WordItem>();
        if (query.empty())
        {
            return results;
        }        
          
        std::string sQuery = winrt::to_string(query);
        
        auto main_db = WordWizServices::Database::DatabaseManager(WordWizServices::Data::FilePathProvider::GetAppLocalFolderPath() + "\\words.sqlite");

        try {
            std::string searchQuery = "%" + sQuery + "%";
            auto rs = main_db.executeQuery("SELECT keyword, definition_html FROM word WHERE keyword LIKE ?", {searchQuery});
            
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
        
        try {
            // Query to get available dictionaries for a word
            std::string sWord = winrt::to_string(word);

			auto dict_db = WordWizServices::Database::DatabaseManager(WordWizServices::Data::FilePathProvider::GetAppLocalFolderPath() + "\\dictionaries\\output.db");

			auto uuid = dict_db.executeScalarQuery("SELECT AttributeValue FROM info Where AttributeName = 'ID'");
            auto title = dict_db.executeScalarQuery("SELECT AttributeValue FROM info Where AttributeName = 'Title'");

            auto rs = dict_db.executeQuery("SELECT definition_html FROM word WHERE keyword = ?", { sWord });
            
            if (rs.rowCount() > 0) {
                dictionaries.Append(WordWiz::DictionaryItemInWordDetail{
                            winrt::to_hstring(uuid),
                            winrt::to_hstring(title),
                            winrt::to_hstring(title)
                    });
            }
        }
        catch (const std::exception& e) {
            WordWizServices::Log::LogMessage(L"Exception in GetAvailableDictionaries: " + winrt::to_hstring(e.what()));
        }
        catch (...) {
            WordWizServices::Log::LogMessage(L"Unknown exception in GetAvailableDictionaries");
        }
        
        // Log search results
        winrt::hstring logMessage = L"Search for '" + word + L"': ";
        if (dictionaries.Size() > 0)
        {
            logMessage = logMessage + L" dictionaries: [";
            for (auto const& item : dictionaries)
            {
                logMessage = logMessage + item.DisplayName() + L",";
            }
            logMessage = logMessage + L"]";
        }
        WordWizServices::Log::LogMessage(logMessage);

        return dictionaries;
    }
      
    winrt::hstring WordSearch::GetDictionaryHtmlContent(winrt::hstring const& word, winrt::hstring const& dictionaryId)
    {
        if (word.empty() || dictionaryId.empty()) {
            return L"Error: Invalid parameters.";
        }

        std::string sWord = winrt::to_string(word);
        std::string sDictionaryId = winrt::to_string(dictionaryId);

        try {
            auto dict_db = WordWizServices::Database::DatabaseManager(WordWizServices::Data::FilePathProvider::GetAppLocalFolderPath() + "\\Dictionaries\\output.db");

            // Get the definition HTML content from the database
            auto rs = dict_db.executeQuery("SELECT definition_html FROM word WHERE keyword = ?", { sWord });

            if (rs.rowCount() == 0 || !rs.moveFirst()) {
                return L"<p>Definition not found for this dictionary.</p>";
            }

            std::string htmlContent = rs["definition_html"].convert<std::string>();

            // 检查是否包含重定向链接
            std::string redirectPattern = "@@@LINK=";
            size_t redirectPos = htmlContent.find(redirectPattern);
            if (redirectPos != std::string::npos) {
                // 提取重定向的目标单词
                size_t startPos = redirectPos + redirectPattern.length();
                size_t endPos = htmlContent.find(' ', startPos);
                if (endPos == std::string::npos) {
                    endPos = htmlContent.length();
                }

                std::string targetWord = htmlContent.substr(startPos, endPos - startPos);
                // 删除可能存在的HTML标签或其他特殊字符
                targetWord.erase(std::remove(targetWord.begin(), targetWord.end(), '\r'), targetWord.end());
                targetWord.erase(std::remove(targetWord.begin(), targetWord.end(), '\n'), targetWord.end());

                // 递归调用以获取目标单词的内容
                winrt::hstring redirectContent = GetDictionaryHtmlContent(winrt::to_hstring(targetWord), dictionaryId);

                // 在结果前添加重定向标记
                return L"<div class=\"redirect-notice\">从 <strong>"+ 
                    winrt::to_hstring(word) + 
                    L"</strong> 重定向到 <strong>" +
                    winrt::to_hstring(targetWord) +
                    L"</strong></div>" + redirectContent;
            }

            // Apply CSS replacements directly within this method
            try {
                auto old_css = dict_db.executeScalarQuery("SELECT AttributeValue FROM info WHERE AttributeName = 'CSS_TARGET_HREF'");
                auto new_css = dict_db.executeScalarQuery("SELECT AttributeValue FROM info WHERE AttributeName = 'CSS_REPLACEMENT_CONTENT'");

                // 检查是否获取到了有效的CSS路径
                if (!old_css.empty() && !new_css.empty()) {
                    // 在HTML内容中查找和替换CSS引用
                    std::string searchStr = old_css;
                    std::string replaceStr = new_css;

                    size_t pos = 0;
                    while ((pos = htmlContent.find(searchStr, pos)) != std::string::npos) {
                        htmlContent.replace(pos, searchStr.length(), replaceStr);
                        pos += replaceStr.length();
                    }

                    WordWizServices::Log::LogMessage(L"CSS replacement applied: " +
                        winrt::to_hstring(old_css) + L" -> " + winrt::to_hstring(new_css));
                }
            }
            catch (...) {
                WordWizServices::Log::LogMessage(L"Warning: Unable to apply CSS replacements for dictionary: " + dictionaryId);
            }
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