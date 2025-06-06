#pragma once

#include <string>
#include <vector>
#include <future>
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Storage.Pickers.h>
#include "database.h"

namespace WordWizServices::Dictionary
{
	class DictionaryImporter
	{
	public:
		DictionaryImporter();
		~DictionaryImporter() = default;

		// 禁用拷贝构造和赋值
		DictionaryImporter(const DictionaryImporter&) = delete;
		DictionaryImporter& operator=(const DictionaryImporter&) = delete;

		// 允许移动构造和赋值
		DictionaryImporter(DictionaryImporter&&) = default;
		DictionaryImporter& operator=(DictionaryImporter&&) = default;

		/**
		 * @brief 导入词典文件到应用程序LocalState目录
		 * @param sourceFilePath 源文件路径
		 * @return std::future<bool> 异步返回导入是否成功
		 */
		std::future<bool> ImportDictionaryAsync(const std::string& sourceFilePath);

		/**
		 * @brief 批量导入词典文件
		 * @param sourceFilePaths 源文件路径列表
		 * @return std::future<std::vector<bool>> 异步返回每个文件的导入结果
		 */
		std::future<std::vector<bool>> ImportDictionariesAsync(const std::vector<std::string>& sourceFilePaths);

		/**
		 * @brief 获取词典目录路径
		 * @return std::string 词典目录的完整路径
		 */
		std::string GetDictionariesFolderPath();

		/**
		 * @brief 创建词典目录（如果不存在）
		 * @return bool 创建是否成功
		 */
		bool EnsureDictionariesFolderExists();

	private:
		/**
		 * @brief 复制单个文件
		 * @param sourceFilePath 源文件路径
		 * @param targetFilePath 目标文件路径
		 * @return bool 复制是否成功
		 */
		bool CopyFile(const std::string& sourceFilePath, const std::string& targetFilePath);
		/**
	     * @brief 验证文件是否为有效的词典文件
	     * @param filePath 文件路径
	     * @return bool 文件是否有效
	     */
		bool IsValidDictionaryFile(const std::string& filePath);

		/**
		 * @brief 从SQLite词典文件中读取ID
		 * @param filePath SQLite文件路径
		 * @return std::string 词典ID，如果读取失败返回空字符串
		 */
		std::string GetDictionaryIdFromDatabase(const std::string& filePath);
	};
}
