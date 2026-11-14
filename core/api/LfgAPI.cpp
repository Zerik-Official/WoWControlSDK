#include "core/api/LfgAPI.h"
#include "core/native/ClientDb.h"
#include "core/native/ObjectManager.h"
#include "memory/MemReader.h"
#include "offsets/OffsetsSocial.h"

namespace
{
    std::vector<CoreAPI::LfgEncounter> readEncounters(int mapId, int difficulty, uint32_t killedMask)
    {
        namespace E = Offsets::DungeonEncounterDb;

        std::vector<CoreAPI::LfgEncounter> encounters;
        int rows = ClientDb::count(E::COUNT);
        for (int i = 0; i < rows; i++)
        {
            uintptr_t row = ClientDb::indexedRow(E::COUNT, E::ROWS, E::ROW_SIZE, i);
            if (!row) continue;
            if (Memory::safeRead<int>(row + E::ROW_MAP_ID) != mapId) continue;
            if (Memory::safeRead<int>(row + E::ROW_DIFFICULTY) != difficulty) continue;

            CoreAPI::LfgEncounter e = {};
            e.id        = Memory::safeRead<int>(row + E::ROW_ID);
            e.name      = ClientDb::readStringField(row, E::ROW_NAME);
            e.order     = Memory::safeRead<int>(row + E::ROW_ORDER);
            e.bit       = Memory::safeRead<int>(row + E::ROW_BIT);
            e.completed = e.bit >= 0 && e.bit < 32 && (killedMask & (1u << e.bit)) != 0;

            int iconId = Memory::safeRead<int>(row + E::ROW_SPELL_ICON_ID);
            if (iconId)
            {
                uintptr_t icon = ClientDb::row(Offsets::SpellIconDb::MIN_ID, Offsets::SpellIconDb::MAX_ID,
                                               Offsets::SpellIconDb::ROWS, iconId);
                e.iconTexture = ClientDb::readStringField(icon, Offsets::SpellIconDb::ROW_TEXTURE);
            }
            encounters.push_back(e);
        }
        return encounters;
    }

    CoreAPI::LfgResult respond(uintptr_t scriptFunction)
    {
        if (!WoW::GetLocalGUID().isValid())
            return CoreAPI::LfgResult::NotInWorld;
        if (Memory::safeRead<uint32_t>(Offsets::Lfg::PROPOSAL_DUNGEON) == 0)
            return CoreAPI::LfgResult::NoProposal;

        using ScriptFn = int(__cdecl*)(void*);
        reinterpret_cast<ScriptFn>(scriptFunction)(nullptr);
        return CoreAPI::LfgResult::Ok;
    }
}

namespace CoreAPI
{
    namespace Lfg
    {
        std::string RoleName(uint32_t roleFlags)
        {
            if (roleFlags & Offsets::Lfg::ROLE_TANK)   return "TANK";
            if (roleFlags & Offsets::Lfg::ROLE_HEALER) return "HEALER";
            if (roleFlags & Offsets::Lfg::ROLE_DAMAGE) return "DAMAGER";
            return "UNKNOWN";
        }

