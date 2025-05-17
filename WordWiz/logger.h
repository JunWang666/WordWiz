#pragma once
#include <string>

namespace WordWizServices::Log
{
    void LogMessage(const std::wstring& message);
    void LogMessage(const std::string& message);
}