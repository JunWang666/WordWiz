#pragma once
#include <string>

namespace WordWizServices::Log
{
    void LogMessage(const std::wstring& message);
    inline void LogMessage(const std::string& message)
    {
        std::wstring wideMessage(message.begin(), message.end());
        LogMessage(wideMessage);
    }

    inline void LogMessage(const winrt::hstring message)
    {
        LogMessage(std::wstring(message.c_str()));
    }

    inline void LogMessage(const wchar_t* message)
    {
        LogMessage(std::wstring(message));
    }
}