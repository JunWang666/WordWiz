#include "pch.h"
#include "FilePathProvider.h"
#include <winrt/Windows.Storage.h>
#include <winrt/base.h>
#include <locale> // 用于 wstring_convert
#include <codecvt> // 用于 wstring_convert

// 辅助函数：将 std::wstring (通常是 UTF-16 on Windows) 转换为 std::string (UTF-8)
std::string WideStringToString_UTF8(const std::wstring& wstr)
{
	if (wstr.empty())
	{
		return std::string();
	}
	// 计算转换后的UTF-8字符串所需的大小
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.length(), NULL, 0, NULL, NULL);
	if (size_needed <= 0)
	{
		return std::string();
	}
	std::string strTo(size_needed, 0);
	// 执行转换
	int chars_converted = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.length(), &strTo[0], size_needed,
	                                          NULL, NULL);
	if (chars_converted <= 0)
	{
		return std::string();
	}
	return strTo;
}


namespace WordWizServices::Data
{
	std::string FilePathProvider::GetAppLocalFolderPath()
	{
		try
		{
			// 获取应用的本地数据文件夹对象
			auto applicationData = winrt::Windows::Storage::ApplicationData::Current();
			if (!applicationData)
			{
				WordWizServices::Log::LogMessage(L"无法获取ApplicationData对象。");
				return std::string();
			}

			winrt::Windows::Storage::StorageFolder localFolder = applicationData.LocalFolder();
			if (!localFolder)
			{
				WordWizServices::Log::LogMessage(L"LocalFolder不可用。");
				return std::string();
			}

			// 获取文件夹路径的 hstring (宽字符)
			winrt::hstring path_hstring = localFolder.Path();
			if (path_hstring.empty())
			{
				WordWizServices::Log::LogMessage(L"LocalFolder路径为空。");
				return std::string();
			}

			// 将 winrt::hstring (其 .c_str() 返回 const wchar_t*) 转换为 std::wstring
			std::wstring wide_path(path_hstring.c_str());
			// 使用辅助函数将 std::wstring (UTF-16) 转换为 std::string (UTF-8)
			return WideStringToString_UTF8(wide_path);
		}
		catch (const winrt::hresult_error& e)
		{
			std::wstring errorMsg = L"获取本地文件夹失败，错误代码: 0x" +
				std::to_wstring(static_cast<uint32_t>(e.code())) + L" - " + e.message().c_str();
			WordWizServices::Log::LogMessage(errorMsg);
			return std::string();
		}
		catch (...)
		{
			WordWizServices::Log::LogMessage(L"获取应用本地文件夹路径失败。请检查应用权限或文件夹是否存在。");
			return std::string(); // 返回空字符串表示失败
		}
	}

	std::string FilePathProvider::GetAppLocalCacheFolderPath()
	{
		try
		{
			// 获取应用的本地缓存文件夹对象
			auto applicationData = winrt::Windows::Storage::ApplicationData::Current();
			if (!applicationData)
			{
				WordWizServices::Log::LogMessage(L"无法获取ApplicationData对象。");
				return std::string();
			}

			winrt::Windows::Storage::StorageFolder localCacheFolder = applicationData.LocalCacheFolder();
			if (!localCacheFolder)
			{
				WordWizServices::Log::LogMessage(L"LocalCacheFolder不可用。");
				return std::string();
			}

			winrt::hstring path_hstring = localCacheFolder.Path();
			if (path_hstring.empty())
			{
				WordWizServices::Log::LogMessage(L"LocalCacheFolder路径为空。");
				return std::string();
			}

			std::wstring wide_path(path_hstring.c_str());
			return WideStringToString_UTF8(wide_path);
		}
		catch (const winrt::hresult_error& e)
		{
			std::wstring errorMsg = L"获取本地缓存文件夹失败，错误代码: 0x" +
				std::to_wstring(static_cast<uint32_t>(e.code())) + L" - " + e.message().c_str();
			WordWizServices::Log::LogMessage(errorMsg);
			return std::string();
		}
		catch (...)
		{
			WordWizServices::Log::LogMessage(L"获取应用本地缓存文件夹路径失败。请检查应用权限或文件夹是否存在。");
			return std::string();
		}
	}

