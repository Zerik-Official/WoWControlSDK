#include "EventPipe.h"
#include "utils/json/Json.h"
#include <Windows.h>
#include <string>
#include <vector>
#include <queue>
#include <algorithm>
#include <cstdio>

namespace Runtime
{
    namespace EventPipe
    {
        static HANDLE           s_pipe         = INVALID_HANDLE_VALUE;
        static HANDLE           s_pipeThread   = nullptr;
        static HANDLE           s_writeThread  = nullptr;
        static HANDLE           s_writeEvent   = nullptr;
        static HANDLE           s_disconnEvent = nullptr;
        static CRITICAL_SECTION s_lock;
        static CRITICAL_SECTION s_writeLock;
        static bool             s_lockInit     = false;
        static bool             s_initialized  = false;
        static bool             s_running      = false;

        static std::vector<BufferedEvent> s_buffer;
        static std::queue<std::string>    s_writeQueue;
        static int                        s_maxBuffer = 256;

        static const char* PIPE_NAME = "\\\\.\\pipe\\WowGameEvent";

        static DWORD WINAPI WriteThread(LPVOID)
        {
            while (s_running)
            {
                WaitForSingleObject(s_writeEvent, 100);

                while (true)
                {
                    std::string msg;
                    EnterCriticalSection(&s_writeLock);
                    if (!s_writeQueue.empty())
                    {
                        msg = std::move(s_writeQueue.front());
                        s_writeQueue.pop();
                    }
                    LeaveCriticalSection(&s_writeLock);

                    if (msg.empty()) break;

                    EnterCriticalSection(&s_lock);
                    HANDLE pipe = s_pipe;
                    LeaveCriticalSection(&s_lock);

                    if (pipe == INVALID_HANDLE_VALUE) continue;

                    DWORD written = 0;
                    BOOL ok = WriteFile(pipe, msg.c_str(), (DWORD)msg.size(), &written, nullptr);
                    if (!ok)
                    {
                        SetEvent(s_disconnEvent);

                        EnterCriticalSection(&s_lock);
                        if (s_pipe == pipe)
                        {
                            DisconnectNamedPipe(s_pipe);
                            CloseHandle(s_pipe);
                            s_pipe = INVALID_HANDLE_VALUE;
                        }
                        LeaveCriticalSection(&s_lock);
                    }
                }
            }
            return 0;
        }

        static DWORD WINAPI PipeAcceptThread(LPVOID)
        {
            while (s_running)
            {
                HANDLE h = CreateNamedPipeA(
                    PIPE_NAME,
                    PIPE_ACCESS_OUTBOUND,
                    PIPE_TYPE_BYTE | PIPE_WAIT,
                    1, 65536, 0, 0, nullptr
                );

                if (h == INVALID_HANDLE_VALUE) { Sleep(500); continue; }

                BOOL connected = ConnectNamedPipe(h, nullptr)
                    ? TRUE
                    : (GetLastError() == ERROR_PIPE_CONNECTED);

                if (!connected) { CloseHandle(h); continue; }

                printf("[EventPipe] Client connected\n");
                ResetEvent(s_disconnEvent);

                EnterCriticalSection(&s_lock);
                if (s_pipe != INVALID_HANDLE_VALUE) CloseHandle(s_pipe);
                s_pipe = h;
                LeaveCriticalSection(&s_lock);

                WaitForSingleObject(s_disconnEvent, INFINITE);

                printf("[EventPipe] Client disconnected\n");

                EnterCriticalSection(&s_lock);
                if (s_pipe == h)
                {
                    DisconnectNamedPipe(s_pipe);
                    CloseHandle(s_pipe);
                    s_pipe = INVALID_HANDLE_VALUE;
                }
                LeaveCriticalSection(&s_lock);
            }
            return 0;
        }

        void Initialize()
        {
            if (s_initialized) return;

            if (!s_lockInit)
            {
                InitializeCriticalSection(&s_lock);
                InitializeCriticalSection(&s_writeLock);
                s_lockInit = true;
            }

            s_writeEvent   = CreateEventA(nullptr, FALSE, FALSE, nullptr);
            s_disconnEvent = CreateEventA(nullptr, TRUE,  FALSE, nullptr);
            s_running      = true;
            s_pipeThread   = CreateThread(nullptr, 0, PipeAcceptThread, nullptr, 0, nullptr);
            s_writeThread  = CreateThread(nullptr, 0, WriteThread,      nullptr, 0, nullptr);
            s_initialized  = true;
        }

