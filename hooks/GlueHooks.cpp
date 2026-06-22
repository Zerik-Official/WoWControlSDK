#include "GlueHooks.h"
#include "core/native/ClientState.h"
#include "utils/json/Json.h"
#include "utils/TaskQueue.h"
#include <deps/Detours/detours.h>
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

static bool s_loginPending = false;
static const char* s_capturedLoginResult = nullptr;

void setLoginPending()
{
    s_loginPending = true;
    s_capturedLoginResult = nullptr;
}

static void __cdecl GruntLoginState_Hook(int param_1, int param_2, const char* param_3, const char* stateStr, const char* resultStr, unsigned char param_6)
{
    if (s_loginPending && stateStr && resultStr)
    {
        if (strcmp(stateStr, "LOGIN_STATE_FAILED") == 0 && !s_capturedLoginResult)
        {
            s_capturedLoginResult = resultStr;
            s_loginPending = false;
        }
    }
    s_gruntOriginal(param_1, param_2, param_3, stateStr, resultStr, param_6);
}

static void GlueMgr_PostUpdate()
{
    if (!s_loginPending) return;

    const char* screen = WoW::GetScreenName();
    if (!screen) return;

    if (strcmp(screen, "charselect") == 0)
    {
        s_loginPending = false;
    }
}

const char* getCapturedLoginResult()
{
    return s_capturedLoginResult;
}

void Initialize()
{
    if (s_initialized) return;

    s_original = (GlueMgrUpdateFn)0x004DAB40;
    s_gruntOriginal = (GruntPrintFn)0x004DA4B0;

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(&(void*&)s_original, GlueMgrUpdate_Hook);
    DetourAttach(&(void*&)s_gruntOriginal, GruntLoginState_Hook);
    DetourTransactionCommit();

    s_initialized = true;
}

void Shutdown()
{
    if (!s_initialized) return;

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourDetach(&(void*&)s_original, GlueMgrUpdate_Hook);
    DetourDetach(&(void*&)s_gruntOriginal, GruntLoginState_Hook);
    DetourTransactionCommit();

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
