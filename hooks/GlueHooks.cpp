#include "GlueHooks.h"
#include "base/DetourHelper.h"
#include "core/native/ClientState.h"
#include "offsets/OffsetsLogin.h"
#include "utils/json/Json.h"
#include "utils/TaskQueue.h"
#include <atomic>
#include <cstring>

namespace Hooks::Glue
{

using GlueMgrUpdateFn = void(*)();
static GlueMgrUpdateFn s_original = nullptr;
static bool s_initialized = false;

static Utils::TaskQueue s_queue;

static void GlueMgr_PostUpdate();

static void __cdecl GlueMgrUpdate_Hook()
{
    s_queue.drainAll();

    s_original();
    GlueMgr_PostUpdate();
}

using GruntPrintFn = void(__cdecl*)(int, int, const char*, const char*, const char*, unsigned char);
static GruntPrintFn s_gruntOriginal = nullptr;

static std::atomic<bool> s_loginPending{false};
static std::atomic<bool> s_loginResultReady{false};
static std::atomic<ULONGLONG> s_lastLoginStateTick{0};
static char s_capturedLoginResult[64] = {};

void setLoginPending()
{
    s_loginResultReady.store(false);
    s_capturedLoginResult[0] = '\0';
    s_lastLoginStateTick.store(GetTickCount64());
    s_loginPending.store(true);
}

static void captureLoginResult(const char* resultStr)
{
    strncpy_s(s_capturedLoginResult, sizeof(s_capturedLoginResult), resultStr, _TRUNCATE);
    s_loginResultReady.store(true);
    s_loginPending.store(false);
}

static void __cdecl GruntLoginState_Hook(int param_1, int param_2, const char* param_3, const char* stateStr, const char* resultStr, unsigned char param_6)
{
    if (s_loginPending.load() && stateStr && resultStr)
    {
        s_lastLoginStateTick.store(GetTickCount64());
        if (strcmp(stateStr, "LOGIN_STATE_FAILED") == 0)
            captureLoginResult(resultStr);
        else if (strcmp(stateStr, "LOGIN_STATE_AUTHENTICATED") == 0)
            captureLoginResult("LOGIN_OK");
    }
    s_gruntOriginal(param_1, param_2, param_3, stateStr, resultStr, param_6);
}

static void GlueMgr_PostUpdate()
{
    if (!s_loginPending.load()) return;

    const char* screen = WoW::GetScreenName();
    if (!screen) return;

    if (strcmp(screen, "charselect") == 0)
    {
        s_loginPending.store(false);
    }
}

const char* getCapturedLoginResult()
{
    return s_loginResultReady.load() ? s_capturedLoginResult : nullptr;
}

ULONGLONG getLastLoginStateTick()
{
    return s_lastLoginStateTick.load();
}

void Initialize()
{
    if (s_initialized) return;

    s_original = (GlueMgrUpdateFn)Offsets::Login::GLUE_MGR_UPDATE;
    s_gruntOriginal = (GruntPrintFn)Offsets::Login::GRUNT_PRINTER;

    Detail::attachBatch(
        Detail::DetourEntry{s_original, GlueMgrUpdate_Hook},
        Detail::DetourEntry{s_gruntOriginal, GruntLoginState_Hook}
    );

    s_initialized = true;
}

void Shutdown()
{
    if (!s_initialized) return;

    Detail::detachBatch(
        Detail::DetourEntry{s_original, GlueMgrUpdate_Hook},
        Detail::DetourEntry{s_gruntOriginal, GruntLoginState_Hook}
    );

    s_initialized = false;
}

void Post(std::function<void()> task)
{
    s_queue.post(std::move(task));
}

std::string Execute(std::function<std::string()> task, DWORD timeoutMs)
{
    return s_queue.execute(std::move(task), timeoutMs);
}

}
