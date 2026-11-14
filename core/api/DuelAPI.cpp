#include "core/api/DuelAPI.h"
#include "core/native/FrameScript.h"
#include "core/native/NameCache.h"
#include "core/native/ObjectManager.h"
#include "memory/MemReader.h"
#include "offsets/OffsetsSocial.h"
#include "runtime/events/GameEventBridge.h"
#include <Windows.h>

namespace
{
    struct DuelState
    {
        bool        pending;
        bool        active;
        std::string challengerName;
        uint64_t    requestedAtMs;
    };

    SRWLOCK   s_lock  = SRWLOCK_INIT;
    DuelState s_state = { false, false, {}, 0 };

    void callVoid(uintptr_t address)
    {
        using VoidFn = void(__cdecl*)();
        reinterpret_cast<VoidFn>(address)();
    }

    void hideDuelPopup()
    {
        Runtime::GameEventBridge::Suppress("DUEL_FINISHED");
        if (!FrameScript::FireEvent("DUEL_FINISHED"))
            Runtime::GameEventBridge::Unsuppress("DUEL_FINISHED");
    }
}

namespace CoreAPI
{
    namespace Duel
    {
        DuelInfo Get()
        {
            AcquireSRWLockShared(&s_lock);
            DuelState state = s_state;
            ReleaseSRWLockShared(&s_lock);

            DuelInfo info = {};
            info.pending        = state.pending;
            info.active         = state.active;
            info.requestedAtMs  = state.requestedAtMs;
            info.challengerGuid = Memory::safeRead<WoWGUID>(Offsets::Duel::CHALLENGER_GUID);
            info.arbiterGuid    = Memory::safeRead<WoWGUID>(Offsets::Duel::ARBITER_GUID);
            info.challengerName = state.challengerName;
            if (info.challengerName.empty() && info.challengerGuid.isValid())
                info.challengerName = NameCache::lookup(info.challengerGuid).name;
            return info;
        }

        void SetRequested(const std::string& challengerName, uint64_t requestedAtMs)
        {
            AcquireSRWLockExclusive(&s_lock);
            s_state = DuelState{ true, false, challengerName, requestedAtMs };
            ReleaseSRWLockExclusive(&s_lock);
        }

        void SetAnswered(bool accepted)
        {
            AcquireSRWLockExclusive(&s_lock);
            s_state.pending = false;
            s_state.active  = accepted;
            ReleaseSRWLockExclusive(&s_lock);
        }

        void SetFinished()
        {
            AcquireSRWLockExclusive(&s_lock);
            s_state = DuelState{ false, false, {}, 0 };
            ReleaseSRWLockExclusive(&s_lock);
        }

        DuelResult Accept()
        {
            if (!WoW::GetLocalGUID().isValid()) return DuelResult::NotInWorld;
            if (!Get().pending) return DuelResult::NoDuel;

            callVoid(Offsets::Duel::ACCEPT);
            hideDuelPopup();
            return DuelResult::Ok;
        }

        DuelResult Decline()
        {
            if (!WoW::GetLocalGUID().isValid()) return DuelResult::NotInWorld;
            DuelInfo info = Get();
            if (!info.pending && !info.active) return DuelResult::NoDuel;

            callVoid(Offsets::Duel::CANCEL);
            if (info.pending)
                hideDuelPopup();
            return DuelResult::Ok;
        }
    }
}
