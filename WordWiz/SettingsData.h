#pragma once
#include <pch.h>

#include "Poco/Data/Session.h"
#include "Poco/Data/SQLite/Connector.h"
#include <memory>      // 包含 <memory> 以使用 std::unique_ptr
#include "Poco/Mutex.h"
#include <string>
#include <vector>

namespace WordWiz::Data{
    // 使用 SQLite 数据库存储和检索应用程序设置
    class SettingsManager {
    public:

        // 关闭数据库连接 (可选，通常在程序结束时由 unique_ptr 的析构函数处理)
        void shutdown();

        // 检查 SettingsManager 是否已初始化
        bool isInitialized();

        std::string getString(const std::string& key, const std::string& defaultValue = "");

        void setString(const std::string& key, const std::string& value);

        // 移除一个键
        void remove(const std::string& key);

        SettingsManager(const std::string& tableName);
        ~SettingsManager();

    private:
        void createTableIfNotExists(); // 创建设置表的辅助函数

        static std::unique_ptr<Poco::Data::Session> _pSession; // POCO 数据库会话

        static std::string _dbPath;         // 数据库文件的路径
        static Poco::FastMutex _mutex;      // 保证线程安全的互斥锁
        static bool _isInitialized;         // 标记是否已初始化

        std::string TABLE_NAME; // 表名
    };
}