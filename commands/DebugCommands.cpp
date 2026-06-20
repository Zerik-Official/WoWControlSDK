#include "DebugCommands.h"
#include "utils/json/Json.h"
#include "utils/json/JsonIPC.h"

#include "hooks/FrameHooks.h"

#include "core/api/UnitAPI.h"
#include "core/api/PlayerAPI.h"
#include "core/api/WorldAPI.h"
#include "core/api/GroupAPI.h"
#include "core/api/ObjectAPI.h"
#include "core/api/UnitHandle.h"
#include "runtime/resolvers/NameResolver.h"
#include "core/native/ObjectManager.h"

#include <string>
#include <vector>

namespace
{

std::string testUnitGet()
{
    return Hooks::Frame::Execute([]() -> std::string {
        CoreAPI::UnitRef player = CoreAPI::Unit::Get("player");
        CoreAPI::UnitRef target = CoreAPI::Unit::Get("target");
        CoreAPI::UnitRef mouse  = CoreAPI::Unit::Get("mouseover");

        SDK::Json j;
        j["ok"] = true;
        j["player"] = {{"exists", player.exists()}, {"isNull", player.getHandle().isNull()}};
        j["target"] = {{"exists", target.exists()}, {"isNull", target.getHandle().isNull()}};
        j["mouseover"] = {{"exists", mouse.exists()}, {"isNull", mouse.getHandle().isNull()}};
        return j.dump();
    });
}

std::string testUnitState()
{
    return Hooks::Frame::Execute([]() -> std::string {
        CoreAPI::UnitRef player = CoreAPI::Unit::Get("player");

        SDK::Json j;
        j["ok"] = true;
        j["exists"] = player.exists();
        j["isDead"] = player.isDead();
        j["isInCombat"] = player.isInCombat();
        j["isMoving"] = player.isMoving();
        j["isSwimming"] = player.isSwimming();
        j["isFlying"] = player.isFlying();
        j["isMounted"] = player.isMounted();
        j["isCasting"] = player.isCasting();
        j["isChanneling"] = player.isChanneling();
        j["isAutoAttacking"] = player.isAutoAttacking();
        return j.dump();
    });
}

std::string testUnitStats()
{
    return Hooks::Frame::Execute([]() -> std::string {
        CoreAPI::UnitRef player = CoreAPI::Unit::Get("player");
        CoreAPI::Position world = CoreAPI::Unit::GetUnitWorldPosition(player.getHandle());

        SDK::Json j;
        j["ok"] = true;
        j["health"] = player.getHealth();
        j["maxHealth"] = player.getMaxHealth();
        j["power"] = player.getPower();
        j["maxPower"] = player.getMaxPower();
        j["level"] = player.getLevel();
        j["race"] = player.getRace();
        j["class"] = player.getClass();
        j["worldX"] = world.x;
        j["worldY"] = world.y;
        j["worldZ"] = world.z;
        return j.dump();
    });
}

std::string testUnitTarget()
{
    return Hooks::Frame::Execute([]() -> std::string {
        CoreAPI::UnitRef player = CoreAPI::Unit::Get("player");

        WoWGUID targetGuid = player.getTargetGUID();
        bool hasTarget = targetGuid.isValid();

        char targetName[64] = {};
        if (hasTarget)
            NameResolver::readName(targetGuid.raw(), targetName, sizeof(targetName));

        SDK::Json j;
        j["ok"] = true;
        j["hasTarget"] = hasTarget;
        j["playerGuid"] = SDK::Json::array({player.getGUID().high, player.getGUID().low});
        j["playerTargetGuid"] = SDK::Json::array({targetGuid.high, targetGuid.low});
        j["targetName"] = targetName;
        j["targetIsPlayer"] = targetGuid.isPlayer();
        return j.dump();
    });
}

std::string testMouseover()
{
    return Hooks::Frame::Execute([]() -> std::string {
        WoWGUID moGuid = WoW::GetMouseoverGUID();
        bool hasMouseover = moGuid.isValid();

        char moName[64] = {};
        if (hasMouseover)
            NameResolver::readName(moGuid.raw(), moName, sizeof(moName));

        uintptr_t moBase = hasMouseover ? WoW::GetObjectByGUID(moGuid) : 0;
        bool moExists = moBase != 0;

        SDK::Json j;
        j["ok"] = true;
        j["hasMouseover"] = hasMouseover;
        j["mouseoverGuid"] = SDK::Json::array({moGuid.high, moGuid.low});
        j["mouseoverName"] = moName;
        j["mouseoverInObjectMgr"] = moExists;
        return j.dump();
    });
}

std::string testUnitMapPosition()
{
    return Hooks::Frame::Execute([]() -> std::string {
        CoreAPI::UnitRef player = CoreAPI::Unit::Get("player");
        CoreAPI::Position pos = CoreAPI::Unit::GetUnitMapPosition(player.getHandle());

        SDK::Json j;
        j["ok"] = true;
        j["mapX"] = pos.x;
        j["mapY"] = pos.y;
        return j.dump();
    });
}

std::string testPlayerState()
{
    return Hooks::Frame::Execute([]() -> std::string {
        CoreAPI::PlayerRef p = CoreAPI::Player::GetLocalPlayer();

        WoWGUID targetGuid = p.getTargetGUID();
        bool hasTarget = targetGuid.isValid();
        float rot = p.getRotation();

        SDK::Json j;
        j["ok"] = true;
        j["exists"] = p.exists();
        j["isGhost"] = p.isGhost();
        j["isUnderwater"] = p.isUnderwater();
        j["isAfk"] = p.isAfk();
        j["isDnd"] = p.isDnd();
        j["isInCombat"] = p.isInCombat();
        j["isDead"] = p.isDead();
        j["isMounted"] = p.isMounted();
        j["isFlying"] = p.isFlying();
        j["isSwimming"] = p.isSwimming();
        j["isCasting"] = p.isCasting();
        j["hasTarget"] = hasTarget;
        j["xp"] = p.getXP();
        j["maxXp"] = p.getMaxXP();
        j["health"] = p.getHealth();
        j["maxHealth"] = p.getMaxHealth();
        j["level"] = p.getLevel();
        j["rotation"] = rot;
        return j.dump();
    });
}

std::string testPlayerGet()
{
    return Hooks::Frame::Execute([]() -> std::string {
        CoreAPI::PlayerRef local = CoreAPI::Player::GetLocalPlayer();
        CoreAPI::UnitRef   target = CoreAPI::Player::GetTarget();

        SDK::Json j;
        j["ok"] = true;
        j["localExists"] = local.exists();
        j["localGuid"] = SDK::Json::array({local.getGUID().high, local.getGUID().low});
        j["localName"] = local.getName();
        j["localRace"] = local.getRace();
        j["localClass"] = local.getClass();
        j["targetExists"] = target.exists();
        j["targetName"] = target.getName();
        return j.dump();
    });
}

std::string testWorld()
{
    return Hooks::Frame::Execute([]() -> std::string {
        SDK::Json j;
        j["ok"] = true;
        j["isInWorld"] = CoreAPI::World::IsInWorld();
        j["isLoading"] = CoreAPI::World::IsOnLoadingScreen();
        j["mapId"] = CoreAPI::World::GetMapId();
        j["zoneId"] = CoreAPI::World::GetZoneId();
        j["continentId"] = CoreAPI::World::GetContinentId();
        j["luaMapId"] = CoreAPI::World::GetLuaMapId();
        j["luaZoneId"] = CoreAPI::World::GetLuaZoneId();
        j["zoneName"] = CoreAPI::World::GetZoneName();
        j["subZoneName"] = CoreAPI::World::GetSubZoneName();
        j["tick"] = CoreAPI::World::GetGameTick();
        return j.dump();
    });
}

std::string testObject()
{
    return Hooks::Frame::Execute([]() -> std::string {
        CoreAPI::UnitRef player = CoreAPI::Unit::Get("player");

        bool exists   = CoreAPI::Object::Exists(player.getHandle());
        bool isUnit   = CoreAPI::Object::IsUnit(player.getHandle());
        bool isPlayer = CoreAPI::Object::IsPlayer(player.getHandle());
        auto type     = CoreAPI::Object::GetType(player.getHandle());
        auto guid     = CoreAPI::Object::GetGUID(player.getHandle());

        const char* typeStr = "Object";
        if (type == WoW::ObjectType::Player)      typeStr = "Player";
        else if (type == WoW::ObjectType::Unit)    typeStr = "Unit";
        else if (type == WoW::ObjectType::GameObject) typeStr = "GameObject";

        SDK::Json j;
        j["ok"] = true;
        j["exists"] = exists;
        j["isUnit"] = isUnit;
        j["isPlayer"] = isPlayer;
        j["type"] = typeStr;
        j["guid"] = SDK::Json::array({guid.high, guid.low});
        return j.dump();
    });
}

std::string testGroup()
{
    return Hooks::Frame::Execute([]() -> std::string {
        CoreAPI::GroupInfo group = CoreAPI::Group::GetGroup();

        const char* groupType = "None";
        if (group.type == CoreAPI::GroupType::Party) groupType = "Party";
        else if (group.type == CoreAPI::GroupType::Raid) groupType = "Raid";

        SDK::Json members = SDK::Json::array();
        for (const auto& m : group.members)
        {
            SDK::Json entry;
            entry["name"] = m.name;
            entry["level"] = m.level;
            entry["class"] = m.class_;
            entry["health"] = m.health;
            entry["maxHealth"] = m.healthMax;
            members.push_back(entry);
        }

        SDK::Json j;
        j["ok"] = true;
        j["inGroup"] = CoreAPI::Group::IsInGroup();
        j["inRaid"] = CoreAPI::Group::IsInRaid();
        j["type"] = groupType;
        j["memberCount"] = CoreAPI::Group::GetMemberCount();
        j["leaderGuid"] = SDK::Json::array({group.leaderGuid.high, group.leaderGuid.low});
        j["members"] = members;
        return j.dump();
    });
}

}

namespace DebugCommands
{

std::string handle(const std::string& cmd)
{
    if (cmd == "unitGet")      return testUnitGet();
    if (cmd == "unitState")    return testUnitState();
    if (cmd == "unitStats")    return testUnitStats();
    if (cmd == "unitTarget")   return testUnitTarget();
    if (cmd == "unitMapPos")   return testUnitMapPosition();
    if (cmd == "mouseover")    return testMouseover();
    if (cmd == "playerState")  return testPlayerState();
    if (cmd == "playerGet")    return testPlayerGet();
    if (cmd == "world")        return testWorld();
    if (cmd == "object")       return testObject();
    if (cmd == "group")        return testGroup();

    return SDK::JsonIPC::serializeCommandError("unknown debug command");
}

}