	std::string FilePathProvider::GetAppLocalSettingsPath()
	{
		try
		{
			// 获取应用的本地设置容器
			auto applicationData = winrt::Windows::Storage::ApplicationData::Current();
			if (!applicationData)
			{
				WordWizServices::Log::LogMessage(L"无法获取ApplicationData对象。");
				return std::string();
			}

			winrt::Windows::Storage::ApplicationDataContainer localSettings = applicationData.LocalSettings();
			if (!localSettings)
			{
				WordWizServices::Log::LogMessage(L"LocalSettings不可用。");
				return std::string();
			}

			// 注意：LocalSettings 没有直接的 Path 属性，我们返回一个描述性的路径
			std::string localFolderPath = GetAppLocalFolderPath();
			if (!localFolderPath.empty())
			{
				return localFolderPath + "\\Settings";
			}
			return std::string();
		}
		catch (const winrt::hresult_error& e)
		{
			std::wstring errorMsg = L"获取本地设置路径失败，错误代码: 0x" +
				std::to_wstring(static_cast<uint32_t>(e.code())) + L" - " + e.message().c_str();
			WordWizServices::Log::LogMessage(errorMsg);
			return std::string();
		}
		catch (...)
		{
			WordWizServices::Log::LogMessage(L"获取应用本地设置路径失败。请检查应用权限。");
			return std::string();
		}
	}

	std::string FilePathProvider::GetAppSharedLocalFolderPath()
	{
		try
		{
			// 获取应用的共享本地文件夹对象
			auto applicationData = winrt::Windows::Storage::ApplicationData::Current();
			if (!applicationData)
			{
				WordWizServices::Log::LogMessage(L"无法获取ApplicationData对象。");
				return std::string();
			}

			winrt::Windows::Storage::StorageFolder sharedLocalFolder = applicationData.SharedLocalFolder();
			if (!sharedLocalFolder)
			{
				WordWizServices::Log::LogMessage(L"SharedLocalFolder不可用，可能系统不支持此功能。");
				return std::string();
			}

			winrt::hstring path_hstring = sharedLocalFolder.Path();
			if (path_hstring.empty())
			{
				WordWizServices::Log::LogMessage(L"SharedLocalFolder路径为空。");
				return std::string();
			}

			std::wstring wide_path(path_hstring.c_str());
			return WideStringToString_UTF8(wide_path);
		}
		catch (const winrt::hresult_error& e)
		{
			std::wstring errorMsg = L"获取共享本地文件夹失败，错误代码: 0x" +
				std::to_wstring(static_cast<uint32_t>(e.code())) + L" - " + e.message().c_str();
			WordWizServices::Log::LogMessage(errorMsg);
			return std::string();
		}
		catch (...)
		{
			WordWizServices::Log::LogMessage(L"获取应用共享本地文件夹路径失败。SharedLocalFolder可能不受支持。");
			return std::string();
		}
	}

	std::string FilePathProvider::GetAppTemporaryFolderPath()
	{
		try
		{
			// 获取应用的临时文件夹对象
			auto applicationData = winrt::Windows::Storage::ApplicationData::Current();
			if (!applicationData)
			{
				WordWizServices::Log::LogMessage(L"无法获取ApplicationData对象。");
				return std::string();
			}

			winrt::Windows::Storage::StorageFolder temporaryFolder = applicationData.TemporaryFolder();
			if (!temporaryFolder)
			{
				WordWizServices::Log::LogMessage(L"TemporaryFolder不可用。");
				return std::string();
			}

			winrt::hstring path_hstring = temporaryFolder.Path();
			if (path_hstring.empty())
			{
				WordWizServices::Log::LogMessage(L"TemporaryFolder路径为空。");
				return std::string();
			}

			std::wstring wide_path(path_hstring.c_str());
			return WideStringToString_UTF8(wide_path);
		}
		catch (const winrt::hresult_error& e)
		{
			std::wstring errorMsg = L"获取临时文件夹失败，错误代码: 0x" +
				std::to_wstring(static_cast<uint32_t>(e.code())) + L" - " + e.message().c_str();
			WordWizServices::Log::LogMessage(errorMsg);
			return std::string();
		}
		catch (...)
		{
			WordWizServices::Log::LogMessage(L"获取应用临时文件夹路径失败。请检查应用权限或文件夹是否存在。");
			return std::string();
		}
	}
}
