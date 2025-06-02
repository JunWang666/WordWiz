// WordSearch.cpp
#include "pch.h"
#include "WordSearch.h"
#include "WordSearch.g.cpp" 
#include "WordItem.h"
#include "DictionaryItemInWordDetail.h"
#include <sqlite3.h> // 假设使用 SQLite，需根据实际数据库调整
#include <vector>
#include <wrl.h> // 用于智能指针管理
#include "DatabaseHelper.h"
using namespace ABI::Windows::Foundation;

using namespace winrt;
using namespace Windows::Foundation::Collections;

namespace winrt::WordWiz::implementation
{
    Windows::Foundation::Collections::IVector<WordWiz::WordItem> WordSearch::Search(winrt::hstring const& query)
    {
        auto results = winrt::single_threaded_observable_vector<WordWiz::WordItem>();
        if (query.empty()) return results;

        // 初始化数据库连接（路径需根据实际情况配置，可通过参数或配置文件传入）
        DatabaseHelper dbHelper;
        if (!dbHelper.OpenDatabase(L"dictionary.db")) // 假设数据库文件为 dictionary.db
        {
            // 处理数据库连接失败（如返回错误提示项）
            auto errorItem = winrt::make<WordWiz::implementation::WordItem>(L"错误", L"无法连接到词典数据库");
            results.Append(errorItem);
            return results;
        }

        // 执行查询
        auto dbResults = dbHelper.QueryWords(query.c_str());

        // 转换为 UI 所需的 WordItem 格式
        for (const auto& item : dbResults)
        {
            // 拆分数据（假设格式为 "单词 | 词性: 释义 [发音]"）
            size_t pos = item.find(L" | ");
            if (pos == std::wstring::npos) continue;

            winrt::hstring word = item.substr(0, pos).c_str();
            winrt::hstring explanation = item.substr(pos + 3).c_str(); // "+3" 跳过 " | "
            auto wordItem = winrt::make<WordWiz::implementation::WordItem>(word, explanation);
            results.Append(wordItem);
        }

        return results;
    } //修改部分

     Windows::Foundation::Collections::IVector<WordWiz::DictionaryItemInWordDetail> WordSearch::GetAvailableDictionaries(winrt::hstring const& word)
    {
        // 模拟返回带字符串ID的字典列表
        auto dictionaries = winrt::single_threaded_vector<WordWiz::DictionaryItemInWordDetail>();
        
        if (!word.empty())
        {
            dictionaries.Append(WordWiz::DictionaryItemInWordDetail{ winrt::hstring(L"dict_a"), winrt::hstring(L"dict_a"), winrt::hstring(L"词典A") });
            dictionaries.Append(WordWiz::DictionaryItemInWordDetail{ winrt::hstring(L"dict_b"), winrt::hstring(L"dict_b"), winrt::hstring(L"词典B") });
            dictionaries.Append(WordWiz::DictionaryItemInWordDetail{ winrt::hstring(L"oxford_advanced"), winrt::hstring(L"oxford_advanced"), winrt::hstring(L"牛津高阶模拟") });
        }
        return dictionaries;
    }


     winrt::hstring WordSearch::GetDictionaryHtmlContent(winrt::hstring const& word, winrt::hstring const& dictionaryId)
    {
        winrt::hstring html_template_start = L"<html style='background-color:transparent;'><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'><style>body{background-color:transparent;color:#333;font-family:sans-serif;margin:15px;padding:0;}h1{color:#0078D4;}p{line-height:1.6;}b,strong{font-weight:bold;}i,em{font-style:italic;}ul{padding-left:20px;}</style></head><body style='background-color:transparent;'>";
        winrt::hstring html_template_end = L"</body></html>";

        winrt::hstring content;

        if (dictionaryId == L"dict_a")
        {
            content = L"<h1>" + word + L" - " + dictionaryId + L"</h1><p>这是来自<b>词典A</b>的详细解释（填充内容）。</p><p style='color:green;'>一些特别的笔记。</p>";
        }
        else if (dictionaryId == L"dict_b")
        {
            content = L"<h1>" + word + L" - " + dictionaryId + L"</h1><p><i>词典B</i> 说：" + word + L" 是一个非常有趣的词。</p><ul><li>要点1</li><li>要点2</li></ul>";
        }
        else if (dictionaryId == L"oxford_advanced") // 这个之前有更详细的样式
        {
            return L"<html style='background-color:transparent;'><head><meta charset='UTF-8'><style>body { background-color:transparent; font-family: sans-serif; margin: 20px; color: #333; } .word { color: #c00; font-size: 2em; } .phonetic { color: #555; } .definition { margin-top: 10px; border-left: 3px solid #007acc; padding-left: 10px; }</style></head>"
                L"<body style='background-color:transparent;'><div class='word'>" + word + L"</div><div class='phonetic'>[ˈwɜːd]</div>"
                L"<div class='definition'><b>(noun)</b> a single distinct meaningful element of speech or writing, used with others (or sometimes alone) to form a sentence and typically shown with a space on either side when written or printed.</div>"
                L"<div class='definition'><b>(verb)</b> express (something) in particular words.</div>"
                L"<p>这是来自<b>牛津高阶模拟词典</b>的详细解释。</p>"
                L"<p>这是来自<b>牛津高阶模拟词典</b>的详细解释。</p>"
                L"<p>这是来自<b>牛津高阶模拟词典</b>的详细解释。</p>"
                L"<p>这是来自<b>牛津高阶模拟词典</b>的详细解释。</p>"
                L"<p>这是来自<b>牛津高阶模拟词典</b>的详细解释。</p>"
                L"<p>这是来自<b>牛津高阶模拟词典</b>的详细解释。</p>"
                L"<p>这是来自<b>牛津高阶模拟词典</b>的详细解释。</p>"
                L"<p>这是来自<b>牛津高阶模拟词典</b>的详细解释。</p>"
                L"<p>这是来自<b>牛津高阶模拟词典</b>的详细解释。</p>"
                L"<p>这是来自<b>牛津高阶模拟词典</b>的详细解释。</p>"
                L"<p>这是来自<b>牛津高阶模拟词典</b>的详细解释。</p>"
                L"<p>这是来自<b>牛津高阶模拟词典</b>的详细解释。</p>"
                L"<p>这是来自<b>牛津高阶模拟词典</b>的详细解释。</p>"
                L"<p>这是来自<b>牛津高阶模拟词典</b>的详细解释。</p>"
                L"<p>这是来自<b>牛津高阶模拟词典</b>的详细解释。</p>"
                L"<p>这是来自<b>牛津高阶模拟词典</b>的详细解释。</p>"
                L"<p>这是来自<b>牛津高阶模拟词典</b>的详细解释。</p>"
                L"<p>这是来自<b>牛津高阶模拟词典</b>的详细解释。</p>"
                L"<p>这是来自<b>牛津高阶模拟词典</b>的详细解释。</p>"
                L"<p>这是来自<b>牛津高阶模拟词典</b>的详细解释。</p>"
                L"</body></html>";
        }
        else
        {
            content = L"<p>未找到字典 " + dictionaryId + L" 中关于 " + word + L" 的内容。</p>";
        }

        return html_template_start + content + html_template_end;
    }

}