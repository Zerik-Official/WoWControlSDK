#include "UnitResolver.h"
#include "core/api/object/ObjectManager.h"
#include "memory/MemReader.h"
#include "OffsetsGroup.h"
#include "OffsetsUnit.h"

#include <cstring>
#include <cstdlib>

namespace WoW
{
    static WoWGUID resolvePartyMember(int index)
    {
        if (index < 1 || index > Offsets::Group::PARTY_MAX_MEMBERS)
            return NULL_GUID;

        uintptr_t guidAddr = Offsets::Group::PARTY_PLAYER_GUIDS + ((index - 1) * sizeof(WoWGUID));
        return Memory::safeRead<WoWGUID>(guidAddr);
    }

    static WoWGUID resolveRaidMember(int index)
    {
        if (index < 1 || index > Offsets::Group::RAID_MAX_MEMBERS)
            return NULL_GUID;

        uintptr_t guidAddr = Offsets::Group::RAID_GROUP_START + ((index - 1) * sizeof(uint64_t));
        return Memory::safeRead<WoWGUID>(guidAddr);
    }

    bool ResolveUnitToken(const char* token, WoWGUID* outGuid, bool allowNull)
    {
        if (!token || !outGuid)
            return false;

        if (std::strcmp(token, "player") == 0)
        {
            *outGuid = GetLocalGUID();
            return allowNull || outGuid->isValid();
        }

        if (std::strcmp(token, "target") == 0)
        {
            WoWGUID localGuid = GetLocalGUID();
            uintptr_t localObj = GetObjectByGUID(localGuid);

            if (!localObj)
            {
                *outGuid = NULL_GUID;
                return allowNull;
            }

            uintptr_t descBase = Memory::safeRead<uintptr_t>(localObj + Offsets::Unit::DESCRIPTOR_BASE);
            *outGuid = Memory::safeRead<WoWGUID>(descBase + Offsets::Unit::Desc::TARGET_GUID);
            return allowNull || outGuid->isValid();
        }

        if (std::strncmp(token, "party", 5) == 0 && token[5] != '\0')
        {
            int index = token[5] - '0';
            *outGuid = resolvePartyMember(index);
            return allowNull || outGuid->isValid();
        }

        if (std::strncmp(token, "raid", 4) == 0 && token[4] != '\0')
        {
            int index = std::atoi(token + 4);
            *outGuid = resolveRaidMember(index);
            return allowNull || outGuid->isValid();
        }

        *outGuid = NULL_GUID;
        return allowNull;
    }
}