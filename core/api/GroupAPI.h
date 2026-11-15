#pragma once

#include "core/api/UnitHandle.h"
#include "core/api/UnitAPI.h"
#include <vector>
#include <string>
#include <cstdint>

namespace CoreAPI
{
    enum class GroupType
    {
        None,
        Party,
        Raid
    };

    struct GroupAura
    {
        uint32_t spellId;
        uint8_t  flags;
    };

    struct GroupPet
    {
        WoWGUID                guid;
        std::string            name;
        int                    modelId;
        int                    powerType;
        uint32_t               health;
        uint32_t               maxHealth;
        int                    power;
        int                    maxPower;
        std::vector<GroupAura> auras;
    };

    struct GroupMemberStats
    {
        bool                   available;
        uint16_t               statusFlags;
        bool                   online;
        bool                   pvp;
        bool                   dead;
        bool                   ghost;
        bool                   pvpFreeForAll;
        bool                   afk;
        bool                   dnd;
        int                    powerType;
        uint32_t               health;
        uint32_t               maxHealth;
        int                    power;
        int                    maxPower;
        int                    level;
        int                    zoneId;
        std::string            zoneName;
        int                    positionX;
        int                    positionY;
        std::vector<GroupAura> auras;
        bool                   hasPet;
        GroupPet               pet;
        uint32_t               vehicleSeat;
    };

    struct GroupMember
    {
        UnitHandle          handle;
        WoWGUID             guid;
        bool                inRange;
        bool                nameKnown;
        std::string         name;
        int                 level;
        int                 race;
        int                 gender;
        int                 class_;
        std::string         className;
        std::string         classFileName;
        int                 health;
        int                 healthMax;
        int                 power;
        int                 powerMax;
        WoWGUID             targetGuid;
        bool                isInCombat;
        bool                isDead;
        bool                isGhost;
        bool                isCasting;
        bool                isChanneling;
        bool                isAutoAttacking;

        int                 partyIndex;
        int                 raidIndex;
        int                 subgroup;
        int                 rank;
        bool                isLeader;
        bool                isAssistant;
        bool                isMainTank;
        bool                isMainAssist;
        bool                isMasterLooter;
        uint32_t            lfgRoles;
        GroupMemberStats    stats;
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

        int                        size;
        int                        partyMemberCount;
        int                        raidMemberCount;
        int                        realPartyMemberCount;
        int                        realRaidMemberCount;
        int                        lootMethod;
        WoWGUID                    masterLooterGuid;
        int                        dungeonDifficulty;
        int                        raidDifficulty;
        int                        playerSubgroup;
        int                        playerRank;
    };

    struct GroupInvite
    {
        bool        pending;
        std::string inviterName;
        uint64_t    receivedAtMs;
    };

    struct InviteRoles
    {
        bool tank;
        bool healer;
        bool damage;
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

        GroupInvite         GetInvite();
        void                SetInvite(const std::string& inviterName, uint64_t receivedAtMs);
        void                ClearInvite();
        bool                AcceptInvite(const InviteRoles& roles);
        bool                DeclineInvite();
    }
}
