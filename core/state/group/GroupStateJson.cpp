#include "GroupStateJson.h"
#include "GroupState.h"
#include "GroupCombatState.h"
#include <cstdio>
#include <cstring>

static const char *groupTypeName(GroupState::GroupType t)
{
    switch (t)
    {
    case GroupState::GroupType::PARTY:
        return "party";
    case GroupState::GroupType::RAID:
        return "raid";
    default:
        return "none";
    }
}

static const char *powerTypeNameForInt(int t)
{
    switch (t)
    {
    case 0:
        return "mana";
    case 1:
        return "rage";
    case 3:
        return "energy";
    case 6:
        return "runicPower";
    default:
        return "unknown";
    }
}

namespace GroupStateJson
{

    char *toJson()
    {
        GroupState::Info group = GroupState::read();

        if (!group.valid)
        {
            static const char *empty =
                "{\"ok\":true,\"groupType\":\"none\",\"memberCount\":0,\"members\":[],\"combat\":{\"anyMemberInCombat\":false,\"anyMemberDead\":false,\"anyMemberCasting\":false,\"anyMemberEngaging\":false,\"membersInCombat\":0,\"membersDead\":0}}";

            size_t len = strlen(empty) + 1;
            char *buf = new char[len];
            memcpy(buf, empty, len);
            return buf;
        }

        GroupCombatState::Info combat = GroupCombatState::evaluate(group);

        char membersJson[8192] = "[";
        bool first = true;

        for (const GroupMemberState::Info &m : group.members)
        {
            if (!m.valid)
                continue;

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
                     m.isInCombat ? "true" : "false",
                     m.isDead ? "true" : "false",
                     m.isCasting ? "true" : "false",
                     m.isChanneling ? "true" : "false",
                     m.isAutoAttacking ? "true" : "false");

            strncat_s(membersJson, sizeof(membersJson), entry, _TRUNCATE);
            first = false;
        }

        strncat_s(membersJson, sizeof(membersJson), "]", _TRUNCATE);

        char tmp[10240];
        snprintf(tmp, sizeof(tmp),
                 "{"
                 "\"ok\":true,"
                 "\"groupType\":\"%s\","
                 "\"memberCount\":%d,"
                 "\"members\":%s,"
                 "\"combat\":{"
                 "\"anyMemberInCombat\":%s,"
                 "\"anyMemberDead\":%s,"
                 "\"anyMemberCasting\":%s,"
                 "\"anyMemberEngaging\":%s,"
                 "\"membersInCombat\":%d,"
                 "\"membersDead\":%d"
                 "}"
                 "}",
                 groupTypeName(group.type),
                 (int)group.members.size(),
                 membersJson,
                 combat.anyMemberInCombat ? "true" : "false",
                 combat.anyMemberDead ? "true" : "false",
                 combat.anyMemberCasting ? "true" : "false",
                 combat.anyMemberEngaging ? "true" : "false",
                 combat.membersInCombat,
                 combat.membersDead);

        size_t len = strlen(tmp) + 1;
        char *buf = new char[len];
        memcpy(buf, tmp, len);
        return buf;
    }

}