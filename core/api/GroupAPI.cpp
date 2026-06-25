#include "core/api/GroupAPI.h"
#include "core/native/Unit.h"
#include "core/native/ObjectManager.h"
#include "memory/MemReader.h"
#include "offsets/OffsetsGroup.h"
#include <algorithm>

namespace
{
    struct GroupCache
    {
        CoreAPI::GroupInfo info;
        uint32_t checksum = 0;
        bool valid = false;
    };

    CoreAPI::GroupInfo buildGroup();

    static GroupCache& getCache()
    {
        static GroupCache cache;
        return cache;
    }

    static uint32_t computeChecksum()
    {
        uint32_t sum = 0;
        for (int i = 0; i < Offsets::Group::PARTY_MAX_MEMBERS; i++)
        {
            WoWGUID guid = Memory::safeRead<WoWGUID>(
                Offsets::Group::PARTY_PLAYER_GUIDS + (uintptr_t)(i * sizeof(WoWGUID))
            );
            sum ^= guid.low ^ guid.high;
        }
        for (int i = 0; i < Offsets::Group::RAID_MAX_MEMBERS; i++)
        {
            uintptr_t ptr = Memory::safeRead<uintptr_t>(
                Offsets::Group::RAID_GROUP_START + (uintptr_t)(i * sizeof(uintptr_t))
            );
            if (!ptr) continue;
            WoWGUID guid = Memory::safeRead<WoWGUID>(ptr);
            sum ^= guid.low ^ guid.high;
        }
        return sum;
    }

    static CoreAPI::GroupInfo& getOrBuildGroup()
    {
        GroupCache& cache = getCache();
        uint32_t cs = computeChecksum();
        if (cache.valid && cache.checksum == cs)
            return cache.info;
        cache.info = buildGroup();
        cache.checksum = cs;
        cache.valid = true;
        return cache.info;
    }

    std::vector<WoWGUID> readPartyGuids()
    {
        std::vector<WoWGUID> guids;
        guids.reserve(Offsets::Group::PARTY_MAX_MEMBERS);

        for (int i = 0; i < Offsets::Group::PARTY_MAX_MEMBERS; i++)
        {
            WoWGUID guid = Memory::safeRead<WoWGUID>(
                Offsets::Group::PARTY_PLAYER_GUIDS + (uintptr_t)(i * sizeof(WoWGUID))
            );
            if (guid.isValid()) guids.push_back(guid);
        }

        return guids;
    }

    std::vector<WoWGUID> readRaidGuids(const WoWGUID& localGuid)
    {
        std::vector<WoWGUID> guids;
        guids.reserve(Offsets::Group::RAID_MAX_MEMBERS);

        for (int i = 0; i < Offsets::Group::RAID_MAX_MEMBERS; i++)
        {
            uintptr_t ptr = Memory::safeRead<uintptr_t>(
                Offsets::Group::RAID_GROUP_START + (uintptr_t)(i * sizeof(uintptr_t))
            );
            if (!ptr) continue;

            WoWGUID guid = Memory::safeRead<WoWGUID>(ptr);
            if (guid.isValid() && guid != localGuid)
                guids.push_back(guid);
        }

        return guids;
    }

    CoreAPI::GroupMember buildMember(const WoWGUID& guid)
    {
        CoreAPI::GroupMember m = {};

        uintptr_t base = WoW::GetObjectByGUID(guid);
        if (!base) return m;

        ::Unit unit(base);
        if (!unit.exists()) return m;

        m.handle         = CoreAPI::UnitHandle{ base, guid };
        m.name           = unit.getName();
        m.level          = unit.getLevel();
        m.race           = unit.getRace();
        m.class_         = unit.getClass();
        m.health         = unit.getHealth();
        m.healthMax      = unit.getMaxHealth();
        m.power          = unit.getPower();
        m.powerMax       = unit.getMaxPower();
        m.targetGuid     = unit.getTargetGUID();
        m.isInCombat     = unit.isInCombat();
        m.isDead         = unit.isDead();
        m.isGhost        = unit.isGhost();
        m.isCasting      = unit.isCasting();
        m.isChanneling   = unit.isChanneling();
        m.isAutoAttacking= unit.isAutoAttacking();

        return m;
    }

