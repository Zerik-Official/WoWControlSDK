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
        uint64_t    sequence;
    };

    struct LogConfig
    {
        int  maxEntries;
        bool showInConsole;
    };

    namespace LogCapture
    {
        constexpr int DEFAULT_MAX_ENTRIES = 1000;

        void    Initialize();
        void    Shutdown();
        void    SetConfig(int maxEntries, bool showInConsole);
        LogConfig   GetConfig();
        std::vector<LogEntry> GetLogs(int offset, int count);
        int     GetLogCount();
        void    OnConsoleMessage(const char* text, int style);
    }
}
