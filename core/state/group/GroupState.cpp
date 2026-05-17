#include "GroupState.h"
#include "memory/MemReader.h"
#include "OffsetsGroup.h"
#include "OffsetsObjectMgr.h"
#include <cstring>
#include <algorithm>

namespace GroupState {

static std::vector<uint64_t> readPartyGuids()
{
    std::vector<uint64_t> guids;
    guids.reserve(Offsets::Group::PARTY_MAX_MEMBERS);

    for (int i = 0; i < Offsets::Group::PARTY_MAX_MEMBERS; i++)
    {
        uint64_t guid = Memory::safeRead<uint64_t>(
            Offsets::Group::PARTY_PLAYER_GUIDS + (uintptr_t)(i * sizeof(uint64_t))
        );
        if (guid) guids.push_back(guid);
    }

    return guids;
}

static std::vector<uint64_t> readRaidGuids(uint64_t localGuid)
{
    std::vector<uint64_t> guids;
    guids.reserve(Offsets::Group::RAID_MAX_MEMBERS);

    for (int i = 0; i < Offsets::Group::RAID_MAX_MEMBERS; i++)
    {
        uintptr_t ptr = Memory::safeRead<uintptr_t>(
            Offsets::Group::RAID_GROUP_START + (uintptr_t)(i * sizeof(uintptr_t))
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

    uint64_t raidLeader  = Memory::safeRead<uint64_t>(Offsets::Group::RAID_LEADER);
    uint64_t partyLeader = Memory::safeRead<uint64_t>(Offsets::Group::PARTY_LEADER);

    info.leaderGuid = raidLeader ? raidLeader : partyLeader;
    if (!info.leaderGuid) return info;

    uintptr_t cc = Memory::safeRead<uintptr_t>(Offsets::ObjectMgr::STATIC_CLIENT_CONNECTION);
    if (!cc) return info;

    uintptr_t om = Memory::safeRead<uintptr_t>(cc + Offsets::ObjectMgr::OBJECT_MANAGER_OFFSET);
    if (!om) return info;

    uint64_t localGuid = Memory::safeRead<uint64_t>(om + Offsets::ObjectMgr::LOCAL_GUID_OFFSET);

    std::vector<uint64_t> guids;

    if (partyLeader != 0)
    {
        info.type = GroupType::PARTY;
        auto pg   = readPartyGuids();
        guids.insert(guids.end(), pg.begin(), pg.end());
    }

    if (raidLeader != 0)
    {
        info.type = GroupType::RAID;
        auto rg   = readRaidGuids(localGuid);
        guids.insert(guids.end(), rg.begin(), rg.end());
    }

    std::sort(guids.begin(), guids.end());
    guids.erase(std::unique(guids.begin(), guids.end()), guids.end());
    guids.erase(std::remove_if(guids.begin(), guids.end(),
        [&](uint64_t g) { return g == 0 || g == localGuid; }), guids.end());

    info.members.reserve(guids.size());
    for (uint64_t guid : guids)
    {
        GroupMemberState::Info member = GroupMemberState::read(guid);
        if (member.valid) info.members.push_back(member);
    }

    info.valid = true;
    return info;
}

}