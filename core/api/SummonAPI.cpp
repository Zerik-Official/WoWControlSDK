#include "core/api/SummonAPI.h"
#include "core/native/ClientDb.h"
#include "core/native/NameCache.h"
#include "core/native/ObjectManager.h"
#include "core/native/FrameScript.h"
#include "core/native/GameClock.h"
#include "memory/MemReader.h"
#include "offsets/OffsetsSocial.h"
#include "runtime/events/GameEventBridge.h"

namespace
{
    constexpr uintptr_t UNIT_DESCRIPTORS   = 0xD0;
    constexpr uintptr_t UNIT_FIELD_FLAGS   = 0xD4;
    constexpr uintptr_t VTABLE_IS_LOCKED   = 0x128;
    constexpr uint32_t  TELEPORT_BLOCKERS  = 0x00040000 | 0x00080000 | 0x00400000 | 0x00800000;

    uintptr_t localPlayer()
    {
        WoWGUID guid = WoW::GetLocalGUID();
        if (!guid.isValid()) return 0;
        return WoW::GetObjectByGUID(guid);
    }

    void callScriptFunction(uintptr_t address)
    {
        using ScriptFn = int(__cdecl*)(void*);
        reinterpret_cast<ScriptFn>(address)(nullptr);
    }
}

namespace CoreAPI
{
    namespace Summon
    {
        SummonInfo Get()
        {
            namespace S = Offsets::Summon;

            SummonInfo info = {};
            int expiresAt = Memory::safeRead<int>(S::EXPIRES_AT_MS);
            if (expiresAt != 0)
            {
                int left = expiresAt - GameClock::nowMs();
                info.timeLeftMs = left > 0 ? left : 0;
            }

            info.summonerGuid = Memory::safeRead<WoWGUID>(S::SUMMONER_GUID);
            info.areaId       = Memory::safeRead<int>(S::AREA_ID);
            info.pending      = info.timeLeftMs > 0 && info.summonerGuid.isValid();

            if (info.summonerGuid.isValid())
                info.summonerName = NameCache::lookup(info.summonerGuid).name;
            info.areaName = ClientDb::areaName(info.areaId);
            return info;
        }

        bool CanTeleport()
        {
            uintptr_t player = localPlayer();
            if (!player) return false;

            uintptr_t vtable = Memory::safeRead<uintptr_t>(player);
            uintptr_t isLockedAddr = Memory::safeRead<uintptr_t>(vtable + VTABLE_IS_LOCKED);
            if (!isLockedAddr) return false;

            using IsLockedFn = char(__thiscall*)(uintptr_t);
            if (reinterpret_cast<IsLockedFn>(isLockedAddr)(player) != 0)
                return false;

            uintptr_t descriptors = Memory::safeRead<uintptr_t>(player + UNIT_DESCRIPTORS);
            uint32_t flags = Memory::safeRead<uint32_t>(descriptors + UNIT_FIELD_FLAGS);
            return (flags & TELEPORT_BLOCKERS) == 0;
        }

        SummonResult Accept()
        {
            if (!localPlayer()) return SummonResult::NotInWorld;
            if (!Get().pending) return SummonResult::NoSummon;
            if (!CanTeleport()) return SummonResult::CannotTeleport;

            callScriptFunction(Offsets::Summon::CONFIRM);
            Runtime::GameEventBridge::OnSummonAnswered(true);
            FrameScript::FireEvent("CANCEL_SUMMON");
            return SummonResult::Ok;
        }

        SummonResult Decline()
        {
            if (!localPlayer()) return SummonResult::NotInWorld;
            if (!Get().pending) return SummonResult::NoSummon;

            callScriptFunction(Offsets::Summon::CANCEL);
            Runtime::GameEventBridge::OnSummonAnswered(false);
            FrameScript::FireEvent("CANCEL_SUMMON");
            return SummonResult::Ok;
        }
    }
}
