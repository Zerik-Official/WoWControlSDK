#include "GroupRpc.h"
#include "hooks/GlueHooks.h"
#include "core/api/GroupAPI.h"
#include "rpc/handlers/social/SocialJson.h"
#include "utils/json/Json.h"

namespace Rpc
{
    using SDK::Json;

    static const char* lootMethodName(int method)
    {
        switch (method)
        {
            case 0:  return "freeforall";
            case 1:  return "roundrobin";
            case 2:  return "master";
            case 3:  return "group";
            case 4:  return "needbeforegreed";
            default: return "unknown";
        }
    }

    static Json buildAurasJson(const std::vector<CoreAPI::GroupAura>& auras)
    {
        Json list = Json::array();
        for (const auto& a : auras)
        {
            Json aura;
            aura["spellId"] = a.spellId;
            aura["flags"] = a.flags;
            list.push_back(aura);
        }
        return list;
    }

    static Json buildStatsJson(const CoreAPI::GroupMemberStats& s)
    {
        Json j;
        j["available"] = s.available;
        if (!s.available)
            return j;

        j["statusFlags"] = s.statusFlags;
        j["online"] = s.online;
        j["pvp"] = s.pvp;
        j["dead"] = s.dead;
        j["ghost"] = s.ghost;
        j["pvpFreeForAll"] = s.pvpFreeForAll;
        j["afk"] = s.afk;
        j["dnd"] = s.dnd;
        j["powerType"] = s.powerType;
        j["health"] = s.health;
        j["maxHealth"] = s.maxHealth;
        j["power"] = s.power;
        j["maxPower"] = s.maxPower;
        j["level"] = s.level;
        j["zoneId"] = s.zoneId;
        j["zoneName"] = s.zoneName;
        j["position"] = Json{{"x", s.positionX}, {"y", s.positionY}};
        j["auras"] = buildAurasJson(s.auras);
        j["vehicleSeat"] = s.vehicleSeat;

        if (s.hasPet)
        {
            Json pet;
            pet["guid"] = makeGuidArrayJson(s.pet.guid);
            pet["name"] = s.pet.name;
            pet["modelId"] = s.pet.modelId;
            pet["powerType"] = s.pet.powerType;
            pet["health"] = s.pet.health;
            pet["maxHealth"] = s.pet.maxHealth;
            pet["power"] = s.pet.power;
            pet["maxPower"] = s.pet.maxPower;
            pet["auras"] = buildAurasJson(s.pet.auras);
            j["pet"] = pet;
        }
        else
        {
            j["pet"] = nullptr;
        }
        return j;
    }

    static Json buildMemberJson(const CoreAPI::GroupMember& m)
    {
        Json j;
        j["guid"] = makeGuidArrayJson(m.guid);
        j["inRange"] = m.inRange;
        j["nameKnown"] = m.nameKnown;
        j["name"] = m.name;
        j["level"] = m.level;
        j["race"] = m.race;
        j["gender"] = m.gender;
        j["class"] = m.class_;
        j["className"] = m.className;
        j["classFileName"] = m.classFileName;
        j["online"] = m.stats.available ? m.stats.online : m.inRange;
        j["health"] = m.health;
        j["maxHealth"] = m.healthMax;
        j["power"] = m.power;
        j["powerMax"] = m.powerMax;
        j["targetGuid"] = makeGuidArrayJson(m.targetGuid);
        j["inCombat"] = m.isInCombat;
        j["dead"] = m.isDead;
        j["ghost"] = m.isGhost;
        j["casting"] = m.isCasting;
        j["channeling"] = m.isChanneling;
        j["autoAttacking"] = m.isAutoAttacking;
        j["partyIndex"] = m.partyIndex;
        j["raidIndex"] = m.raidIndex;
        j["subgroup"] = m.subgroup;
        j["rank"] = m.rank;
        j["isLeader"] = m.isLeader;
        j["isAssistant"] = m.isAssistant;
        j["isMainTank"] = m.isMainTank;
        j["isMainAssist"] = m.isMainAssist;
        j["isMasterLooter"] = m.isMasterLooter;
        j["lfgRoles"] = makeRolesJson(m.lfgRoles);
        j["stats"] = buildStatsJson(m.stats);
        return j;
    }

