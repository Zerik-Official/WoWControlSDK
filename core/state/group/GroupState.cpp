#include "GroupState.h"
#include "memory/MemReader.h"
#include "offsets_world.h"
#include <cstring>

namespace GroupState {

static std::vector<uint64_t> readPartyGuids()
{
    std::vector<uint64_t> guids;
    guids.reserve(PARTY_MAX_MEMBERS);

    for (int i = 0; i < PARTY_MAX_MEMBERS; i++)
    {
        uint64_t guid = Memory::safeRead<uint64_t>(
            ADDR_PARTY_PLAYER_GUIDS + (uintptr_t)(i * sizeof(uint64_t))
        );
        if (guid) guids.push_back(guid);
    }

    return guids;
}

static std::vector<uint64_t> readRaidGuids(uint64_t localGuid)
{
    std::vector<uint64_t> guids;
    guids.reserve(RAID_MAX_MEMBERS);

    for (int i = 0; i < RAID_MAX_MEMBERS; i++)
    {
        uintptr_t ptr = Memory::safeRead<uintptr_t>(
            ADDR_RAID_GROUP_START + (uintptr_t)(i * sizeof(uintptr_t))
        );
        if (!ptr) continue;

        uint64_t guid = Memory::safeRead<uint64_t>(ptr);
        if (guid && guid != localGuid) guids.push_back(guid);
    }

    return guids;
}

Info read()
{
    Info info = {};
    info.valid = false;
    info.type  = GroupType::NONE;

    uint64_t raidLeader  = Memory::safeRead<uint64_t>(ADDR_RAID_LEADER);
    uint64_t partyLeader = Memory::safeRead<uint64_t>(ADDR_PARTY_LEADER);

    info.leaderGuid = raidLeader ? raidLeader : partyLeader;

    if (!info.leaderGuid) return info;

    uintptr_t cc = Memory::safeRead<uintptr_t>(STATIC_CLIENT_CONNECTION);
    if (!cc) return info;

    uintptr_t om = Memory::safeRead<uintptr_t>(cc + OBJECT_MANAGER_OFFSET);
    if (!om) return info;

    uint64_t localGuid = Memory::safeRead<uint64_t>(om + LOCAL_GUID_OFFSET);

    std::vector<uint64_t> guids;

    if (raidLeader)
    {
        info.type = GroupType::RAID;
        guids     = readRaidGuids(localGuid);
    }
    else
    {
        info.type = GroupType::PARTY;
        guids     = readPartyGuids();
    }

    info.members.reserve(guids.size());
    for (uint64_t guid : guids)
    {
        if (guid == localGuid) continue;
        GroupMemberState::Info member = GroupMemberState::read(guid);
        if (member.valid) info.members.push_back(member);
    }

    info.valid = true;
    return info;
}

}