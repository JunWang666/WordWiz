#include "pch.h"
#include "DictionaryImpoter.h"
#include "FilePathProvider.h"
#include "logger.h"
#include <filesystem>
#include <fstream>
#include <thread>
#include <algorithm>

namespace WordWizServices::Dictionary
{
	DictionaryImporter::DictionaryImporter()
	{
		// 确保词典目录存在
		EnsureDictionariesFolderExists();
	}

	std::future<bool> DictionaryImporter::ImportDictionaryAsync(const std::string& sourceFilePath)
	{
		return std::async(std::launch::async, [this, sourceFilePath]() -> bool
		{
			try
			{
				// 处理文件路径，去除可能的引号
				std::string cleanFilePath = sourceFilePath;

				// 安全检查：只有在路径长度足够且首尾确实都是引号时才去除
				if (!cleanFilePath.empty())
				{
					// 检查是否同时以引号开始和结束
					if (cleanFilePath.size() >= 2 &&
						cleanFilePath[0] == '"' &&
						cleanFilePath[cleanFilePath.size() - 1] == '"')
					{
						// 去除首尾引号
						cleanFilePath = cleanFilePath.substr(1, cleanFilePath.size() - 2);
						Log::LogMessage(L"检测到源文件路径包含引号，已去除引号: " + winrt::to_hstring(cleanFilePath));

						// 验证去除引号后的路径不为空
						if (cleanFilePath.empty())
						{
							Log::LogMessage(L"导入失败：去除引号后源文件路径为空");
							return false;
						}
					}
				}

				// 验证源文件是否存在
				if (!std::filesystem::exists(cleanFilePath))
				{
					Log::LogMessage(L"源文件不存在: " + winrt::to_hstring(cleanFilePath));
					return false;
				}

				// 验证是否为有效词典文件
				if (!IsValidDictionaryFile(cleanFilePath))
				{
					Log::LogMessage(L"无效的词典文件: " + winrt::to_hstring(cleanFilePath));
					return false;
				} // 确保目标目录存在
				if (!EnsureDictionariesFolderExists())
				{
					Log::LogMessage(L"无法创建词典目录");
					return false;
				}

				// 从SQLite数据库中读取词典ID
				std::string dictionaryId = GetDictionaryIdFromDatabase(cleanFilePath);
				if (dictionaryId.empty())
				{
					Log::LogMessage(L"无法从数据库中读取词典ID: " + winrt::to_hstring(cleanFilePath));
					return false;
				}

				// 构建目标文件路径，使用词典ID命名
				std::string targetFileName = dictionaryId + ".db";
				std::string targetFilePath = GetDictionariesFolderPath() + "\\" + targetFileName;

				// 检查目标文件是否已存在
				if (std::filesystem::exists(targetFilePath))
				{
					Log::LogMessage(L"文件已存在，将覆盖: " + winrt::to_hstring(targetFilePath));
				} // 复制文件
				bool success = CopyFile(cleanFilePath, targetFilePath);
				if (success)
				{
					Log::LogMessage(L"词典导入成功: " + winrt::to_hstring(targetFileName));
				}
				else
				{
					Log::LogMessage(L"词典导入失败: " + winrt::to_hstring(targetFileName));
				}

				return success;
			}
			catch (const std::exception& e)
			{
				Log::LogMessage(L"导入词典时发生异常: " + winrt::to_hstring(e.what()));
				return false;
			}
			catch (...)
			{
				Log::LogMessage(L"导入词典时发生未知异常");
				return false;
			}
		});
	}

	std::future<std::vector<bool>> DictionaryImporter::ImportDictionariesAsync(
		const std::vector<std::string>& sourceFilePaths)
	{
		return std::async(std::launch::async, [this, sourceFilePaths]() -> std::vector<bool>
		{
			std::vector<bool> results;
			results.reserve(sourceFilePaths.size());

			for (const auto& filePath : sourceFilePaths)
			{
				auto future = ImportDictionaryAsync(filePath);
				results.push_back(future.get());
			}

			return results;
		});
	}

	std::string DictionaryImporter::GetDictionariesFolderPath()
	{
		std::string localFolderPath = Data::FilePathProvider::GetAppLocalFolderPath();
		if (localFolderPath.empty())
		{
			return "";
		}
		return localFolderPath + "\\Dictionaries";
	}

