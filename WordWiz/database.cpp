#include <pch.h>
#include "database.h"
#include "FilePathProvider.h"
#include "logger.h"
#include <Poco/Data/Session.h>
#include <Poco/Data/SQLite/Connector.h>
#include "Poco/Data/Statement.h"
#include "Poco/Data/RecordSet.h"
#include "Poco/Exception.h"

using namespace winrt;
using namespace ::WordWizServices::Data; // For FilePathProvider

namespace WordWizServices::Database
{
    DatabaseManager::DatabaseManager(const std::string& dbPath)
    {
        try {
            if (!dbPath.empty()) {
                _dbPath = dbPath;
                this->initialize();
                if (!this->isInitialized()) {
                    throw std::runtime_error("Failed to initialize database connection");
                }
                WordWizServices::Log::LogMessage(L"初始化数据库成功：" + winrt::to_hstring(_dbPath));
            }
            else {
                throw std::invalid_argument("Database path cannot be empty");
            }
        }
        catch (...) {
            WordWizServices::Log::LogMessage("初始化数据库失败");
        }
    }

    DatabaseManager::~DatabaseManager()
    {
        try {
            shutdown();
        }
        catch (...) {
            // Ignore exceptions in destructor
        }
    }

    void DatabaseManager::initialize()
    {
        if (!_isInitialized) {
            Poco::FastMutex::ScopedLock lock(_mutex);
            if (!_isInitialized) { // Double-checked locking
                try {
                    Poco::Data::SQLite::Connector::registerConnector();
                    _pSession = std::make_unique<Poco::Data::Session>("SQLite", _dbPath);
                    _isInitialized = true;
                }
                catch (const Poco::Exception& e) {
                    WordWizServices::Log::LogMessage(L"Failed to initialize database: " + 
                                                   winrt::to_hstring(e.displayText()));
                    throw;
                }
            }
        }
    }

    void DatabaseManager::shutdown()
    {
        Poco::FastMutex::ScopedLock lock(_mutex);
        if (_isInitialized && _pSession && _pSession->isConnected()) {
            try {
                _pSession->close();
            }
            catch (const Poco::Exception& e) {
                WordWizServices::Log::LogMessage(L"Error shutting down database: " + 
                                               winrt::to_hstring(e.displayText()));
            }
        }
        _pSession.reset();
        _isInitialized = false;
    }

    bool DatabaseManager::isInitialized()
    {
        Poco::FastMutex::ScopedLock lock(_mutex);
        return _isInitialized && _pSession && _pSession->isConnected();
    }


    Poco::Data::RecordSet DatabaseManager::executeQuery(const std::string& sql, const std::vector<std::string>& params)
    /**
     * @brief 执行SQL查询并返回结果集
     *
     * @param sql 要执行的SQL查询语句
     * @param params SQL参数列表，用于安全绑定到查询中
     * @return Poco::Data::RecordSet 查询结果集
     * @throws std::runtime_error 如果数据库未初始化
     * @throws Poco::Exception 如果执行查询时发生错误
     */
    {
        Poco::FastMutex::ScopedLock lock(_mutex);
        if (!isInitialized()) {
            throw std::runtime_error("Database not initialized");
        }

        try {
            Poco::Data::Statement select(*_pSession);
            select << sql;
            
            // Bind parameters
            for (const auto& param : params) {
                select, Poco::Data::Keywords::bind(param);
            }
            
            select.execute();
            return Poco::Data::RecordSet(select);
        }
        catch (const Poco::Exception& e) {
            WordWizServices::Log::LogMessage(L"Database query error: " + winrt::to_hstring(e.displayText()));
            throw;
        }
    }

    std::string DatabaseManager::executeScalarQuery(const std::string& sql, const std::vector<std::string>& params)
        /**
         * @brief 执行SQL查询并返回单个标量值
         *
         * @param sql 要执行的SQL查询语句
         * @param params SQL参数列表，用于安全绑定到查询中
         * @return std::string 查询结果的第一行第一列值，如果没有结果则返回空字符串
         * @throws std::runtime_error 如果数据库未初始化
         * @throws Poco::Exception 如果执行查询时发生错误
         */
    {
        auto rs = executeQuery(sql, params);
        if (rs.rowCount() == 0 || !rs.moveFirst()) {
            return "";
        }
        
        if (rs.columnCount() == 0 || rs[0].isEmpty()) {
            return "";
        }
        
        return rs[0].convert<std::string>();
    }
}