    static Json handleGet(const Json&)
    {
        std::string raw = Hooks::Glue::Execute([]() -> std::string {
            CoreAPI::GroupInfo group = CoreAPI::Group::GetGroup();

            Json j;
            j["valid"] = group.valid;

            if (!group.valid)
                return j.dump();

            switch (group.type)
            {
                case CoreAPI::GroupType::Party: j["type"] = "party"; break;
                case CoreAPI::GroupType::Raid:  j["type"] = "raid";  break;
                default:                        j["type"] = "none";   break;
            }

            j["leaderGuid"] = makeGuidArrayJson(group.leaderGuid);
            j["memberCount"] = static_cast<int>(group.members.size());
            j["size"] = group.size;
            j["partyMemberCount"] = group.partyMemberCount;
            j["raidMemberCount"] = group.raidMemberCount;
            j["realPartyMemberCount"] = group.realPartyMemberCount;
            j["realRaidMemberCount"] = group.realRaidMemberCount;
            j["lootMethod"] = lootMethodName(group.lootMethod);
            j["lootMethodId"] = group.lootMethod;
            j["masterLooterGuid"] = makeGuidArrayJson(group.masterLooterGuid);
            j["dungeonDifficulty"] = group.dungeonDifficulty + 1;
            j["raidDifficulty"] = group.raidDifficulty + 1;
            j["playerSubgroup"] = group.playerSubgroup;
            j["playerRank"] = group.playerRank;

            int inRange = 0;
            Json members = Json::array();
            for (const auto& m : group.members)
            {
                if (m.inRange) inRange++;
                members.push_back(buildMemberJson(m));
            }
            j["inRangeCount"] = inRange;
            j["members"] = members;

            return j.dump();
        });

        return Json::parse(raw);
    }

    static Json handleCombatSummary(const Json&)
    {
        std::string raw = Hooks::Glue::Execute([]() -> std::string {
            CoreAPI::GroupCombatSummary s = CoreAPI::Group::GetCombatSummary();

            Json j;
            j["anyInCombat"] = s.anyInCombat;
            j["anyDead"] = s.anyDead;
            j["anyCasting"] = s.anyCasting;
            j["anyEngaging"] = s.anyEngaging;
            j["countInCombat"] = s.countInCombat;
            j["countDead"] = s.countDead;
            return j.dump();
        });

        return Json::parse(raw);
    }

    static Json handleGetInvite(const Json&)
    {
        CoreAPI::GroupInvite invite = CoreAPI::Group::GetInvite();
        Json j;
        j["pending"] = invite.pending;
        j["inviterName"] = invite.inviterName;
        j["receivedAtMs"] = invite.receivedAtMs;
        return j;
    }

    static bool readRole(const Json& params, const char* key)
    {
        return params.is_object() && params.contains(key) && params[key].is_boolean() && params[key].get<bool>();
    }

    static Json handleAcceptInvite(const Json& params)
    {
        CoreAPI::InviteRoles roles = {
            readRole(params, "tank"),
            readRole(params, "healer"),
            readRole(params, "damage")
        };

        std::string raw = Hooks::Glue::Execute([roles]() -> std::string {
            if (!CoreAPI::Group::AcceptInvite(roles))
                return SDK::makeErrorJson("not in world").dump();
            return SDK::okJson().dump();
        });

        return Json::parse(raw);
    }

    static Json handleDeclineInvite(const Json&)
    {
        std::string raw = Hooks::Glue::Execute([]() -> std::string {
            if (!CoreAPI::Group::DeclineInvite())
                return SDK::makeErrorJson("not in world").dump();
            return SDK::okJson().dump();
        });

        return Json::parse(raw);
    }

    void registerGroupMethods(Runtime::MethodRegistry& registry)
    {
        registry.registerMethod("group.get", handleGet);
        registry.registerMethod("group.combatSummary", handleCombatSummary);
        registry.registerMethod("group.getInvite", handleGetInvite);
        registry.registerMethod("group.acceptInvite", handleAcceptInvite);
        registry.registerMethod("group.declineInvite", handleDeclineInvite);
    }
}
