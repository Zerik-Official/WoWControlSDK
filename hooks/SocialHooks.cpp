#include <Windows.h>
#include "base/DetourHelper.h"
#include "SocialHooks.h"
#include "core/api/SummonAPI.h"
#include "offsets/OffsetsGroup.h"
#include "offsets/OffsetsSocial.h"
#include "runtime/events/GameEventBridge.h"
#include <cstdint>

namespace Hooks::Social
{

static void(__thiscall* AcceptInvite_orig)(uintptr_t, uint32_t) = (decltype(AcceptInvite_orig))Offsets::Group::ACCEPT_INVITE;
static void(__thiscall* DeclineInvite_orig)(uintptr_t)          = (decltype(DeclineInvite_orig))Offsets::Group::DECLINE_INVITE;
static int(__cdecl* ConfirmSummon_orig)(void*)                  = (decltype(ConfirmSummon_orig))Offsets::Summon::CONFIRM;
static int(__cdecl* CancelSummon_orig)(void*)                   = (decltype(CancelSummon_orig))Offsets::Summon::CANCEL;

static void(__cdecl* BeginTrade_orig)()                         = (decltype(BeginTrade_orig))Offsets::Trade::BEGIN;
static void(__cdecl* CancelTrade_orig)()                        = (decltype(CancelTrade_orig))Offsets::Trade::CANCEL;
static void(__cdecl* AcceptDuel_orig)()                         = (decltype(AcceptDuel_orig))Offsets::Duel::ACCEPT;
static void(__cdecl* CancelDuel_orig)()                         = (decltype(CancelDuel_orig))Offsets::Duel::CANCEL;

static bool s_initialized = false;

static void __fastcall AcceptInvite_hook(uintptr_t self, void*, uint32_t roles)
{
    AcceptInvite_orig(self, roles);
    Runtime::GameEventBridge::OnInviteAnswered(true, roles);
}

static void __fastcall DeclineInvite_hook(uintptr_t self, void*)
{
    DeclineInvite_orig(self);
    Runtime::GameEventBridge::OnInviteAnswered(false, 0);
}

static int __cdecl ConfirmSummon_hook(void* state)
{
    bool canTeleport = CoreAPI::Summon::CanTeleport();
    int result = ConfirmSummon_orig(state);
    if (canTeleport)
        Runtime::GameEventBridge::OnSummonAnswered(true);
    return result;
}

static int __cdecl CancelSummon_hook(void* state)
{
    int result = CancelSummon_orig(state);
    Runtime::GameEventBridge::OnSummonAnswered(false);
    return result;
}

static void __cdecl BeginTrade_hook()
{
    BeginTrade_orig();
    Runtime::GameEventBridge::OnTradeRequestAnswered(true);
}

static void __cdecl CancelTrade_hook()
{
    CancelTrade_orig();
    Runtime::GameEventBridge::OnTradeRequestAnswered(false);
}

static void __cdecl AcceptDuel_hook()
{
    AcceptDuel_orig();
    Runtime::GameEventBridge::OnDuelAnswered(true);
}

static void __cdecl CancelDuel_hook()
{
    CancelDuel_orig();
    Runtime::GameEventBridge::OnDuelAnswered(false);
}

void Initialize()
{
    if (s_initialized) return;

    Detail::attachBatch(
        Detail::DetourEntry{AcceptInvite_orig, (decltype(AcceptInvite_orig))AcceptInvite_hook},
        Detail::DetourEntry{DeclineInvite_orig, (decltype(DeclineInvite_orig))DeclineInvite_hook},
        Detail::DetourEntry{ConfirmSummon_orig, ConfirmSummon_hook},
        Detail::DetourEntry{CancelSummon_orig, CancelSummon_hook},
        Detail::DetourEntry{BeginTrade_orig, BeginTrade_hook},
        Detail::DetourEntry{CancelTrade_orig, CancelTrade_hook},
        Detail::DetourEntry{AcceptDuel_orig, AcceptDuel_hook},
        Detail::DetourEntry{CancelDuel_orig, CancelDuel_hook}
    );

    s_initialized = true;
}

void Shutdown()
{
    if (!s_initialized) return;

    Detail::detachBatch(
        Detail::DetourEntry{AcceptInvite_orig, (decltype(AcceptInvite_orig))AcceptInvite_hook},
        Detail::DetourEntry{DeclineInvite_orig, (decltype(DeclineInvite_orig))DeclineInvite_hook},
        Detail::DetourEntry{ConfirmSummon_orig, ConfirmSummon_hook},
        Detail::DetourEntry{CancelSummon_orig, CancelSummon_hook},
        Detail::DetourEntry{BeginTrade_orig, BeginTrade_hook},
        Detail::DetourEntry{CancelTrade_orig, CancelTrade_hook},
        Detail::DetourEntry{AcceptDuel_orig, AcceptDuel_hook},
        Detail::DetourEntry{CancelDuel_orig, CancelDuel_hook}
    );

    s_initialized = false;
}

}
