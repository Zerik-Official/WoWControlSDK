#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace Runtime
{
    struct LogEntry
    {
        std::string text;
        int         style;
        uint64_t    timestampMs;
    };

    struct LogConfig
    {
        int  maxEntries;
        bool showInConsole;
    };

    namespace LogCapture
    {
        void    Initialize();
        void    Shutdown();
        void    SetConfig(int maxEntries, bool showInConsole);
        LogConfig   GetConfig();
        std::vector<LogEntry> GetLogs(int offset, int count);
        int     GetLogCount();
        void    OnConsoleMessage(const char* text, int style);
    }
}