    CoreAPI::GroupInfo buildGroup()
    {
        CoreAPI::GroupInfo info = {};
        info.valid = false;
        info.type  = CoreAPI::GroupType::None;

        WoWGUID raidLeader  = Memory::safeRead<WoWGUID>(Offsets::Group::RAID_LEADER);
        WoWGUID partyLeader = Memory::safeRead<WoWGUID>(Offsets::Group::PARTY_LEADER);

        info.leaderGuid = raidLeader.isValid() ? raidLeader : partyLeader;
        if (!info.leaderGuid.isValid()) return info;

        WoWGUID localGuid = WoW::GetLocalGUID();
        if (!localGuid.isValid()) return info;

        std::vector<WoWGUID> guids;

        if (partyLeader.isValid())
        {
            info.type = CoreAPI::GroupType::Party;
            auto pg = readPartyGuids();
            guids.insert(guids.end(), pg.begin(), pg.end());
        }

        if (raidLeader.isValid())
        {
            info.type = CoreAPI::GroupType::Raid;
            auto rg = readRaidGuids(localGuid);
            guids.insert(guids.end(), rg.begin(), rg.end());
        }

        std::sort(guids.begin(), guids.end(),
            [](const WoWGUID& a, const WoWGUID& b) {
                return a.raw() < b.raw();
            });
        guids.erase(std::unique(guids.begin(), guids.end(),
            [](const WoWGUID& a, const WoWGUID& b) { return a == b; }),
            guids.end());
        guids.erase(std::remove_if(guids.begin(), guids.end(),
            [&](const WoWGUID& g) { return !g.isValid() || g == localGuid; }),
            guids.end());

        info.members.reserve(guids.size());
        for (const WoWGUID& guid : guids)
        {
            CoreAPI::GroupMember member = buildMember(guid);
            if (!member.handle.isNull())
                info.members.push_back(member);
        }

        info.valid = true;
        return info;
    }
}

namespace CoreAPI
{
    namespace Group
    {
        GroupInfo GetGroup()
        {
            return getOrBuildGroup();
        }

        GroupType GetGroupType()
        {
            return getOrBuildGroup().type;
        }

        bool IsInGroup()
        {
            GroupInfo& g = getOrBuildGroup();
            return g.valid && g.type != GroupType::None;
        }

        bool IsInRaid()
        {
            return getOrBuildGroup().type == GroupType::Raid;
        }

        int GetMemberCount()
        {
            return (int)getOrBuildGroup().members.size();
        }

        WoWGUID GetLeaderGuid()
        {
            return getOrBuildGroup().leaderGuid;
        }

        GroupCombatSummary GetCombatSummary()
        {
            GroupInfo&         group = getOrBuildGroup();
            GroupCombatSummary s     = {};

            for (const GroupMember& m : group.members)
            {
                if (m.handle.isNull()) continue;

                if (m.isInCombat)
                {
                    s.anyInCombat = true;
                    s.countInCombat++;
                }

                if (m.isDead)
                {
                    s.anyDead = true;
                    s.countDead++;
                }

                if (m.isCasting || m.isChanneling)
                    s.anyCasting = true;

                if (m.isInCombat || m.isAutoAttacking || m.isCasting)
                    s.anyEngaging = true;
            }

            return s;
        }

        UnitRef GetMember(int index)
        {
            GroupInfo& g = getOrBuildGroup();
            if (index < 0 || index >= (int)g.members.size())
                return UnitRef(NullHandle());

            return UnitRef(g.members[index].handle);
        }

        UnitRef GetMemberByGUID(WoWGUID guid)
        {
            if (!guid.isValid()) return UnitRef(NullHandle());

            uintptr_t base = WoW::GetObjectByGUID(guid);
            if (!base) return UnitRef(NullHandle());

            return UnitRef(UnitHandle{ base, guid });
        }
    }
}
