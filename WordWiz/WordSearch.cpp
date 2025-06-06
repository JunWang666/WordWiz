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

	IVector<WordWiz::WordItem> WordSearch::Search(const hstring& query)
	{
		auto results = winrt::single_threaded_observable_vector<WordWiz::WordItem>();
		if (query.empty())
		{
			return results;
		}

		std::string sQuery = to_string(query);

		auto main_db = WordWizServices::Database::DatabaseManager(
			WordWizServices::Data::FilePathProvider::GetAppLocalFolderPath() + "\\words.sqlite");

		std::string processedContainsQuery = "%" + sQuery + "%";
		std::string exactMatch = sQuery;
		std::string startsWith = sQuery + "%";
		std::string endsWith = "%" + sQuery;


		// 2. 直接将参数值嵌入到 SQL 字符串中 (!!! 注意 SQL 注入风险 !!!)
		//     如果字符串包含单引号，需要进行额外的转义 (用两个单引号代替一个)
		//     例如：SQLite 的字符串字面量是单引号包围的，如果参数本身有单引号，则需要 ' -> ''
		auto escapeSingleQuotes = [](const std::string& s)
		{
			std::string result;
			result.reserve(s.length() * 2); // 预留空间
			for (char c : s)
			{
				if (c == '\'')
				{
					result += "''"; // 双引号转义
				}
				else
				{
					result += c;
				}
			}
			return result;
		};

		std::string sqlQuery =
			"SELECT keyword, definition_html FROM word "
			"WHERE keyword LIKE '" + escapeSingleQuotes(processedContainsQuery) + "' ESCAPE '" + "'\'" + "' "
			// 主查询的模糊匹配
			"ORDER BY CASE "
			"    WHEN keyword = '" + escapeSingleQuotes(exactMatch) + "' THEN 0 " // 精确匹配
			"    WHEN keyword LIKE '" + escapeSingleQuotes(startsWith) + "' ESCAPE '" + "'\'" + "' THEN 1 " // 以...开头
			"    WHEN keyword LIKE '" + escapeSingleQuotes(endsWith) + "' ESCAPE '" + "'\'" + "' THEN 2 " // 以...结尾
			"    ELSE 3 "
			"END, "
			"LENGTH(keyword) ASC, "
			"keyword COLLATE NOCASE ASC "
			"LIMIT 100";

		// 3. 调用没有 bind 的 executeQuery 版本
		try
		{
			auto rs = main_db.executeQuery(sqlQuery); // 直接传入完整的 SQL 字符串

			if (rs.rowCount() == 0)
			{
				WordWizServices::Log::LogMessage(L"Search for '" + query + L"': No records found in database.");
				return results;
			}

			bool more = rs.moveFirst();
			while (more)
			{
				if (!rs["keyword"].isEmpty())
				{
					auto keyword = rs["keyword"].convert<std::string>();

					auto item = winrt::make<WordItem>(
						to_hstring(keyword),
						to_hstring("Definition for " + keyword)
					);
					results.Append(item);
				}
				more = rs.moveNext();
			}
		}
		catch (const std::exception& e)
		{
			WordWizServices::Log::LogMessage(L"Exception in Search: " + to_hstring(e.what()));
		}
		catch (...)
		{
			WordWizServices::Log::LogMessage(L"Unknown exception in Search");
		}

		// Log search results
		hstring logMessage = L"Search for '" + query + L"': " + to_hstring(results.Size()) + L" results found.";
		if (results.Size() > 0)
		{
			logMessage = logMessage + L" Keywords: [";
			unsigned int count = 0;
			for (const auto& item : results)
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

	IVector<WordWiz::DictionaryItemInWordDetail> WordSearch::GetAvailableDictionaries(const hstring& word)
	{
		auto dictionaries = winrt::single_threaded_vector<WordWiz::DictionaryItemInWordDetail>();

		try
		{
			// Query to get available dictionaries for a word
			std::string sWord = to_string(word);

			auto dict_db = WordWizServices::Database::DatabaseManager(
				WordWizServices::Data::FilePathProvider::GetAppLocalFolderPath() + "\\dictionaries\\output.db");

			auto uuid = dict_db.executeScalarQuery("SELECT AttributeValue FROM info Where AttributeName = 'ID'");
			auto title = dict_db.executeScalarQuery("SELECT AttributeValue FROM info Where AttributeName = 'Title'");

			auto rs = dict_db.executeQuery("SELECT definition_html FROM word WHERE keyword = ?", {sWord});

			if (rs.rowCount() > 0)
			{
				dictionaries.Append(WordWiz::DictionaryItemInWordDetail{
					to_hstring(uuid),
					to_hstring(title),
					to_hstring(title)
				});
			}
		}
		catch (const std::exception& e)
		{
			WordWizServices::Log::LogMessage(L"Exception in GetAvailableDictionaries: " + to_hstring(e.what()));
		}
		catch (...)
		{
			WordWizServices::Log::LogMessage(L"Unknown exception in GetAvailableDictionaries");
		}

		// Log search results
		hstring logMessage = L"Search for '" + word + L"': ";
		if (dictionaries.Size() > 0)
		{
			logMessage = logMessage + L" dictionaries: [";
			for (const auto& item : dictionaries)
			{
				logMessage = logMessage + item.DisplayName() + L",";
			}
			logMessage = logMessage + L"]";
		}
		WordWizServices::Log::LogMessage(logMessage);

		return dictionaries;
	}

	hstring WordSearch::GetDictionaryHtmlContent(const hstring& word, const hstring& dictionaryId)
	{
		if (word.empty() || dictionaryId.empty())
		{
			return L"Error: Invalid parameters.";
		}

		std::string sWord = to_string(word);
		std::string sDictionaryId = to_string(dictionaryId);

		try
		{
			auto dict_db = WordWizServices::Database::DatabaseManager(
				WordWizServices::Data::FilePathProvider::GetAppLocalFolderPath() + "\\Dictionaries\\output.db");

			// Get the definition HTML content from the database
			auto rs = dict_db.executeQuery("SELECT definition_html FROM word WHERE keyword = ?", {sWord});

			if (rs.rowCount() == 0 || !rs.moveFirst())
			{
				return L"<p>Definition not found for this dictionary.</p>";
			}

			auto htmlContent = rs["definition_html"].convert<std::string>();

			// 检查是否包含重定向链接
			std::string redirectPattern = "@@@LINK=";
			size_t redirectPos = htmlContent.find(redirectPattern);
			if (redirectPos != std::string::npos)
			{
				// 提取重定向的目标单词
				size_t startPos = redirectPos + redirectPattern.length();
				size_t endPos = htmlContent.find(' ', startPos);
				if (endPos == std::string::npos)
				{
					endPos = htmlContent.length();
				}

				std::string targetWord = htmlContent.substr(startPos, endPos - startPos);
				// 删除可能存在的HTML标签或其他特殊字符
				targetWord.erase(std::remove(targetWord.begin(), targetWord.end(), '\r'), targetWord.end());
				targetWord.erase(std::remove(targetWord.begin(), targetWord.end(), '\n'), targetWord.end());

				// 递归调用以获取目标单词的内容
				hstring redirectContent = GetDictionaryHtmlContent(to_hstring(targetWord), dictionaryId);

				// 在结果前添加重定向标记
				return L"<div class=\"redirect-notice\">从 <strong>" +
					to_hstring(word) +
					L"</strong> 重定向到 <strong>" +
					to_hstring(targetWord) +
					L"</strong></div>" + redirectContent;
			}

			// Apply CSS replacements directly within this method
			try
			{
				auto old_css = dict_db.executeScalarQuery(
					"SELECT AttributeValue FROM info WHERE AttributeName = 'CSS_TARGET_HREF'");
				auto new_css = dict_db.executeScalarQuery(
					"SELECT AttributeValue FROM info WHERE AttributeName = 'CSS_REPLACEMENT_CONTENT'");

				// 检查是否获取到了有效的CSS路径
				if (!old_css.empty() && !new_css.empty())
				{
					// 在HTML内容中查找和替换CSS引用
					std::string searchStr = old_css;
					std::string replaceStr = new_css;

					size_t pos = 0;
					while ((pos = htmlContent.find(searchStr, pos)) != std::string::npos)
					{
						htmlContent.replace(pos, searchStr.length(), replaceStr);
						pos += replaceStr.length();
					}

					WordWizServices::Log::LogMessage(L"CSS replacement applied: " +
						to_hstring(old_css) + L" -> " + to_hstring(new_css));
				}
			}
			catch (...)
			{
				WordWizServices::Log::LogMessage(
					L"Warning: Unable to apply CSS replacements for dictionary: " + dictionaryId);
			}
			return to_hstring(htmlContent);
		}
		catch (const std::exception& e)
		{
			WordWizServices::Log::LogMessage(L"Exception in GetDictionaryHtmlContent: " + to_hstring(e.what()));
			return L"<p>Error retrieving definition: " + to_hstring(e.what()) + L"</p>";
		}
		catch (...)
		{
			WordWizServices::Log::LogMessage(L"Unknown exception in GetDictionaryHtmlContent");
			return L"<p>Unknown error retrieving definition</p>";
		}
	}
}