	bool DictionaryImporter::EnsureDictionariesFolderExists()
	{
		try
		{
			std::string dictionariesPath = GetDictionariesFolderPath();
			if (dictionariesPath.empty())
			{
				Log::LogMessage(L"无法获取词典目录路径");
				return false;
			}

			// 使用std::filesystem创建目录
			std::filesystem::path dirPath(dictionariesPath);
			if (!std::filesystem::exists(dirPath))
			{
				bool created = std::filesystem::create_directories(dirPath);
				if (created)
				{
					Log::LogMessage(L"词典目录创建成功: " + winrt::to_hstring(dictionariesPath));
				}
				else
				{
					Log::LogMessage(L"词典目录创建失败: " + winrt::to_hstring(dictionariesPath));
					return false;
				}
			}

			return true;
		}
		catch (const std::exception& e)
		{
			Log::LogMessage(L"创建词典目录时发生异常: " + winrt::to_hstring(e.what()));
			return false;
		}
		catch (...)
		{
			Log::LogMessage(L"创建词典目录时发生未知异常");
			return false;
		}
	}

	bool DictionaryImporter::CopyFile(const std::string& sourceFilePath, const std::string& targetFilePath)
	{
		try
		{
			std::filesystem::path sourcePath(sourceFilePath);
			std::filesystem::path targetPath(targetFilePath);

			// 使用std::filesystem::copy_file进行文件复制
			auto options = std::filesystem::copy_options::overwrite_existing;
			bool copied = std::filesystem::copy_file(sourcePath, targetPath, options);

			return copied;
		}
		catch (const std::filesystem::filesystem_error& e)
		{
			Log::LogMessage(L"文件复制错误: " + winrt::to_hstring(e.what()));
			return false;
		}
		catch (const std::exception& e)
		{
			Log::LogMessage(L"复制文件时发生异常: " + winrt::to_hstring(e.what()));
			return false;
		}
		catch (...)
		{
			Log::LogMessage(L"复制文件时发生未知异常");
			return false;
		}
	}

	bool DictionaryImporter::IsValidDictionaryFile(const std::string& filePath)
	{
		try
		{
			std::filesystem::path path(filePath);
			std::string extension = path.extension().string();

			// 转换为小写进行比较
			std::transform(extension.begin(), extension.end(), extension.begin(), tolower);

			// 只支持SQLite数据库文件
			return (extension == ".db" || extension == ".sqlite");
		}
		catch (...)
		{
			return false;
		}
	}

	std::string DictionaryImporter::GetDictionaryIdFromDatabase(const std::string& filePath)
	{
		try
		{
			// 处理文件路径，去除可能的引号
			std::string cleanFilePath = filePath;

			Log::LogMessage(L"开始从数据库读取词典ID: " + winrt::to_hstring(cleanFilePath));

			// 创建数据库管理器实例
			Database::DatabaseManager dbManager(cleanFilePath);
			dbManager.initialize();

			if (!dbManager.isInitialized())
			{
				Log::LogMessage(L"无法初始化数据库: " + winrt::to_hstring(cleanFilePath));
				return "";
			}

			std::string sql = "SELECT AttributeValue FROM info WHERE AttributeName = ?";
			std::vector<std::string> params = {"ID"};

			try
			{
				std::string dictionaryId = dbManager.executeScalarQuery(sql, params);

				Log::LogMessage(L"查询执行完成，返回值长度: " + winrt::to_hstring(dictionaryId.length()));

				dbManager.shutdown(); // 处理查询结果 - 如果查不到ID就拒绝导入
				if (dictionaryId.empty())
				{
					Log::LogMessage(L"导入失败：未找到词典ID记录（AttributeName='ID'），拒绝导入: " + winrt::to_hstring(cleanFilePath));
					throw std::exception();
				}

				Log::LogMessage(
					L"成功读取词典ID: \"" + winrt::to_hstring(dictionaryId) + L"\" 来自文件: " + winrt::to_hstring(
						cleanFilePath));
				return dictionaryId;
			}
			catch (const std::exception& queryEx)
			{
				dbManager.shutdown();
				Log::LogMessage(
					L"导入失败：执行数据库查询时发生异常: " + winrt::to_hstring(queryEx.what()) + L" 文件: " + winrt::to_hstring(
						cleanFilePath));
				return "";
			}
		}
		catch (const std::exception& e)
		{
			Log::LogMessage(L"读取词典ID时发生异常: " + winrt::to_hstring(e.what()) + L" 文件: " + winrt::to_hstring(filePath));
			return "";
		}
		catch (...)
		{
			Log::LogMessage(L"读取词典ID时发生未知异常，文件: " + winrt::to_hstring(filePath));
			return "";
		}
	}
}
