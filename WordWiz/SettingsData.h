#pragma once
#include <pch.h>

#include "Poco/Data/Session.h"
#include "Poco/Data/SQLite/Connector.h"
// #include "Poco/AutoPtr.h" // 移除 AutoPtr
#include <memory>      // 包含 <memory> 以使用 std::unique_ptr
#include "Poco/Mutex.h"
#include <string>
#include <vector>

// 静态配置管理类
// 使用 SQLite 数据库存储和检索应用程序设置
class SettingsManager {
public:
    // 使用指定的 SQLite 数据库文件路径初始化 SettingsManager
    // 在进行任何 get/set 操作之前必须调用此方法
    // dbPath: SQLite 数据库文件的路径 (例如 "settings.db")
    void initialize(const std::string& dbPath);

    // 关闭数据库连接 (可选，通常在程序结束时由 unique_ptr 的析构函数处理)
    void shutdown();

    // 检查 SettingsManager 是否已初始化
    bool isInitialized();

    // --- Getters ---
    std::string getString(const std::string& key, const std::string& defaultValue = "");

    // --- Setters ---
    // 如果 SettingsManager 未初始化，则抛出 Poco::IllegalStateException
    void setString(const std::string& key, const std::string& value);

    // 移除一个键
    void remove(const std::string& key);

	void setTableName(const std::string tableName) {
		TABLE_NAME = tableName;
	}

private:
    SettingsManager() = delete;
    ~SettingsManager() = delete;
    SettingsManager(const SettingsManager&) = delete;
    SettingsManager& operator=(const SettingsManager&) = delete;

    void createTableIfNotExists(); // 创建设置表的辅助函数

    // static Poco::AutoPtr<Poco::Data::Session> _pSession; // 替换为 unique_ptr
    static std::unique_ptr<Poco::Data::Session> _pSession; // POCO 数据库会话

    static std::string _dbPath;         // 数据库文件的路径
    static Poco::FastMutex _mutex;      // 保证线程安全的互斥锁
    static bool _isInitialized;         // 标记是否已初始化

    std::string TABLE_NAME; // 表名
};
