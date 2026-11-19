#include "GlueHooks.h"
#include "base/GameScheduler.h"
#include "base/VTableHook.h"
#include "core/native/ClientState.h"
#include "offsets/OffsetsLogin.h"
#include "utils/json/Json.h"
#include "utils/TaskQueue.h"
#include <cstring>

namespace Hooks::Glue
{

static constexpr float GLUE_PRIORITY = 1.0f;

static bool s_initialized = false;

static Utils::TaskQueue s_queue;

using LoginStateFn = void(__thiscall*)(void*, int, int, const char*, const char*, const char*, unsigned char);
static Detail::VTableHook s_loginStatus;

static bool s_loginPending = false;
static const char* s_capturedLoginResult = nullptr;

void setLoginPending()
{
    s_loginPending = true;
    s_capturedLoginResult = nullptr;
}

static void __fastcall LoginState_Hook(void* self, void*, int param_1, int param_2, const char* param_3, const char* stateStr, const char* resultStr, unsigned char param_6)
{
    if (s_loginPending && stateStr && resultStr)
    {
        if (strcmp(stateStr, "LOGIN_STATE_FAILED") == 0 && !s_capturedLoginResult)
        {
            s_capturedLoginResult = resultStr;
            s_loginPending = false;
        }
    }
    auto original = reinterpret_cast<LoginStateFn>(s_loginStatus.original(Offsets::Login::LOGIN_STATUS_ON_STATE_SLOT));
    if (original)
        original(self, param_1, param_2, param_3, stateStr, resultStr, param_6);
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

static void AttachLoginStatus()
{
    void* object = *reinterpret_cast<void**>(Offsets::Login::LOGIN_STATUS_OBJECT);
    if (!object)
        return;
    s_loginStatus.attach(object, reinterpret_cast<const void*>(Offsets::Login::LOGIN_STATUS_VTABLE),
                         Offsets::Login::LOGIN_STATUS_VTABLE_SLOTS);
}

static void OnIdle()
{
    AttachLoginStatus();
    s_queue.drainAll();
    GlueMgr_PostUpdate();
}

void Initialize()
{
    if (s_initialized) return;

    s_loginStatus.replace(Offsets::Login::LOGIN_STATUS_ON_STATE_SLOT, reinterpret_cast<void*>(LoginState_Hook));
    GameScheduler::Initialize();
    GameScheduler::addIdleHandler(OnIdle, GLUE_PRIORITY);

    s_initialized = true;
}

void Shutdown()
{
    if (!s_initialized) return;

    s_loginStatus.detach();

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
