#include "LogCapture.h"
#include "ConsoleManager.h"
#include <Windows.h>
#include <vector>
#include <algorithm>
#include <cstring>

namespace Runtime
{
    namespace LogCapture
    {
        static std::vector<LogEntry> s_buffer;
        static LogConfig s_config = { 256, false };
        static bool s_initialized = false;

        void Initialize()
        {
            if (s_initialized) return;
            s_buffer.reserve(s_config.maxEntries);
            s_initialized = true;
        }

        void Shutdown()
        {
            s_initialized = false;
            s_buffer.clear();
            std::vector<LogEntry>().swap(s_buffer);
        }

        void SetConfig(int maxEntries, bool showInConsole)
        {
            s_config.maxEntries = (maxEntries > 0) ? maxEntries : 256;
            s_config.showInConsole = showInConsole;
            if ((int)s_buffer.capacity() != s_config.maxEntries)
            {
                s_buffer.reserve(s_config.maxEntries);
            }
        }

        LogConfig GetConfig()
        {
            return s_config;
        }

        void OnConsoleMessage(const char* text, int style)
        {
            if (!s_initialized || !text) return;

            LogEntry entry;
            entry.text = text;
            entry.style = style;
            entry.timestampMs = GetTickCount64();

            s_buffer.push_back(entry);

            if ((int)s_buffer.size() > s_config.maxEntries)
                s_buffer.erase(s_buffer.begin());

            if (s_config.showInConsole)
                ConsoleManager::Print(text);
        }

        std::vector<LogEntry> GetLogs(int offset, int count)
        {
            std::vector<LogEntry> result;
            if (!s_initialized || s_buffer.empty()) return result;

            int total = (int)s_buffer.size();
            if (offset < 0) offset = 0;
            if (offset >= total) return result;
            if (count <= 0) count = total - offset;
            count = (std::min)(count, total - offset);

            result.reserve(count);
            for (int i = 0; i < count; i++)
                result.push_back(s_buffer[offset + i]);

            return result;
        }

        int GetLogCount()
        {
            return s_initialized ? (int)s_buffer.size() : 0;
        }
    }
}
