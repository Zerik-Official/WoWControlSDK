#include "Auth.h"
#include <TlHelp32.h>
#include <cstdio>
#include <cwchar>
#include <mutex>

namespace Auth {

namespace {

constexpr size_t MIN_TOKEN_LENGTH = 32;
constexpr size_t MAX_TOKEN_LENGTH = 128;
constexpr const wchar_t* INJECTOR_EXE = L"WowInjector.exe";

std::once_flag   s_once;
CRITICAL_SECTION s_lock;
State            s_state       = State::Pending;
std::string      s_token;
DWORD            s_injectorPid = 0;

DWORD findInjectorParent()
{
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE)
        return 0;

    DWORD self   = GetCurrentProcessId();
    DWORD parent = 0;
    PROCESSENTRY32W entry{};
    entry.dwSize = sizeof(entry);

    for (BOOL ok = Process32FirstW(snapshot, &entry); ok; ok = Process32NextW(snapshot, &entry))
    {
        if (entry.th32ProcessID == self)
        {
            parent = entry.th32ParentProcessID;
            break;
        }
    }

    DWORD injector = 0;
    if (parent)
    {
        entry.dwSize = sizeof(entry);
        for (BOOL ok = Process32FirstW(snapshot, &entry); ok; ok = Process32NextW(snapshot, &entry))
        {
            if (entry.th32ProcessID == parent)
            {
                if (_wcsicmp(entry.szExeFile, INJECTOR_EXE) == 0)
                    injector = parent;
                break;
            }
        }
    }

    CloseHandle(snapshot);
    return injector;
}

bool isValidToken(const std::string& token)
{
    if (token.size() < MIN_TOKEN_LENGTH || token.size() > MAX_TOKEN_LENGTH)
        return false;

    for (char c : token)
    {
        bool hex = (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
        if (!hex)
            return false;
    }
    return true;
}

bool constantTimeEquals(const std::string& a, const std::string& b)
{
    unsigned char diff = static_cast<unsigned char>(a.size() != b.size());
    size_t length = a.size() > b.size() ? a.size() : b.size();
    for (size_t i = 0; i < length; i++)
    {
        unsigned char x = i < a.size() ? static_cast<unsigned char>(a[i]) : 0;
        unsigned char y = i < b.size() ? static_cast<unsigned char>(b[i]) : 0;
        diff |= static_cast<unsigned char>(x ^ y);
    }
    return diff == 0;
}

SDK::Json failure(const char* message)
{
    SDK::Json result;
    result["ok"]    = false;
    result["error"] = message;
    return result;
}

}

void initialize()
{
    std::call_once(s_once, []
    {
        InitializeCriticalSection(&s_lock);
        s_injectorPid = findInjectorParent();

        if (s_injectorPid)
            printf("[Auth] Waiting for the injector (pid %lu) to configure authentication\n", s_injectorPid);
        else
            printf("[Auth] Waiting for the first client to configure authentication\n");
    });
}

State state()
{
    initialize();
    EnterCriticalSection(&s_lock);
    State current = s_state;
    LeaveCriticalSection(&s_lock);
    return current;
}

const char* stateName()
{
    switch (state())
    {
        case State::Enabled:  return "enabled";
        case State::Disabled: return "disabled";
        default:              return "pending";
    }
}

bool verify(const std::string& token)
{
    initialize();
    EnterCriticalSection(&s_lock);
    bool ok = false;
    if (s_state == State::Disabled)
        ok = true;
    else if (s_state == State::Enabled)
        ok = constantTimeEquals(token, s_token);
    LeaveCriticalSection(&s_lock);
    return ok;
}

SDK::Json handleInit(const SDK::Json& params, DWORD clientPid)
{
    bool disable = params.is_object() && params.contains("disabled")
        && params["disabled"].is_boolean() && params["disabled"].get<bool>();

    std::string token;
    if (params.is_object() && params.contains("token") && params["token"].is_string())
        token = params["token"].get<std::string>();

    if (!disable && !isValidToken(token))
        return failure("token must be 32 to 128 hexadecimal characters");

    initialize();
    EnterCriticalSection(&s_lock);

    if (s_state != State::Pending)
    {
        LeaveCriticalSection(&s_lock);
        return failure("authentication is already configured");
    }

    if (s_injectorPid && clientPid != s_injectorPid)
    {
        LeaveCriticalSection(&s_lock);
        return failure("only the injector that started the game can configure authentication");
    }

    s_state = disable ? State::Disabled : State::Enabled;
    s_token = disable ? std::string() : token;
    LeaveCriticalSection(&s_lock);

    printf("[Auth] Authentication %s\n", disable ? "disabled" : "enabled");

    SDK::Json result;
    result["ok"]    = true;
    result["state"] = disable ? "disabled" : "enabled";
    return result;
}

}
