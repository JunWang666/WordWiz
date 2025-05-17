#include "pch.h"
#include "logger.h"
#include <windows.h>
#include <iostream>
#include <string>

namespace WordWizServices::Log
{
   void LogMessage(const std::wstring& message)
   {
       std::clog << message.c_str() << std::endl;
       OutputDebugStringW((message + L"\n").c_str());
   }

   void LogMessage(const std::string& message)
   {
       std::clog << message << std::endl;

       std::wstring wideMessage(message.begin(), message.end());
       OutputDebugStringW((wideMessage + L"\n").c_str());
   }
}