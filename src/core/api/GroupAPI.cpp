#include "core/api/GroupAPI.h"
#include "core/native/Unit.h"
#include "core/native/ObjectManager.h"
#include "core/native/ClientDb.h"
#include "core/native/NameCache.h"
#include "core/native/FrameScript.h"
#include "memory/MemReader.h"
#include "offsets/OffsetsGroup.h"
#include "offsets/OffsetsSocial.h"
#include "runtime/events/GameEventBridge.h"
#include <Windows.h>
#include <algorithm>

namespace
{
    constexpr uint16_t STATUS_ONLINE   = 0x01;
    constexpr uint16_t STATUS_PVP      = 0x02;
    constexpr uint16_t STATUS_DEAD     = 0x04;
    constexpr uint16_t STATUS_GHOST    = 0x08;
    constexpr uint16_t STATUS_PVP_FFA  = 0x10;
    constexpr uint16_t STATUS_AFK      = 0x40;
    constexpr uint16_t STATUS_DND      = 0x80;

    constexpr uint32_t ROLE_FLAG_MAIN_TANK   = 0x02;
    constexpr uint32_t ROLE_FLAG_MAIN_ASSIST = 0x04;

    constexpr int RANK_ASSISTANT = 1;
    constexpr int RANK_LEADER    = 2;

    constexpr int LOOT_METHOD_MASTER = 2;

    SRWLOCK              s_inviteLock = SRWLOCK_INIT;
    CoreAPI::GroupInvite s_invite     = { false, {}, 0 };

    std::vector<CoreAPI::GroupAura> readAuras(uintptr_t spells, uintptr_t flags)
    {
        std::vector<CoreAPI::GroupAura> auras;
        for (int i = 0; i < Offsets::Group::STATS_MAX_AURAS; i++)
        {
            uint32_t spellId = Memory::safeRead<uint32_t>(spells + static_cast<uintptr_t>(i) * 4);
            if (!spellId) continue;
            uint8_t flag = Memory::safeRead<uint8_t>(flags + static_cast<uintptr_t>(i));
            auras.push_back(CoreAPI::GroupAura{ spellId, flag });
        }
        return auras;
    }

    CoreAPI::GroupMemberStats readStats(uintptr_t stats)
    {
        namespace G = Offsets::Group;

        CoreAPI::GroupMemberStats s = {};
        if (!stats) return s;

        s.available     = true;
        s.statusFlags   = Memory::safeRead<uint16_t>(stats + G::STATS_STATUS);
        s.online        = (s.statusFlags & STATUS_ONLINE) != 0;
        s.pvp           = (s.statusFlags & STATUS_PVP) != 0;
        s.dead          = (s.statusFlags & STATUS_DEAD) != 0;
        s.ghost         = (s.statusFlags & STATUS_GHOST) != 0;
        s.pvpFreeForAll = (s.statusFlags & STATUS_PVP_FFA) != 0;
        s.afk           = (s.statusFlags & STATUS_AFK) != 0;
        s.dnd           = (s.statusFlags & STATUS_DND) != 0;
        s.powerType     = Memory::safeRead<uint8_t>(stats + G::STATS_POWER_TYPE);
        s.health        = Memory::safeRead<uint32_t>(stats + G::STATS_HEALTH);
        s.maxHealth     = Memory::safeRead<uint32_t>(stats + G::STATS_MAX_HEALTH);
        s.power         = Memory::safeRead<uint16_t>(stats + G::STATS_POWER);
        s.maxPower      = Memory::safeRead<uint16_t>(stats + G::STATS_MAX_POWER);
        s.level         = Memory::safeRead<uint16_t>(stats + G::STATS_LEVEL);
        s.zoneId        = Memory::safeRead<uint16_t>(stats + G::STATS_ZONE_ID);
        s.zoneName      = ClientDb::areaName(s.zoneId);
        s.positionX     = Memory::safeRead<int16_t>(stats + G::STATS_POSITION_X);
        s.positionY     = Memory::safeRead<int16_t>(stats + G::STATS_POSITION_Y);
        s.auras         = readAuras(stats + G::STATS_AURA_SPELLS, stats + G::STATS_AURA_FLAGS);
        s.vehicleSeat   = Memory::safeRead<uint32_t>(stats + G::STATS_VEHICLE_SEAT);

        WoWGUID petGuid = Memory::safeRead<WoWGUID>(stats + G::STATS_PET_GUID);
        s.hasPet = petGuid.isValid();
        if (s.hasPet)
        {
            s.pet.guid      = petGuid;
            s.pet.name      = ClientDb::readCString(stats + G::STATS_PET_NAME, G::STATS_PET_NAME_LENGTH);
            s.pet.modelId   = Memory::safeRead<uint16_t>(stats + G::STATS_PET_MODEL_ID);
            s.pet.powerType = Memory::safeRead<uint8_t>(stats + G::STATS_PET_POWER_TYPE);
            s.pet.health    = Memory::safeRead<uint32_t>(stats + G::STATS_PET_HEALTH);
            s.pet.maxHealth = Memory::safeRead<uint32_t>(stats + G::STATS_PET_MAX_HEALTH);
            s.pet.power     = Memory::safeRead<uint16_t>(stats + G::STATS_PET_POWER);
            s.pet.maxPower  = Memory::safeRead<uint16_t>(stats + G::STATS_PET_MAX_POWER);
            s.pet.auras     = readAuras(stats + G::STATS_PET_AURA_SPELLS, stats + G::STATS_PET_AURA_FLAGS);
        }
        return s;
    }

