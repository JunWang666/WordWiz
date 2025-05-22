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

// 静态成员变量定义
// Poco::AutoPtr<Poco::Data::Session> SettingsManager::_pSession; // 替换
std::unique_ptr<Poco::Data::Session> SettingsManager::_pSession; // 使用 std::unique_ptr

std::string SettingsManager::_dbPath = WordWiz::Data::FilePathProvider::GetAppLocalFolderPath() + "AppData.db";
Poco::FastMutex SettingsManager::_mutex;
bool SettingsManager::_isInitialized = false;

void SettingsManager::initialize(const std::string& dbPath) {
    Poco::FastMutex::ScopedLock lock(_mutex); // 保证线程安全

    if (_isInitialized && _dbPath == dbPath && _pSession && _pSession->isConnected()) {
        return;
    }

    if (_pSession && _pSession->isConnected()) {
        try {
            _pSession->close();
        }
        catch (const Poco::Exception& e) {
            // 在实际应用中记录关闭旧会话时发生的错误
            // 例如: Poco::Logger::get("SettingsManager").error("Error closing previous session: " + e.displayText());
        }
    }
    _pSession.reset(); // 使用 reset() 清空 unique_ptr

    _dbPath = dbPath;
    try {
        Poco::Data::SQLite::Connector::registerConnector();

        Poco::Path path(_dbPath);
        Poco::File dir(path.parent());
        if (!dir.exists()) {
            dir.createDirectories();
        }

        // _pSession = new Poco::Data::Session("SQLite", _dbPath); // 替换
        _pSession = std::make_unique<Poco::Data::Session>("SQLite", _dbPath); // 使用 std::make_unique

        if (!_pSession || !_pSession->isConnected()) {
            _isInitialized = false;
            _pSession.reset(); // 确保在出错时 unique_ptr 也被清空
            throw Poco::Data::ConnectionFailedException("Failed to connect to SQLite database: " + _dbPath);
        }

        createTableIfNotExists();
        _isInitialized = true;
    }
    catch (const Poco::Exception& e) {
        _isInitialized = false;
        _pSession.reset(); // 出错时确保会话指针为空
        // Poco::Logger::get("SettingsManager").critical("Failed to initialize SQLite settings: " + e.displayText());
        throw;
    }
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
