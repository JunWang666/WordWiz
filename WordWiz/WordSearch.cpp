// WordSearch.cpp
#include "pch.h"
#include "WordSearch.h"
#include "WordSearch.g.cpp" 
#include "WordItem.h"

// 新增包含 for std::this_thread::sleep_for
#include <thread>
#include <chrono>

using namespace winrt;
using namespace Windows::Foundation::Collections;
using namespace WordWiz;

namespace winrt::WordWiz::implementation
{
    Windows::Foundation::Collections::IVector<WordWiz::WordItem> WordSearch::Search(winrt::hstring const& query)
    {
        // 模拟耗时计算
        std::this_thread::sleep_for(std::chrono::seconds(1));

        auto results = winrt::single_threaded_observable_vector<WordWiz::WordItem>();

        // 如果查询为空，可以提前返回空结果，避免后续处理
        if (query.empty())
        {
            return results;
        }

        auto item1 = winrt::make<WordWiz::implementation::WordItem>(query, query);
        results.Append(item1);

        winrt::hstring queryWithTest = query + L"Test";
        auto item2 = winrt::make<WordWiz::implementation::WordItem>(queryWithTest, queryWithTest);
        results.Append(item2);

        return results;
    }
}