    CoreAPI::GroupMember buildMember(const WoWGUID& guid, uintptr_t stats)
    {
        CoreAPI::GroupMember m = {};
        m.guid        = guid;
        m.handle      = CoreAPI::NullHandle();
        m.partyIndex  = -1;
        m.raidIndex   = -1;
        m.subgroup    = 0;
        m.stats       = readStats(stats);

        NameCache::Entry cached = NameCache::lookup(guid);
        m.nameKnown = cached.found;
        m.name      = cached.name;
        m.race      = cached.race;
        m.gender    = cached.gender;
        m.class_    = cached.class_;

        m.level     = m.stats.level;
        m.health    = static_cast<int>(m.stats.health);
        m.healthMax = static_cast<int>(m.stats.maxHealth);
        m.power     = m.stats.power;
        m.powerMax  = m.stats.maxPower;
        m.isDead    = m.stats.dead;
        m.isGhost   = m.stats.ghost;

        uintptr_t base = WoW::GetObjectByGUID(guid);
        if (base)
        {
            ::Unit unit(base);
            if (unit.exists())
            {
                m.inRange         = true;
                m.handle          = CoreAPI::UnitHandle{ base, guid };
                m.name            = unit.getName();
                m.nameKnown       = !m.name.empty();
                m.level           = unit.getLevel();
                m.race            = unit.getRace();
                m.gender          = unit.getGender();
                m.class_          = unit.getClass();
                m.health          = unit.getHealth();
                m.healthMax       = unit.getMaxHealth();
                m.power           = unit.getPower();
                m.powerMax        = unit.getMaxPower();
                m.targetGuid      = unit.getTargetGUID();
                m.isInCombat      = unit.isInCombat();
                m.isDead          = unit.isDead();
                m.isGhost         = unit.isGhost();
                m.isCasting       = unit.isCasting();
                m.isChanneling    = unit.isChanneling();
                m.isAutoAttacking = unit.isAutoAttacking();
            }
        }

        m.className     = ClientDb::className(m.class_);
        m.classFileName = ClientDb::classFileName(m.class_);
        return m;
    }

    void readPartyMembers(CoreAPI::GroupInfo& info, const WoWGUID& localGuid)
    {
        for (int i = 0; i < Offsets::Group::PARTY_MAX_MEMBERS; i++)
        {
            WoWGUID guid = Memory::safeRead<WoWGUID>(
                Offsets::Group::PARTY_PLAYER_GUIDS + static_cast<uintptr_t>(i) * sizeof(WoWGUID)
            );
            if (!guid.isValid() || guid == localGuid) continue;

            uintptr_t stats = Offsets::Group::PARTY_MEMBER_STATS
                            + static_cast<uintptr_t>(i) * Offsets::Group::MEMBER_STATS_SIZE;
            CoreAPI::GroupMember m = buildMember(guid, stats);
            m.partyIndex = i + 1;
            m.isLeader   = guid == info.leaderGuid;
            info.members.push_back(m);
        }
    }

