#include "pch.h"
#include "WordFavorite.h"
#include "database.h"         // 引用我们强大的数据库管理器
#include "FilePathProvider.h"   // 用于获取 LocalState 路径
#include "logger.h"             // 用于记录日志
#include <chrono>               // 用于获取当前时间
#include <iomanip>              // 用于格式化时间
#include <sstream>              // 用于格式化时间

// 使用 using 来简化代码
using namespace WordWizServices::Database;
using namespace WordWizServices::Data;

namespace
{
    // 获取当前时间的ISO 8601格式字符串 (e.g., "YYYY-MM-DD HH:MM:SS")
    std::string getCurrentTimestamp()
    {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }

    // 使用静态局部变量来维护一个单例的数据库连接
    // 这样可以避免每次调用都重新打开和关闭数据库，效率更高
    DatabaseManager& getDb()
    {
        // 静态变量只会在第一次调用此函数时初始化
        static DatabaseManager dbManager(FilePathProvider::GetAppLocalFolderPath() + "\\favoriteWords.db");

        // 确保表已创建
        static bool table_created = false;
        if (!table_created)
        {
            try
            {
                // 使用 "IF NOT EXISTS" 确保只有在表不存在时才创建
                dbManager.executeQuery(
                    "CREATE TABLE IF NOT EXISTS FavoriteWords ("
                    "word TEXT PRIMARY KEY, "
                    "level INTEGER, "
                    "time TEXT);"
                );
                table_created = true;
            }
            catch (const std::exception& e)
            {
                WordWizServices::Log::LogMessage(L"Failed to create FavoriteWords table: " + winrt::to_hstring(e.what()));
                // 即使失败，也标记为true，避免反复尝试
                table_created = true;
            }
        }
        return dbManager;
    }
} // namespace

namespace WordWizModules::WordFavorite {

    bool isWordFavorite(winrt::hstring const& word)
    {
        try
        {
            auto& db = getDb(); // 获取数据库连接
            std::string sWord = winrt::to_string(word);

            // 查询该单词是否存在于收藏表中
            std::string result = db.executeScalarQuery(
                "SELECT COUNT(*) FROM FavoriteWords WHERE word = ?", { sWord }
            );
            WordWizServices::Log::LogMessage(L"词汇" + winrt::to_hstring(word) + L"的收藏状态为" + winrt::to_hstring(!result.empty() && std::stoi(result) > 0));
            // executeScalarQuery 返回的是字符串，需要转换
            return !result.empty() && std::stoi(result) > 0;
        }
        catch (const std::exception& e)
        {
            WordWizServices::Log::LogMessage(L"isWordFavorite failed: " + winrt::to_hstring(e.what()));
            return false;
        }
    }

    void setWordFavorite(winrt::hstring const& word, bool target)
    {
        try
        {
            auto& db = getDb(); // 获取数据库连接
            std::string sWord = winrt::to_string(word);

            WordWizServices::Log::LogMessage(L"设置词汇" + winrt::to_hstring(word) + L"为" + winrt::to_hstring(target));

            if (target) // 如果目标是“收藏”
            {
                // 使用 INSERT OR REPLACE 来插入或更新记录
                // 如果单词已存在，会更新它的时间和等级
                db.executeQuery(
                    "INSERT OR REPLACE INTO FavoriteWords (word, level, time) VALUES (?, ?, ?)",
                    { sWord, "1", getCurrentTimestamp() } // level 统一为 1，时间为当前时间
                );
            }
            else // 如果目标是“取消收藏”
            {
                db.executeQuery("DELETE FROM FavoriteWords WHERE word = ?", { sWord });
            }
        }
        catch (const std::exception& e)
        {
            WordWizServices::Log::LogMessage(L"setWordFavorite failed: " + winrt::to_hstring(e.what()));
        }
    }

    void switchWordFavorite(winrt::hstring const& word)
    {
        // 这个函数逻辑很简单：先检查当前状态，然后设置为相反的状态
        bool currentState = isWordFavorite(word);
        setWordFavorite(word, !currentState);
    }

}