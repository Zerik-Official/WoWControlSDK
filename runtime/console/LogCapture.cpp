#include "LogCapture.h"
#include "ConsoleManager.h"
#include "utils/CircularBuffer.h"
#include <Windows.h>
#include <cstring>

namespace Runtime
{
    namespace LogCapture
    {
        static Utils::CircularBuffer<LogEntry> s_buffer;
        static LogConfig s_config = { 256, false };
        static bool s_initialized = false;

        void Initialize()
        {
            if (s_initialized) return;
            s_initialized = true;
        }

        void Shutdown()
        {
            s_initialized = false;
            s_buffer.clear();
        }

        void SetConfig(int maxEntries, bool showInConsole)
        {
            s_config.maxEntries = (maxEntries > 0) ? maxEntries : 256;
            s_config.showInConsole = showInConsole;
            s_buffer.setMaxSize(s_config.maxEntries);
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

            s_buffer.push(std::move(entry));

            if (s_config.showInConsole)
                ConsoleManager::Print(text);
        }

        std::vector<LogEntry> GetLogs(int offset, int count)
        {
            if (!s_initialized) return {};
            return s_buffer.getRange(offset, count);
        }

        int GetLogCount()
        {
            return s_initialized ? s_buffer.size() : 0;
        }
    }
}