    void readRaidMembers(CoreAPI::GroupInfo& info, const WoWGUID& localGuid)
    {
        namespace G = Offsets::Group;

        int count = (std::min)(Memory::safeRead<int>(G::RAID_MEMBER_COUNT), G::RAID_MAX_MEMBERS);
        for (int i = 0; i < count; i++)
        {
            uintptr_t entry = Memory::safeRead<uintptr_t>(G::RAID_GROUP_START + static_cast<uintptr_t>(i) * sizeof(uintptr_t));
            if (!entry) continue;

            WoWGUID guid = Memory::safeRead<WoWGUID>(entry + G::RAID_MEMBER_GUID);
            if (!guid.isValid()) continue;

            int      subgroup  = Memory::safeRead<int>(entry + G::RAID_MEMBER_SUBGROUP) + 1;
            int      rank      = Memory::safeRead<int>(entry + G::RAID_MEMBER_RANK);
            uint32_t roleFlags = Memory::safeRead<uint32_t>(entry + G::RAID_MEMBER_ROLE_FLAGS);
            uint32_t lfgRoles  = Memory::safeRead<uint32_t>(entry + G::RAID_MEMBER_LFG_ROLES);

            if (guid == localGuid)
            {
                info.playerSubgroup = subgroup;
                info.playerRank     = rank;
                continue;
            }

            CoreAPI::GroupMember m = buildMember(guid, entry + G::RAID_MEMBER_STATS);
            m.raidIndex      = i + 1;
            m.subgroup       = subgroup;
            m.rank           = rank;
            m.isLeader       = rank == RANK_LEADER || guid == info.leaderGuid;
            m.isAssistant    = rank == RANK_ASSISTANT;
            m.isMainTank     = (roleFlags & ROLE_FLAG_MAIN_TANK) != 0;
            m.isMainAssist   = (roleFlags & ROLE_FLAG_MAIN_ASSIST) != 0;
            m.lfgRoles       = lfgRoles;
            info.members.push_back(m);
        }
    }

    CoreAPI::GroupInfo buildGroup()
    {
        namespace G = Offsets::Group;

        CoreAPI::GroupInfo info = {};
        info.valid = false;
        info.type  = CoreAPI::GroupType::None;
        info.size  = 1;

        WoWGUID raidLeader  = Memory::safeRead<WoWGUID>(G::RAID_LEADER);
        WoWGUID partyLeader = Memory::safeRead<WoWGUID>(G::PARTY_LEADER);

        info.leaderGuid = raidLeader.isValid() ? raidLeader : partyLeader;

        WoWGUID localGuid = WoW::GetLocalGUID();
        if (!localGuid.isValid()) return info;

        for (int i = 0; i < G::PARTY_MAX_MEMBERS; i++)
        {
            WoWGUID guid = Memory::safeRead<WoWGUID>(G::PARTY_PLAYER_GUIDS + static_cast<uintptr_t>(i) * sizeof(WoWGUID));
            if (guid.isValid()) info.partyMemberCount++;
        }
        info.raidMemberCount      = Memory::safeRead<int>(G::RAID_MEMBER_COUNT);
        info.realPartyMemberCount = Memory::safeRead<int>(G::REAL_PARTY_MEMBER_COUNT);
        info.realRaidMemberCount  = Memory::safeRead<int>(G::REAL_RAID_MEMBER_COUNT);
        info.lootMethod           = Memory::safeRead<int>(G::LOOT_METHOD);
        info.masterLooterGuid     = Memory::safeRead<WoWGUID>(G::MASTER_LOOTER_GUID);
        info.dungeonDifficulty    = Memory::safeRead<int>(G::PARTY_DUNGEON_DIFFICULTY);
        info.raidDifficulty       = Memory::safeRead<int>(G::PARTY_RAID_DIFFICULTY);

        if (info.raidMemberCount > 0)
        {
            info.type = CoreAPI::GroupType::Raid;
            info.size = info.raidMemberCount;
            readRaidMembers(info, localGuid);
        }
        else if (info.partyMemberCount > 0)
        {
            info.type = CoreAPI::GroupType::Party;
            info.size = info.partyMemberCount + 1;
            readPartyMembers(info, localGuid);
        }

        bool masterLoot = info.lootMethod == LOOT_METHOD_MASTER && info.masterLooterGuid.isValid();
        for (CoreAPI::GroupMember& m : info.members)
            m.isMasterLooter = masterLoot && m.guid == info.masterLooterGuid;

        info.valid = info.leaderGuid.isValid() || info.type != CoreAPI::GroupType::None;
        return info;
    }

