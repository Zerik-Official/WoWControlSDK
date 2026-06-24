#include "GroupRpc.h"
#include "hooks/GlueHooks.h"
#include "core/api/GroupAPI.h"
#include "utils/json/Json.h"

namespace Rpc
{
    using SDK::Json;

    static Json buildMemberJson(const CoreAPI::GroupMember& m)
    {
        Json j;
        j["name"] = m.name;
        j["level"] = m.level;
        j["race"] = m.race;
        j["class"] = m.class_;
        j["health"] = m.health;
        j["maxHealth"] = m.healthMax;
        j["power"] = m.power;
        j["powerMax"] = m.powerMax;
        j["targetGuid"] = Json::array({m.targetGuid.high, m.targetGuid.low});
        j["inCombat"] = m.isInCombat;
        j["dead"] = m.isDead;
        j["ghost"] = m.isGhost;
        j["casting"] = m.isCasting;
        j["channeling"] = m.isChanneling;
        j["autoAttacking"] = m.isAutoAttacking;
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

            j["leaderGuid"] = Json::array({group.leaderGuid.high, group.leaderGuid.low});
            j["memberCount"] = static_cast<int>(group.members.size());

            Json members = Json::array();
            for (const auto& m : group.members)
                members.push_back(buildMemberJson(m));
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

    void registerGroupMethods(Runtime::MethodRegistry& registry)
    {
        registry.registerMethod("group.get", handleGet);
        registry.registerMethod("group.combatSummary", handleCombatSummary);
    }
}
