#include "core/api/BattlefieldAPI.h"
#include "core/native/ClientDb.h"
#include "core/native/ClientPacket.h"
#include "core/native/FrameScript.h"
#include "core/native/GameClock.h"
#include "core/native/ObjectManager.h"
#include "memory/MemReader.h"
#include "offsets/OffsetsLoading.h"
#include "offsets/OffsetsMap.h"
#include "offsets/OffsetsSocial.h"

namespace
{
    namespace B = Offsets::Battlefield;

    constexpr int STATUS_NONE    = 0;
    constexpr int STATUS_QUEUED  = 1;
    constexpr int STATUS_CONFIRM = 2;
    constexpr int STATUS_ACTIVE  = 3;

    const char* statusName(int status)
    {
        switch (status)
        {
            case STATUS_NONE:    return "none";
            case STATUS_QUEUED:  return "queued";
            case STATUS_CONFIRM: return "confirm";
            case STATUS_ACTIVE:  return "active";
            default:             return "error";
        }
    }

    uintptr_t queueBase(int index)
    {
        return B::QUEUES + static_cast<uintptr_t>(index - 1) * B::QUEUE_SIZE;
    }

    std::string mapName(int mapId)
    {
        uintptr_t row = ClientDb::row(Offsets::MapDb::MIN_ID, Offsets::MapDb::MAX_ID, Offsets::MapDb::ROWS, mapId);
        return ClientDb::readStringField(row, Offsets::Instance::MAP_ROW_NAME);
    }

    uintptr_t battlemasterRow(int typeId)
    {
        return ClientDb::row(Offsets::BattlemasterListDb::MIN_ID, Offsets::BattlemasterListDb::MAX_ID,
                             Offsets::BattlemasterListDb::ROWS, typeId);
    }

    bool validIndex(int index)
    {
        return index >= 1 && index <= B::MAX_QUEUES;
    }

    void sendPort(int index, bool accept)
    {
        uintptr_t base = queueBase(index);
        WoWGUID guid = Memory::safeRead<WoWGUID>(base + B::QUEUE_GUID);

        ClientPacket packet(B::OPCODE_PORT);
        packet.writeU64(guid.low, guid.high);
        packet.writeU8(accept ? 1 : 0);
        packet.send();
    }
}

namespace CoreAPI
{
    namespace Battlefield
    {
        BattlefieldQueue GetQueue(int index)
        {
            BattlefieldQueue q = {};
            q.index = index;
            q.status = statusName(STATUS_NONE);
            if (!validIndex(index))
                return q;

            uintptr_t base = queueBase(index);
            q.statusId     = Memory::safeRead<int>(base + B::QUEUE_STATUS);
            q.status       = statusName(q.statusId);
            q.queueGuid    = Memory::safeRead<WoWGUID>(base + B::QUEUE_GUID);
            q.instanceGuid = Memory::safeRead<WoWGUID>(base + B::QUEUE_INSTANCE_GUID);
            q.typeId       = Memory::safeRead<int>(base + B::QUEUE_TYPE_ID);
            q.random       = q.typeId == B::TYPE_RANDOM;
            q.minLevel     = Memory::safeRead<int>(base + B::QUEUE_MIN_LEVEL);
            q.maxLevel     = Memory::safeRead<int>(base + B::QUEUE_MAX_LEVEL);
            q.instanceId   = Memory::safeRead<int>(base + B::QUEUE_INSTANCE_ID);
            q.teamSize     = Memory::safeRead<int>(base + B::QUEUE_TEAM_SIZE);
            q.arena        = q.teamSize > 0;
            q.rated        = Memory::safeRead<int>(base + B::QUEUE_RATED) != 0;
            q.canLeave     = q.teamSize == 0;
            q.typeName     = ClientDb::readStringField(battlemasterRow(q.typeId), Offsets::BattlemasterListDb::ROW_NAME);
            q.estimatedWaitMs = Memory::safeRead<int>(base + B::QUEUE_ESTIMATED_WAIT);

            int now = GameClock::nowMs();
            int joinedAt = Memory::safeRead<int>(base + B::QUEUE_JOINED_AT);
            q.timeWaitedMs = joinedAt != 0 ? now - joinedAt : 0;
            int expiresAt = Memory::safeRead<int>(base + B::QUEUE_PORT_EXPIRES_AT);
            q.portExpiresInMs = (expiresAt != 0 && expiresAt - now > 0) ? expiresAt - now : 0;

            if (q.statusId == STATUS_ACTIVE)
            {
                q.mapId = Memory::safeRead<int>(B::ACTIVE_MAP_ID);
            }
            else if (q.statusId == STATUS_CONFIRM && q.random)
            {
                uint64_t raw = q.instanceGuid.raw();
                int realType = static_cast<int>(static_cast<uint32_t>(raw >> 16));
                uintptr_t row = battlemasterRow(realType);
                q.mapId = row ? Memory::safeRead<int>(row + Offsets::BattlemasterListDb::ROW_MAP_ID)
                              : Memory::safeRead<int>(base + B::QUEUE_MAP_ID);
            }
            else
            {
                q.mapId = Memory::safeRead<int>(base + B::QUEUE_MAP_ID);
            }

            q.mapName = (q.statusId == STATUS_CONFIRM || q.statusId == STATUS_ACTIVE) ? mapName(q.mapId) : std::string();
            if (q.mapName.empty())
                q.mapName = q.typeName;
            return q;
        }

        std::vector<BattlefieldQueue> GetQueues()
        {
            std::vector<BattlefieldQueue> queues;
            for (int i = 1; i <= B::MAX_QUEUES; i++)
                queues.push_back(GetQueue(i));
            return queues;
        }

        BattlefieldResult AcceptPort(int index)
        {
            if (!validIndex(index)) return BattlefieldResult::InvalidIndex;
            if (!WoW::GetLocalGUID().isValid()) return BattlefieldResult::NotInWorld;
            if (GetQueue(index).statusId != STATUS_CONFIRM) return BattlefieldResult::NotReady;

            sendPort(index, true);
            return BattlefieldResult::Ok;
        }

        BattlefieldResult LeaveQueue(int index)
        {
            if (!validIndex(index)) return BattlefieldResult::InvalidIndex;
            if (!WoW::GetLocalGUID().isValid()) return BattlefieldResult::NotInWorld;

            BattlefieldQueue queue = GetQueue(index);
            if (queue.statusId != STATUS_QUEUED && queue.statusId != STATUS_CONFIRM)
                return BattlefieldResult::NotReady;
            if (queue.statusId == STATUS_CONFIRM && !queue.canLeave)
                return BattlefieldResult::CannotLeave;

            sendPort(index, false);

            using StopSoundFn = void(__cdecl*)();
            reinterpret_cast<StopSoundFn>(B::STOP_READY_SOUND)();
            if (Memory::safeRead<int>(queueBase(index) + B::QUEUE_TYPE_ID) == Memory::safeRead<int>(B::STATUS_QUEUE_TYPE))
                FrameScript::FireEventById(B::LEAVE_EVENT_ID);
            return BattlefieldResult::Ok;
        }
    }
}