    uintptr_t localPlayerBase()
    {
        WoWGUID guid = WoW::GetLocalGUID();
        if (!guid.isValid()) return 0;
        return WoW::GetObjectByGUID(guid);
    }
}

namespace CoreAPI
{
    namespace Group
    {
        GroupInfo GetGroup()
        {
            return buildGroup();
        }

        GroupType GetGroupType()
        {
            return buildGroup().type;
        }

        bool IsInGroup()
        {
            GroupInfo g = buildGroup();
            return g.valid && g.type != GroupType::None;
        }

        bool IsInRaid()
        {
            return buildGroup().type == GroupType::Raid;
        }

        int GetMemberCount()
        {
            return static_cast<int>(buildGroup().members.size());
        }

        WoWGUID GetLeaderGuid()
        {
            return buildGroup().leaderGuid;
        }

        GroupCombatSummary GetCombatSummary()
        {
            GroupInfo          group = buildGroup();
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
            GroupInfo g = buildGroup();
            if (index < 0 || index >= static_cast<int>(g.members.size()))
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

        GroupInvite GetInvite()
        {
            AcquireSRWLockShared(&s_inviteLock);
            GroupInvite invite = s_invite;
            ReleaseSRWLockShared(&s_inviteLock);
            return invite;
        }

        void SetInvite(const std::string& inviterName, uint64_t receivedAtMs)
        {
            AcquireSRWLockExclusive(&s_inviteLock);
            s_invite = GroupInvite{ true, inviterName, receivedAtMs };
            ReleaseSRWLockExclusive(&s_inviteLock);
        }

        void ClearInvite()
        {
            AcquireSRWLockExclusive(&s_inviteLock);
            s_invite = GroupInvite{ false, {}, 0 };
            ReleaseSRWLockExclusive(&s_inviteLock);
        }

        bool AcceptInvite(const InviteRoles& roles)
        {
            uintptr_t player = localPlayerBase();
            if (!player) return false;

            uint32_t flags = 0;
            if (roles.tank)   flags |= Offsets::Lfg::ROLE_TANK;
            if (roles.healer) flags |= Offsets::Lfg::ROLE_HEALER;
            if (roles.damage) flags |= Offsets::Lfg::ROLE_DAMAGE;

            using AcceptFn = void(__thiscall*)(uintptr_t, uint32_t);
            reinterpret_cast<AcceptFn>(Offsets::Group::ACCEPT_INVITE)(player, flags);
            Runtime::GameEventBridge::OnInviteAnswered(true, flags);
            FrameScript::FireEvent("PARTY_INVITE_CANCEL");
            return true;
        }

        bool DeclineInvite()
        {
            uintptr_t player = localPlayerBase();
            if (!player) return false;

            using DeclineFn = void(__thiscall*)(uintptr_t);
            reinterpret_cast<DeclineFn>(Offsets::Group::DECLINE_INVITE)(player);
            Runtime::GameEventBridge::OnInviteAnswered(false, 0);
            FrameScript::FireEvent("PARTY_INVITE_CANCEL");
            return true;
        }
    }
}
