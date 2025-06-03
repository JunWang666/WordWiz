#pragma once
#include <memory>
#include <string>
#include <vector>
#include "Poco/Data/Session.h"
#include "Poco/Data/SQLite/Connector.h"
#include "Poco/Mutex.h"
#include "Poco/Data/RecordSet.h"

namespace WordWizServices::Database
{
    class DatabaseManager
    {
    public:
        // Constructor with custom database path
        explicit DatabaseManager(const std::string& dbPath = "");
        ~DatabaseManager();
        
        // Disable copy constructor and assignment operator
        DatabaseManager(const DatabaseManager&) = delete;
        DatabaseManager& operator=(const DatabaseManager&) = delete;
        
        // Enable move constructor and assignment operator
        DatabaseManager(DatabaseManager&&) = default;
        DatabaseManager& operator=(DatabaseManager&&) = default;
        
        // Database lifecycle
        void initialize();
        void shutdown();
        bool isInitialized();
       
        
        // Basic database query operations
        Poco::Data::RecordSet executeQuery(const std::string& sql, const std::vector<std::string>& params = {});
        std::string executeScalarQuery(const std::string& sql, const std::vector<std::string>& params = {});    
    
    private:
        std::unique_ptr<Poco::Data::Session> _pSession;
        std::string _dbPath;
        mutable Poco::FastMutex _mutex;
        bool _isInitialized = false;
    };
}