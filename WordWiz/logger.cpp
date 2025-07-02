#include "pch.h"
#include "logger.h"
#include <windows.h>
#include <iostream>
#include <string>


namespace WordWizServices::Log
{
	// 主实现，所有重载都转发到这里
	void LogMessage(const std::wstring& message)
	{
#ifdef _DEBUG
		std::wstring truncatedMessage;
		if (message.length() > MAX_LOG_LENGTH)
		{
			 truncatedMessage = message.substr(0, MAX_LOG_LENGTH) + L"... [truncated]";
		}
		else {
			 truncatedMessage = message;
		}
		std::clog << truncatedMessage.c_str() << std::endl;
		OutputDebugStringW((truncatedMessage + L"\n").c_str());
#endif // DEBUG
	}
}
