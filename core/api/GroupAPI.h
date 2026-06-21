#pragma once

#include "core/api/UnitHandle.h"
#include "core/api/UnitAPI.h"
#include <vector>
#include <cstdint>

namespace CoreAPI
{
    enum class GroupType
    {
        None,
        Party,
        Raid
    };

    struct GroupMember
    {
        UnitHandle  handle;
        std::string name;
        int         level;
        int         race;
        int         class_;
        int         health;
        int         healthMax;
        int         power;
        int         powerMax;
        WoWGUID     targetGuid;
        bool        isInCombat;
        bool        isDead;
        bool        isGhost;
        bool        isCasting;
        bool        isChanneling;
        bool        isAutoAttacking;
    };

    struct GroupCombatSummary
    {
        bool anyInCombat;
        bool anyDead;
        bool anyCasting;
        bool anyEngaging;
        int  countInCombat;
        int  countDead;
    };

    struct GroupInfo
    {
        GroupType                  type;
        WoWGUID                    leaderGuid;
        std::vector<GroupMember>   members;
        bool                       valid;
    };

    namespace Group
    {
        GroupInfo           GetGroup();
        GroupType           GetGroupType();
        bool                IsInGroup();
        bool                IsInRaid();
        int                 GetMemberCount();
        WoWGUID             GetLeaderGuid();

        GroupCombatSummary  GetCombatSummary();

        UnitRef             GetMember(int index);
        UnitRef             GetMemberByGUID(WoWGUID guid);
    }
}
