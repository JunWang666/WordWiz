#include <pch.h>

#include "SettingsData.h"
#include "Poco/Data/Statement.h"
#include "Poco/Data/RecordSet.h"
#include "Poco/Data/SQLite/Connector.h"
#include "Poco/Exception.h"
#include "Poco/NumberParser.h"
#include "Poco/NumberFormatter.h"
#include "Poco/String.h" // for Poco::toLower
#include "Poco/File.h"   // For Poco::File
#include "Poco/Path.h"   // For Poco::Path

#include<FilePathProvider.h>
#include<logger.h>

namespace WordWiz::Data {
    // 静态成员变量定义
    // Poco::AutoPtr<Poco::Data::Session> SettingsManager::_pSession; // 替换
    std::unique_ptr<Poco::Data::Session> SettingsManager::_pSession; // 使用 std::unique_ptr

    std::string SettingsManager::_dbPath = WordWiz::Data::FilePathProvider::GetAppLocalFolderPath() + "\\AppData.db";
    Poco::FastMutex SettingsManager::_mutex;
    bool SettingsManager::_isInitialized = false;

    SettingsManager::SettingsManager(const std::string& tableName = "test_table") : TABLE_NAME(tableName) {
        if (!_isInitialized) {
            Poco::FastMutex::ScopedLock lock(_mutex);
            if (!_isInitialized) { // 双重检查锁定
                try {
					WordWizServices::Log::LogMessage("Initializing SettingsManager with database path: " + _dbPath);
                    // 注册 SQLite 连接器
                    Poco::Data::SQLite::Connector::registerConnector();
                    _pSession = std::make_unique<Poco::Data::Session>("SQLite", _dbPath);
                    createTableIfNotExists();
                    _isInitialized = true;
                }
                catch (const Poco::Exception& e) {
                    // Poco::Logger::get("SettingsManager").error("Failed to initialize SettingsManager: " + e.displayText());
                }
            }
        }
    }

	SettingsManager::~SettingsManager() {
		// 析构函数中不需要关闭数据库连接
		// 由 std::unique_ptr 自动管理
		// Poco::Data::SQLite::Connector::unregisterConnector();
	}

    void SettingsManager::shutdown() {
        Poco::FastMutex::ScopedLock lock(_mutex);
        if (_isInitialized && _pSession && _pSession->isConnected()) {
            try {
                _pSession->close();
                // Poco::Logger::get("SettingsManager").information("SQLite session shut down successfully.");
            }
            catch (const Poco::Exception& e) {
                // Poco::Logger::get("SettingsManager").error("Error shutting down SQLite session: " + e.displayText());
            }
        }
        _pSession.reset(); // 使用 reset() 清空 unique_ptr
        _isInitialized = false;
    }


    void SettingsManager::createTableIfNotExists() {
        if (!_pSession || !_pSession->isConnected()) { // _pSession 可以直接用于布尔判断
            throw Poco::IllegalStateException("Session not available or not connected for createTableIfNotExists");
        }
        try {
            *_pSession << "CREATE TABLE IF NOT EXISTS " + TABLE_NAME +
                " (key TEXT PRIMARY KEY NOT NULL, value TEXT)",
                Poco::Data::Keywords::now;
        }
        catch (const Poco::Data::DataException& e) {
            // Poco::Logger::get("SettingsManager").error("Failed to create settings table '" + TABLE_NAME + "': " + e.displayText());
            throw;
        }
    }

    bool SettingsManager::isInitialized() {
        Poco::FastMutex::ScopedLock lock(_mutex);
        return _isInitialized && _pSession && _pSession->isConnected(); // _pSession 可以直接用于布尔判断
    }

    // --- Getters ---
    std::string SettingsManager::getString(const std::string& key, const std::string& defaultValue) {
        Poco::FastMutex::ScopedLock lock(_mutex);
        if (!isInitialized()) { // isInitialized 内部会检查 _pSession
            // Poco::Logger::get("SettingsManager").warning("Attempted to get string from uninitialized SettingsManager for key: " + key);
            return defaultValue;
        }

        std::string valueFromDb;
        try {
            Poco::Data::Statement select(*_pSession); // 使用 _pSession.get() 或直接 *_pSession 都可以
            select << "SELECT value FROM " + TABLE_NAME + " WHERE key = ?",
                Poco::Data::Keywords::into(valueFromDb),
                Poco::Data::Keywords::bind(key),
                Poco::Data::Keywords::now;

            if (select.rowsExtracted() == 0) {
                return defaultValue;
            }
            return valueFromDb;
        }
        catch (const Poco::Exception& e) {
            // Poco::Logger::get("SettingsManager").error("Failed to get string for key '" + key + "': " + e.displayText() + ". Returning default value.");
            return defaultValue;
        }
    }

    // --- Setters ---
    void SettingsManager::setString(const std::string& key, const std::string& value) {
        Poco::FastMutex::ScopedLock lock(_mutex);
        if (!isInitialized()) {
            throw Poco::IllegalStateException("SettingsManager is not initialized. Cannot set string for key: " + key);
        }
        if (key.empty()) {
            throw Poco::InvalidArgumentException("Key cannot be empty when setting string value.");
        }

        try {
            Poco::Data::Statement upsert(*_pSession);
            upsert << "INSERT OR REPLACE INTO " + TABLE_NAME + " (key, value) VALUES (?, ?)",
                Poco::Data::Keywords::bind(key),
                Poco::Data::Keywords::bind(value),
                Poco::Data::Keywords::now;
        }
        catch (const Poco::Exception& e) {
            // Poco::Logger::get("SettingsManager").error("Failed to set string for key '" + key + "': " + e.displayText());
            throw;
        }
    }

    void SettingsManager::remove(const std::string& key) {
        Poco::FastMutex::ScopedLock lock(_mutex);
        if (!isInitialized()) {
            throw Poco::IllegalStateException("SettingsManager is not initialized. Cannot remove key: " + key);
        }
        if (key.empty()) {
            // Poco::Logger::get("SettingsManager").warning("Attempted to remove an empty key.");
            return;
        }

        try {
            Poco::Data::Statement removeStmt(*_pSession);
            removeStmt << "DELETE FROM " + TABLE_NAME + " WHERE key = ?",
                Poco::Data::Keywords::bind(key),
                Poco::Data::Keywords::now;
        }
        catch (const Poco::Exception& e) {
            // Poco::Logger::get("SettingsManager").error("Failed to remove key '" + key + "': " + e.displayText());
            throw;
        }
    }
}