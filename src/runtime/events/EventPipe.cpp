#include "EventPipe.h"
#include "ipc/Auth.h"
#include "ipc/PipeSecurity.h"
#include "utils/json/Json.h"
#include "utils/CircularBuffer.h"
#include <Windows.h>
#include <string>
#include <queue>
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

        static Utils::CircularBuffer<BufferedEvent> s_buffer;
        static std::queue<std::string>              s_writeQueue;

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

        static constexpr DWORD HANDSHAKE_TIMEOUT_MS = 5000;
        static constexpr size_t HANDSHAKE_MAX_BYTES = 512;

        static bool ReadHandshakeLine(HANDLE h, std::string& line)
        {
            ULONGLONG deadline = GetTickCount64() + HANDSHAKE_TIMEOUT_MS;
            while (GetTickCount64() < deadline && line.size() < HANDSHAKE_MAX_BYTES)
            {
                DWORD available = 0;
                if (!PeekNamedPipe(h, nullptr, 0, nullptr, &available, nullptr))
                    return false;

                if (available == 0)
                {
                    Sleep(10);
                    continue;
                }

                char c = 0;
                DWORD read = 0;
                if (!ReadFile(h, &c, 1, &read, nullptr) || read == 0)
                    return false;
                if (c == '\n')
                    return true;
                line.push_back(c);
            }
            return false;
        }

        static void WriteControlEvent(HANDLE h, const char* eventName, const char* error)
        {
            SDK::Json wire;
            wire["event"] = eventName;
            wire["args"]  = SDK::Json::object();
            if (error)
                wire["args"]["error"] = error;
            wire["time"]  = static_cast<int64_t>(GetTickCount64());
            std::string msg = wire.dump() + "\n";
            DWORD written = 0;
            WriteFile(h, msg.c_str(), (DWORD)msg.size(), &written, nullptr);
        }

        static bool Authenticate(HANDLE h)
        {
            if (Auth::state() == Auth::State::Disabled)
                return true;

            std::string line;
            if (!ReadHandshakeLine(h, line))
            {
                WriteControlEvent(h, "auth.failed", "missing handshake");
                return false;
            }

            SDK::Json message = SDK::Json::parse(line, nullptr, false);
            std::string token;
            if (message.is_object() && message.contains("auth") && message["auth"].is_string())
                token = message["auth"].get<std::string>();

            if (!Auth::verify(token))
            {
                WriteControlEvent(h, "auth.failed", "unauthorized");
                return false;
            }

            WriteControlEvent(h, "auth.ok", nullptr);
            return true;
        }

        static DWORD WINAPI PipeAcceptThread(LPVOID)
        {
            while (s_running)
            {
                HANDLE h = CreateNamedPipeA(
                    PIPE_NAME,
                    PIPE_ACCESS_DUPLEX,
                    PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT | PipeSecurity::PIPE_FLAGS,
                    1, 65536, 4096, 0, PipeSecurity::attributes()
                );

                if (h == INVALID_HANDLE_VALUE) { Sleep(500); continue; }

                BOOL connected = ConnectNamedPipe(h, nullptr)
                    ? TRUE
                    : (GetLastError() == ERROR_PIPE_CONNECTED);

                if (!connected) { CloseHandle(h); continue; }

                if (!Authenticate(h))
                {
                    printf("[EventPipe] Client rejected: authentication failed\n");
                    DisconnectNamedPipe(h);
                    CloseHandle(h);
                    continue;
                }

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
            s_buffer.push(std::move(entry));
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
            s_buffer.setMaxSize(max);
            LeaveCriticalSection(&s_lock);
        }

        int GetBufferCount()
        {
            EnterCriticalSection(&s_lock);
            int count = s_buffer.size();
            LeaveCriticalSection(&s_lock);
            return count;
        }

        std::vector<BufferedEvent> GetHistory(int offset, int count)
        {
            EnterCriticalSection(&s_lock);
            auto result = s_buffer.getRange(offset, count);
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
