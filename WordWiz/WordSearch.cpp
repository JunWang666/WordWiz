// WordSearch.cpp
#include "pch.h"
#include "WordSearch.h"
#include "WordSearch.g.cpp" 
#include "WordItem.h"

// 移除: #include <thread>
// 移除: #include <chrono>

using namespace winrt;
using namespace Windows::Foundation::Collections;
// using namespace WordWiz; // 最好在实现内部明确命名空间

namespace winrt::WordWiz::implementation
{
    Windows::Foundation::Collections::IVector<WordWiz::WordItem> WordSearch::Search(winrt::hstring const& query)
    {
        // 移除: std::this_thread::sleep_for(std::chrono::seconds(1));

        auto results = winrt::single_threaded_observable_vector<WordWiz::WordItem>();
        if (query.empty())
        {
            return results;
        }
        auto item1 = winrt::make<WordWiz::implementation::WordItem>(query, query + L" (short explanation)");
        results.Append(item1);
        // ... 其他示例代码 ...
        return results;
    }

    // 新增方法的实现
    Windows::Foundation::Collections::IVector<winrt::hstring> WordSearch::GetAvailableDictionaries(winrt::hstring const& word)
    {
        // 模拟返回字典列表
        auto dictionaryNames = winrt::single_threaded_vector<winrt::hstring>();
        if (!word.empty())
        {
            dictionaryNames.Append(L"词典A");
            dictionaryNames.Append(L"词典B");
            dictionaryNames.Append(L"牛津高阶模拟");
        }
        return dictionaryNames;
    }

    // 在 WordSearch.cpp 的 GetDictionaryHtmlContent 方法中
    winrt::hstring WordSearch::GetDictionaryHtmlContent(winrt::hstring const& word, winrt::hstring const& dictionaryName)
    {
        // 基础模板
        winrt::hstring html_template_start = L"<html style='background-color:transparent;'><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'><style>body{background-color:transparent;color:#333;font-family:sans-serif;margin:15px;padding:0;}h1{color:#0078D4;}p{line-height:1.6;}b,strong{font-weight:bold;}i,em{font-style:italic;}ul{padding-left:20px;}</style></head><body style='background-color:transparent;'>";
        winrt::hstring html_template_end = L"</body></html>";

        winrt::hstring content;

        if (dictionaryName == L"词典A")
        {
            content = L"<h1>" + word + L" - " + dictionaryName + L"</h1><p>这是来自<b>词典A</b>的详细解释（填充内容）。</p><p style='color:green;'>一些特别的笔记。</p>";
        }
        else if (dictionaryName == L"词典B")
        {
            content = L"<h1>" + word + L" - " + dictionaryName + L"</h1><p><i>词典B</i> 说：" + word + L" 是一个非常有趣的词。</p><ul><li>要点1</li><li>要点2</li></ul>";
        }
        else if (dictionaryName == L"牛津高阶模拟") // 这个之前有更详细的样式
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
            content = L"<p>未找到字典 " + dictionaryName + L" 中关于 " + word + L" 的内容。</p>";
        }

        return html_template_start + content + html_template_end;
    }

}