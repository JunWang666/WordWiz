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


    Poco::Data::RecordSet DatabaseManager::executeQuery(const std::string& sql, std::function<void(Poco::Data::Statement&)> binder)
    {
        Poco::FastMutex::ScopedLock lock(_mutex);
        if (!isInitialized()) {
            throw std::runtime_error("Database not initialized");
        }

        try {
            Poco::Data::Statement select(*_pSession);

            // 这里的 select << sql; 理论上是OK的，它初始化了Statement
            select << sql;

            // 接着调用binder，binder负责将后续的参数绑定到select对象上
            binder(select); // 确保 binder 内部是 select << Poco::Data::Keywords::bind(param);

            select.execute();
            return Poco::Data::RecordSet(select);
        }
        catch (const Poco::Exception& e) {
            // 使用 e.displayText() 获取更详细的错误信息
            WordWizServices::Log::LogMessage(L"Database query error: " + winrt::to_hstring(e.displayText()));
            throw;
        }
    }

    Poco::Data::RecordSet DatabaseManager::executeQuery(const std::string& sql)
    {
        Poco::FastMutex::ScopedLock lock(_mutex);
        if (!isInitialized()) {
            throw std::runtime_error("Database not initialized");
        }

        try {
            Poco::Data::Statement select(*_pSession);
            select << sql; // 只绑定SQL语句，不绑定任何参数

            select.execute();
            return Poco::Data::RecordSet(select);
        }
        catch (const Poco::Exception& e) {
            WordWizServices::Log::LogMessage(L"Database query error (no params): " + winrt::to_hstring(e.displayText()));
            throw;
        }
    }

	Poco::Data::RecordSet DatabaseManager::executeQuery(const std::string& sql, const std::vector<std::string>& params) {
	        Poco::FastMutex::ScopedLock lock(_mutex);
	        if (!isInitialized()) {
	            throw std::runtime_error("Database not initialized");
	        }
	        try {
	            Poco::Data::Statement select(*_pSession);
	            select << sql;
	            // 关键：使用 addBind 进行循环绑定
	            for (const auto& param : params) {
	                select.addBind(Poco::Data::Keywords::bind(param));
	            }
	            select.execute();
	            return Poco::Data::RecordSet(select);
	        }
	        catch (const Poco::Exception& e) {
	            WordWizServices::Log::LogMessage(L"Database query error: " + winrt::to_hstring(e.displayText()));
	            throw;
	        }
	    }

	std::string DatabaseManager::executeScalarQuery(const std::string & sql, const std::vector<std::string>&params)
        {
            // 直接调用接受 vector<string> 参数的 executeQuery 重载
            auto rs = executeQuery(sql, params); // 这里会调用我们上面保留的 executeQuery

            if (rs.rowCount() == 0 || !rs.moveFirst()) {
                return "";
            }
            if (rs.columnCount() == 0 || rs[0].isEmpty()) {
                return "";
            }
            return rs[0].convert<std::string>();
        }
}