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
           std::clog << message.c_str() << std::endl;
           OutputDebugStringW((message + L"\n").c_str());
       }
}