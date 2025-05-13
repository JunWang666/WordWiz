#include "pch.h"
#include "logger.h"
#include <windows.h>
#include "pch.h"

namespace WordWizServices
{
    void LogMessage(const std::wstring& message)
    {
        // 实际的日志记录实现
        OutputDebugString((message + L"\n").c_str());

        // 或者其他日志记录逻辑...
    }
}