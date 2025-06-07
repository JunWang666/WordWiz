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
		Poco::Data::RecordSet executeQuery(const std::string& sql, std::function<void(Poco::Data::Statement&)> binder);
		Poco::Data::RecordSet executeQuery(const std::string& sql, const std::vector<std::string>& params);

		/**
		 * @brief 执行SQL查询并返回结果集，不带任何参数。
		 *
		 * 此函数适用于不需要绑定参数的SQL查询。
		 *
		 * @param sql 要执行的SQL查询语句。
		 * @return Poco::Data::RecordSet 查询结果集。
		 * @throws std::runtime_error 如果数据库未初始化。
		 * @throws Poco::Exception 如果执行查询时发生错误。
		 */
		Poco::Data::RecordSet executeQuery(const std::string& sql);

		/**
		 * @brief 执行SQL查询并返回单个标量值
		 *
		 * @param sql 要执行的SQL查询语句。
		 * @param params SQL参数列表，用于安全绑定到查询中。
		 * @return std::string 查询结果的第一行第一列值，如果没有结果则返回空字符串。
		 * @throws std::runtime_error 如果数据库未初始化。
		 * @throws Poco::Exception 如果执行查询时发生错误。
		 */
		std::string executeScalarQuery(const std::string& sql, const std::vector<std::string>& params = {});

	private:
		std::unique_ptr<Poco::Data::Session> _pSession;
		std::string _dbPath;
		mutable Poco::FastMutex _mutex;
		bool _isInitialized = false;
	};
}
