#include "LogCapture.h"
#include "ConsoleManager.h"
#include "utils/CircularBuffer.h"
#include <Windows.h>
#include <atomic>
#include <cstdio>
#include <mutex>

namespace Runtime
{
    namespace LogCapture
    {
        static constexpr DWORD PRINT_INTERVAL_MS = 100;
        static constexpr DWORD STOP_TIMEOUT_MS = 1000;

        static std::mutex s_mutex;
        static Utils::CircularBuffer<LogEntry> s_buffer(DEFAULT_MAX_ENTRIES);
        static LogConfig s_config = { DEFAULT_MAX_ENTRIES, false };
        static uint64_t s_nextSequence = 0;
        static uint64_t s_printedSequence = 0;
        static std::atomic<bool> s_initialized{false};
        static std::atomic<bool> s_running{false};
        static HANDLE s_printer = nullptr;
        static HANDLE s_wake = nullptr;

        static std::vector<LogEntry> takePending()
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            std::vector<LogEntry> pending;
            if (!s_config.showInConsole || !ConsoleManager::IsVisible())
                return pending;

            for (const auto& entry : s_buffer.getRange(0, 0))
            {
                if (entry.sequence >= s_printedSequence)
                    pending.push_back(entry);
            }
            s_printedSequence = s_nextSequence;
            return pending;
        }

        static DWORD WINAPI printerLoop(LPVOID)
        {
            while (s_running)
            {
                WaitForSingleObject(s_wake, PRINT_INTERVAL_MS);

                std::vector<LogEntry> pending = takePending();
                for (const auto& entry : pending)
                    ConsoleManager::Print(entry.text.c_str());
                if (!pending.empty())
                    fflush(stdout);
            }
            return 0;
        }

        void Initialize()
        {
            if (s_initialized) return;

            s_wake = CreateEventA(nullptr, FALSE, FALSE, nullptr);
            s_running = true;
            s_printer = CreateThread(nullptr, 0, printerLoop, nullptr, 0, nullptr);
            s_initialized = true;
        }

        void Shutdown()
        {
            if (!s_initialized) return;

            s_initialized = false;
            s_running = false;
            if (s_wake) SetEvent(s_wake);
            if (s_printer)
            {
                WaitForSingleObject(s_printer, STOP_TIMEOUT_MS);
                CloseHandle(s_printer);
                s_printer = nullptr;
            }
            if (s_wake)
            {
                CloseHandle(s_wake);
                s_wake = nullptr;
            }

            std::lock_guard<std::mutex> lock(s_mutex);
            s_buffer.clear();
        }

        void SetConfig(int maxEntries, bool showInConsole)
        {
            {
                std::lock_guard<std::mutex> lock(s_mutex);
                s_config.maxEntries = (maxEntries > 0) ? maxEntries : DEFAULT_MAX_ENTRIES;
                s_config.showInConsole = showInConsole;
                s_buffer.setMaxSize(s_config.maxEntries);
            }
            if (s_wake) SetEvent(s_wake);
        }

        LogConfig GetConfig()
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            return s_config;
        }

        void OnConsoleMessage(const char* text, int style)
        {
            if (!s_initialized || !text) return;

            LogEntry entry;
            entry.text = text;
            entry.style = style;
            entry.timestampMs = GetTickCount64();

            {
                std::lock_guard<std::mutex> lock(s_mutex);
                entry.sequence = s_nextSequence++;
                s_buffer.push(std::move(entry));
            }
            if (s_wake) SetEvent(s_wake);
        }

        std::vector<LogEntry> GetLogs(int offset, int count)
        {
            if (!s_initialized) return {};
            std::lock_guard<std::mutex> lock(s_mutex);
            return s_buffer.getRange(offset, count);
        }

        int GetLogCount()
        {
            if (!s_initialized) return 0;
            std::lock_guard<std::mutex> lock(s_mutex);
            return s_buffer.size();
        }
    }
}
