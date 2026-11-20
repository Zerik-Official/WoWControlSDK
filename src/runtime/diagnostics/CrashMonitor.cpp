#include "runtime/diagnostics/CrashMonitor.h"
#include "core/native/CrashReport.h"
#include "runtime/events/EventPipe.h"
#include <cstdio>
#include <mutex>

namespace Runtime::CrashMonitor
{
    namespace
    {
        constexpr DWORD MAX_LOG_BYTES = 65536;

        std::mutex s_mutex;
        SDK::Json  s_last;
        bool       s_hasLast = false;

        std::string hex(uintptr_t value)
        {
            char buffer[16];
            std::snprintf(buffer, sizeof(buffer), "0x%08X", static_cast<unsigned>(value));
            return buffer;
        }

        std::string fileName(const std::string& path)
        {
            size_t slash = path.find_last_of("\\/");
            return slash == std::string::npos ? path : path.substr(slash + 1);
        }

        SDK::Json exceptionJson(const EXCEPTION_POINTERS* pointers)
        {
            SDK::Json j;
            const EXCEPTION_RECORD* record = pointers->ExceptionRecord;
            if (!record)
                return j;

            uintptr_t address = reinterpret_cast<uintptr_t>(record->ExceptionAddress);
            j["code"] = hex(record->ExceptionCode);
            j["address"] = hex(address);

            HMODULE module = nullptr;
            if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                    reinterpret_cast<LPCSTR>(record->ExceptionAddress), &module) && module)
            {
                char path[MAX_PATH] = {};
                GetModuleFileNameA(module, path, MAX_PATH);
                j["module"] = fileName(path);
                j["offset"] = hex(address - reinterpret_cast<uintptr_t>(module));
            }
            return j;
        }

        bool readLog(const std::string& path, std::string& text, bool& truncated)
        {
            HANDLE file = CreateFileA(path.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
                OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
            if (file == INVALID_HANDLE_VALUE)
                return false;

            LARGE_INTEGER size = {};
            GetFileSizeEx(file, &size);
            DWORD toRead = size.QuadPart > MAX_LOG_BYTES ? MAX_LOG_BYTES : static_cast<DWORD>(size.QuadPart);
            truncated = size.QuadPart > MAX_LOG_BYTES;

            text.resize(toRead);
            DWORD read = 0;
            bool ok = toRead == 0 || ReadFile(file, text.data(), toRead, &read, nullptr);
            text.resize(read);
            CloseHandle(file);
            return ok;
        }

        SDK::Json sanitize(const SDK::Json& data)
        {
            return SDK::Json::parse(data.dump(-1, ' ', false, SDK::Json::error_handler_t::replace));
        }
    }

    void Report(const FatalError& error)
    {
        const EXCEPTION_POINTERS* pointers = WoW::CrashReport::GetExceptionPointers();

        SDK::Json data;
        data["kind"] = pointers ? "crash" : "error";
        data["errorNumber"] = static_cast<int>(error.code & 0xFFFF);
        data["code"] = hex(error.code);
        data["title"] = error.title;
        data["description"] = error.description;
        data["details"] = error.details;
        data["exception"] = pointers ? exceptionJson(pointers) : SDK::Json();

        std::string logPath = WoW::CrashReport::GetLogPath();
        data["logPath"] = logPath.empty() ? SDK::Json() : SDK::Json(logPath);

        std::string log;
        bool truncated = false;
        if (!logPath.empty() && readLog(logPath, log, truncated))
        {
            data["log"] = log;
            if (truncated)
                data["logTruncated"] = true;
        }
        else
        {
            data["log"] = nullptr;
        }

        uint64_t timeMs = static_cast<uint64_t>(GetTickCount64());
        data["time"] = timeMs;

        SDK::Json safe = sanitize(data);
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            s_last = safe;
            s_hasLast = true;
        }

        EventPipe::PushEvent("client.crashed", safe, timeMs);
    }

    bool GetLast(SDK::Json& out)
    {
        std::lock_guard<std::mutex> lock(s_mutex);
        if (!s_hasLast)
            return false;
        out = s_last;
        return true;
    }
}