        void Shutdown()
        {
            if (!s_initialized) return;

            s_running = false;
            SetEvent(s_writeEvent);
            SetEvent(s_disconnEvent);

            EnterCriticalSection(&s_lock);
            if (s_pipe != INVALID_HANDLE_VALUE)
            {
                DisconnectNamedPipe(s_pipe);
                CloseHandle(s_pipe);
                s_pipe = INVALID_HANDLE_VALUE;
            }
            LeaveCriticalSection(&s_lock);

            if (s_pipeThread)   { WaitForSingleObject(s_pipeThread,  2000); CloseHandle(s_pipeThread);  s_pipeThread  = nullptr; }
            if (s_writeThread)  { WaitForSingleObject(s_writeThread, 2000); CloseHandle(s_writeThread); s_writeThread = nullptr; }
            if (s_writeEvent)   { CloseHandle(s_writeEvent);   s_writeEvent   = nullptr; }
            if (s_disconnEvent) { CloseHandle(s_disconnEvent); s_disconnEvent = nullptr; }

            EnterCriticalSection(&s_lock);
            s_buffer.clear();
            std::vector<BufferedEvent>().swap(s_buffer);
            LeaveCriticalSection(&s_lock);

            EnterCriticalSection(&s_writeLock);
            std::queue<std::string>().swap(s_writeQueue);
            LeaveCriticalSection(&s_writeLock);

            s_initialized = false;
        }

        bool PushEvent(const char* eventName, const SDK::Json& args, uint64_t timeMs)
        {
            if (!s_initialized || !eventName) return false;

            SDK::Json wire;
            wire["event"] = eventName;
            wire["args"]  = args;
            wire["time"]  = static_cast<int64_t>(timeMs);
            std::string msg = wire.dump() + "\n";

            BufferedEvent entry;
            entry.eventName   = eventName;
            entry.args        = args;
            entry.timestampMs = timeMs;

            EnterCriticalSection(&s_lock);
            s_buffer.push_back(std::move(entry));
            while ((int)s_buffer.size() > s_maxBuffer)
                s_buffer.erase(s_buffer.begin());
            bool hasPipe = (s_pipe != INVALID_HANDLE_VALUE);
            LeaveCriticalSection(&s_lock);

            if (hasPipe)
            {
                EnterCriticalSection(&s_writeLock);
                s_writeQueue.push(msg);
                LeaveCriticalSection(&s_writeLock);
                SetEvent(s_writeEvent);
            }

            return true;
        }

        void SetMaxBuffer(int max)
        {
            EnterCriticalSection(&s_lock);
            s_maxBuffer = (max > 0) ? max : 256;
            s_buffer.reserve(s_maxBuffer);
            while ((int)s_buffer.size() > s_maxBuffer)
                s_buffer.erase(s_buffer.begin());
            LeaveCriticalSection(&s_lock);
        }

        int GetBufferCount()
        {
            EnterCriticalSection(&s_lock);
            int count = (int)s_buffer.size();
            LeaveCriticalSection(&s_lock);
            return count;
        }

        std::vector<BufferedEvent> GetHistory(int offset, int count)
        {
            std::vector<BufferedEvent> result;
            EnterCriticalSection(&s_lock);
            if (!s_buffer.empty())
            {
                int total = (int)s_buffer.size();
                if (offset < 0) offset = 0;
                if (offset < total)
                {
                    if (count <= 0) count = total - offset;
                    count = (std::min)(count, total - offset);
                    result.reserve(count);
                    for (int i = 0; i < count; i++)
                        result.push_back(s_buffer[offset + i]);
                }
            }
            LeaveCriticalSection(&s_lock);
            return result;
        }

        void ClearHistory()
        {
            EnterCriticalSection(&s_lock);
            s_buffer.clear();
            LeaveCriticalSection(&s_lock);
        }
    }
}
