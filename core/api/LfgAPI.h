#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace CoreAPI
{
    struct LfgDungeon
    {
        int         id;
        std::string name;
        int         minLevel;
        int         maxLevel;
        int         targetLevel;
        int         targetLevelMin;
        int         targetLevelMax;
        int         mapId;
        int         difficulty;
        uint32_t    flags;
        int         typeId;
        int         faction;
        std::string texture;
        int         expansion;
        int         order;
        int         groupId;
        std::string description;
        bool        holiday;
        bool        random;
    };

    struct LfgEncounter
    {
        int         id;
        std::string name;
        int         order;
        int         bit;
        std::string iconTexture;
        bool        completed;
    };

    struct LfgProposalMember
    {
        int         index;
        bool        isPlayer;
        bool        isLeader;
        uint32_t    roleFlags;
        std::string role;
        bool        responded;
        bool        accepted;
    };

    struct LfgProposal
    {
        bool                           exists;
        uint32_t                       proposalId;
        uint32_t                       rawDungeon;
        int                            dungeonId;
        int                            proposalType;
        uint32_t                       state;
        uint32_t                       bossesKilledMask;
        LfgDungeon                     dungeon;
        std::string                    role;
        bool                           responded;
        bool                           accepted;
        bool                           isLeader;
        int                            totalEncounters;
        int                            completedEncounters;
        std::vector<LfgProposalMember> members;
        std::vector<LfgEncounter>      encounters;
    };

    enum class LfgResult
    {
        Ok,
        NotInWorld,
        NoProposal
    };

    namespace Lfg
    {
        LfgProposal GetProposal();
        LfgDungeon  GetDungeon(int dungeonId);
        LfgResult   AcceptProposal();
        LfgResult   RejectProposal();
        std::string RoleName(uint32_t roleFlags);
    }
}
