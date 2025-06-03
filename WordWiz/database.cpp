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
    DatabaseManager& DatabaseManager::getInstance()
    {
        static DatabaseManager instance;
        return instance;
    }

    void DatabaseManager::initialize()
    {
        if (!_isInitialized) {
            Poco::FastMutex::ScopedLock lock(_mutex);
            if (!_isInitialized) { // Double-checked locking
                try {
                    _dbPath = FilePathProvider::GetAppLocalFolderPath() + "\\Dictionaries\\output.db";
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