        LfgDungeon GetDungeon(int dungeonId)
        {
            namespace D = Offsets::LfgDungeonsDb;

            LfgDungeon d = {};
            uintptr_t row = ClientDb::row(D::MIN_ID, D::MAX_ID, D::ROWS, dungeonId);
            if (!row)
                return d;

            d.id             = Memory::safeRead<int>(row + D::ROW_ID);
            d.name           = ClientDb::readStringField(row, D::ROW_NAME);
            d.minLevel       = Memory::safeRead<int>(row + D::ROW_MIN_LEVEL);
            d.maxLevel       = Memory::safeRead<int>(row + D::ROW_MAX_LEVEL);
            d.targetLevel    = Memory::safeRead<int>(row + D::ROW_TARGET_LEVEL);
            d.targetLevelMin = Memory::safeRead<int>(row + D::ROW_TARGET_LEVEL_MIN);
            d.targetLevelMax = Memory::safeRead<int>(row + D::ROW_TARGET_LEVEL_MAX);
            d.mapId          = Memory::safeRead<int>(row + D::ROW_MAP_ID);
            d.difficulty     = Memory::safeRead<int>(row + D::ROW_DIFFICULTY);
            d.flags          = Memory::safeRead<uint32_t>(row + D::ROW_FLAGS);
            d.typeId         = Memory::safeRead<int>(row + D::ROW_TYPE);
            d.faction        = Memory::safeRead<int>(row + D::ROW_FACTION);
            d.texture        = ClientDb::readStringField(row, D::ROW_TEXTURE);
            d.expansion      = Memory::safeRead<int>(row + D::ROW_EXPANSION);
            d.order          = Memory::safeRead<int>(row + D::ROW_ORDER);
            d.groupId        = Memory::safeRead<int>(row + D::ROW_GROUP_ID);
            d.description    = ClientDb::readStringField(row, D::ROW_DESCRIPTION);
            d.holiday        = (d.flags & D::FLAG_HOLIDAY) != 0;
            d.random         = d.typeId == D::TYPE_RANDOM;
            return d;
        }

        LfgProposal GetProposal()
        {
            namespace L = Offsets::Lfg;

            LfgProposal p = {};
            p.rawDungeon = Memory::safeRead<uint32_t>(L::PROPOSAL_DUNGEON);
            if (p.rawDungeon == 0)
                return p;

            p.dungeonId        = static_cast<int>(p.rawDungeon & 0xFFFFFF);
            p.proposalType     = static_cast<int>(p.rawDungeon >> 24);
            p.state            = Memory::safeRead<uint32_t>(L::PROPOSAL_STATE);
            p.proposalId       = Memory::safeRead<uint32_t>(L::PROPOSAL_ID);
            p.bossesKilledMask = Memory::safeRead<uint32_t>(L::PROPOSAL_BOSSES_MASK);
            p.dungeon          = GetDungeon(p.dungeonId);
            p.exists           = p.dungeon.id != 0;
            p.role             = RoleName(0);

            for (int i = 0; i < L::PROPOSAL_MAX_MEMBERS; i++)
            {
                uintptr_t member = L::PROPOSAL_MEMBERS + static_cast<uintptr_t>(i) * L::MEMBER_SIZE;

                LfgProposalMember m = {};
                m.index     = i + 1;
                m.roleFlags = Memory::safeRead<uint32_t>(member + L::MEMBER_FLAGS);
                m.isLeader  = (m.roleFlags & L::MEMBER_FLAG_LEADER) != 0;
                m.role      = RoleName(m.roleFlags);
                m.isPlayer  = Memory::safeRead<uint32_t>(member + L::MEMBER_IS_PLAYER) != 0;
                m.responded = Memory::safeRead<uint32_t>(member + L::MEMBER_RESPONDED) != 0;
                m.accepted  = Memory::safeRead<uint32_t>(member + L::MEMBER_ACCEPTED) != 0;
                p.members.push_back(m);

                if (m.isPlayer)
                {
                    p.role      = m.role;
                    p.responded = m.responded;
                    p.accepted  = m.accepted;
                    p.isLeader  = m.isLeader;
                }
            }

            p.encounters = readEncounters(p.dungeon.mapId, p.dungeon.difficulty, p.bossesKilledMask);
            if (p.dungeon.random)
            {
                p.totalEncounters     = 0;
                p.completedEncounters = p.bossesKilledMask != 0 ? 1 : 0;
            }
            else if (p.dungeon.holiday)
            {
                p.totalEncounters     = 1;
                p.completedEncounters = p.bossesKilledMask != 0 ? 1 : 0;
            }
            else
            {
                p.totalEncounters = static_cast<int>(p.encounters.size());
                for (const LfgEncounter& e : p.encounters)
                    if (e.completed) p.completedEncounters++;
            }
            return p;
        }

        LfgResult AcceptProposal()
        {
            return respond(Offsets::Lfg::ACCEPT_PROPOSAL);
        }

        LfgResult RejectProposal()
        {
            return respond(Offsets::Lfg::REJECT_PROPOSAL);
        }
    }
}
