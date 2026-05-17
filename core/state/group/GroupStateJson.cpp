#include "GroupStateJson.h"
#include "GroupState.h"
#include "GroupCombatState.h"
#include <string>
#include <cstdio>

static const char* groupTypeName(GroupState::GroupType t)
{
    switch (t)
    {
    case GroupState::GroupType::PARTY: return "party";
    case GroupState::GroupType::RAID:  return "raid";
    default:                           return "none";
    }
}

static const char* powerTypeNameForInt(int t)
{
    switch (t)
    {
    case 0: return "mana";
    case 1: return "rage";
    case 3: return "energy";
    case 6: return "runicPower";
    default: return "unknown";
    }
}

static std::string boolStr(bool v)
{
    return v ? "true" : "false";
}

namespace GroupStateJson
{

char* toJson()
{
    GroupState::Info group = GroupState::read();

    if (!group.valid)
    {
        const char* empty =
            "{\"ok\":true,\"groupType\":\"none\",\"memberCount\":0,\"members\":[],"
            "\"combat\":{\"anyMemberInCombat\":false,\"anyMemberDead\":false,"
            "\"anyMemberCasting\":false,\"anyMemberEngaging\":false,"
            "\"membersInCombat\":0,\"membersDead\":0}}";

        size_t len = strlen(empty) + 1;
        char*  buf = new char[len];
        memcpy(buf, empty, len);
        return buf;
    }

    GroupCombatState::Info combat = GroupCombatState::evaluate(group);

    std::string members = "[";
    bool first = true;

    for (const GroupMemberState::Info& m : group.members)
    {
        if (!m.valid) continue;

        char entry[512];
        snprintf(entry, sizeof(entry),
            "%s{"
            "\"name\":\"%s\","
            "\"level\":%d,"
            "\"race\":%d,\"class\":%d,\"gender\":%d,"
            "\"health\":%d,\"healthMax\":%d,"
            "\"powerType\":\"%s\","
            "\"power\":%d,\"powerMax\":%d,"
            "\"isInCombat\":%s,"
            "\"isDead\":%s,"
            "\"isCasting\":%s,"
            "\"isChanneling\":%s,"
            "\"isAutoAttacking\":%s"
            "}",
            first ? "" : ",",
            m.name,
            m.level,
            m.race, m.class_, m.gender,
            m.health, m.healthMax,
            powerTypeNameForInt(m.powerType),
            m.power, m.powerMax,
            boolStr(m.isInCombat).c_str(),
            boolStr(m.isDead).c_str(),
            boolStr(m.isCasting).c_str(),
            boolStr(m.isChanneling).c_str(),
            boolStr(m.isAutoAttacking).c_str()
        );

        members += entry;
        first = false;
    }

    members += "]";

    char combat_buf[256];
    snprintf(combat_buf, sizeof(combat_buf),
        "{\"anyMemberInCombat\":%s,\"anyMemberDead\":%s,"
        "\"anyMemberCasting\":%s,\"anyMemberEngaging\":%s,"
        "\"membersInCombat\":%d,\"membersDead\":%d}",
        boolStr(combat.anyMemberInCombat).c_str(),
        boolStr(combat.anyMemberDead).c_str(),
        boolStr(combat.anyMemberCasting).c_str(),
        boolStr(combat.anyMemberEngaging).c_str(),
        combat.membersInCombat,
        combat.membersDead
    );

    std::string result;
    result.reserve(512 + members.size());
    result += "{\"ok\":true,\"groupType\":\"";
    result += groupTypeName(group.type);
    result += "\",\"memberCount\":";
    result += std::to_string((int)group.members.size());
    result += ",\"members\":";
    result += members;
    result += ",\"combat\":";
    result += combat_buf;
    result += "}";

    size_t len = result.size() + 1;
    char*  buf = new char[len];
    memcpy(buf, result.c_str(), len);
    return buf;
}

}