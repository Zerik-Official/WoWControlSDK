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

using LoginMgrFireStateChangeFn = void(__thiscall*)(void* this_ptr, int state, int result);
static LoginMgrFireStateChangeFn s_loginMgrOriginal = nullptr;

static int s_lastState = 0;
static int s_lastResult = 0;

static void __fastcall LoginMgrFireStateChange_Hook(void* this_ptr, int /*edx*/, int state, int result)
{
    s_lastState = state;
    s_lastResult = result;
    s_loginMgrOriginal(this_ptr, state, result);
}

int getLastLoginState()
{
    return s_lastState;
}

int getLastLoginResult()
{
    return s_lastResult;
}

const char* getLastLoginResultStr()
{
    switch (s_lastResult)
    {
    case 0x00: return "LOGIN_OK";
    case 0x0B: return "LOGIN_FAILED";
    case 0x0D: return "LOGIN_BANNED";
    case 0x0E: return "LOGIN_BADVERSION";
    case 0x0F: return "LOGIN_ALREADYONLINE";
    case 0x10: return "LOGIN_NOTIME";
    case 0x11: return "LOGIN_DBBUSY";
    case 0x12: return "LOGIN_SUSPENDED";
    case 0x13: return "LOGIN_PARENTALCONTROL";
    case 0x14: return "LOGIN_LOCKED_ENFORCED";
    case 0x15: return "LOGIN_DISCONNECTED";
    case 0x16: return "LOGIN_ACCOUNT_CONVERTED";
    case 0x19: return "LOGIN_TRIAL_EXPIRED";
    case 0x1C: return "LOGIN_GAME_ACCOUNT_LOCKED";
    case 0x22: return "LOGIN_CHARGEBACK";
    case 0x23: return "LOGIN_IGR_WITHOUT_BNET";
    case 0x24: return "LOGIN_UNLOCKABLE_LOCK";
    case 0x25: return "LOGIN_CONVERSION_REQUIRED";
    default: return "LOGIN_UNKNOWN";
    }
}

using HandleAuthChallengeFn = void(__thiscall*)(void* this_ptr, int param_2, void* param_3, size_t param_4, int param_5);
static HandleAuthChallengeFn s_authChallengeOriginal = nullptr;

static bool s_loginPending = false;
static int s_capturedAuthCode = -1;

void setLoginPending()
{
    s_loginPending = true;
    s_capturedAuthCode = -1;
}

static void __fastcall HandleAuthChallenge_Hook(void* this_ptr, int /*edx*/, int param_2, void* param_3, size_t param_4, int param_5)
{
    if (s_loginPending)
    {
        s_capturedAuthCode = param_2;
        s_loginPending = false;
    }
    s_authChallengeOriginal(this_ptr, param_2, param_3, param_4, param_5);
}

static void GlueMgr_PostUpdate()
{
    if (!s_loginPending) return;

    const char* screen = WoW::GetScreenName();
    if (!screen) return;

    if (strcmp(screen, "charselect") == 0)
    {
        s_capturedAuthCode = 0;
        s_loginPending = false;
    }
}

bool tryGetCapturedAuthCode(int& outCode)
{
    if (s_capturedAuthCode < 0) return false;
    outCode = s_capturedAuthCode;
    return true;
}

void Initialize()
{
    if (s_initialized) return;

    s_original = (GlueMgrUpdateFn)0x004DAB40;
    s_loginMgrOriginal = (LoginMgrFireStateChangeFn)0x00465480;
    s_authChallengeOriginal = (HandleAuthChallengeFn)0x008CB160;

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(&(void*&)s_original, GlueMgrUpdate_Hook);
    DetourAttach(&(void*&)s_loginMgrOriginal, LoginMgrFireStateChange_Hook);
    DetourAttach(&(void*&)s_authChallengeOriginal, HandleAuthChallenge_Hook);
    DetourTransactionCommit();

    s_initialized = true;
}

void Shutdown()
{
    if (!s_initialized) return;

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourDetach(&(void*&)s_original, GlueMgrUpdate_Hook);
    DetourDetach(&(void*&)s_loginMgrOriginal, LoginMgrFireStateChange_Hook);
    DetourDetach(&(void*&)s_authChallengeOriginal, HandleAuthChallenge_Hook);
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
