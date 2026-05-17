#include "DebugCommands.h"

#include "hooks/EndSceneHook.h"

#include "core/api/internal/UnitResolver.h"
#include "core/api/unit/Unit.h"
#include "core/api/object/ObjectManager.h"
#include "memory/MemReader.h"

#include <cstdio>

namespace DebugCommands
{

std::string testUnitPointers()
{
    return EndSceneHook::dispatch([]() -> std::string {

        WoWGUID guid;

        if (!WoW::ResolveUnitToken("player", &guid))
            return "{\"ok\":false,\"error\":\"failed to resolve player\"}";

        uintptr_t base = WoW::GetObjectByGUID(guid);

        if (!base)
            return "{\"ok\":false,\"error\":\"GetObjectByGUID failed\"}";

        Unit unit(base);

        uintptr_t objectData =
            Memory::safeRead<uintptr_t>(base + 0x8);

        uintptr_t unitData =
            Memory::safeRead<uintptr_t>(base + 0xD0);

        uintptr_t oldUnitDesc =
            objectData + 0x18;

        uint32_t typeMask =
            Memory::safeRead<uint32_t>(objectData + 0x8);

        int hpOld =
            Memory::safeRead<int>(oldUnitDesc + 0x48);

        int hpNew =
            Memory::safeRead<int>(unitData + 0x48);

        char buffer[1024];

        sprintf_s(
            buffer,
            "{"
            "\"ok\":true,"
            "\"base\":\"0x%p\","
            "\"objectData\":\"0x%p\","
            "\"oldUnitDesc\":\"0x%p\","
            "\"unitData\":\"0x%p\","
            "\"typeMask\":%u,"
            "\"hpOld\":%d,"
            "\"hpNew\":%d"
            "}",
            (void*)base,
            (void*)objectData,
            (void*)oldUnitDesc,
            (void*)unitData,
            typeMask,
            hpOld,
            hpNew
        );

        return std::string(buffer);
    });
}

std::string testUnitStates()
{
    return EndSceneHook::dispatch([]() -> std::string {

        Unit player = Unit::fromToken("player");
        Unit target = Unit::fromToken("target");

        char buf[2048];

        sprintf_s(buf, sizeof(buf),
            "{"
            "\"ok\":true,"
            "\"player\":{"
            "\"exists\":%s,"
            "\"isPlayer\":%s,"
            "\"isUnit\":%s,"
            "\"isDead\":%s,"
            "\"isInCombat\":%s,"
            "\"hasTarget\":%s,"
            "\"isMoving\":%s,"
            "\"isSwimming\":%s,"
            "\"isFlying\":%s,"
            "\"isMounted\":%s"
            "},"
            "\"target\":{"
            "\"exists\":%s,"
            "\"isPlayer\":%s,"
            "\"isUnit\":%s,"
            "\"isDead\":%s,"
            "\"isInCombat\":%s,"
            "\"isSameAsPlayer\":%s"
            "}"
            "}",
            player.exists()      ? "true" : "false",
            player.isPlayer()    ? "true" : "false",
            player.isUnit()      ? "true" : "false",
            player.isDead()      ? "true" : "false",
            player.isInCombat()  ? "true" : "false",
            player.hasTarget()   ? "true" : "false",
            player.isMoving()    ? "true" : "false",
            player.isSwimming()  ? "true" : "false",
            player.isFlying()    ? "true" : "false",
            player.isMounted()   ? "true" : "false",
            target.exists()      ? "true" : "false",
            target.isPlayer()    ? "true" : "false",
            target.isUnit()      ? "true" : "false",
            target.isDead()      ? "true" : "false",
            target.isInCombat()  ? "true" : "false",
            target.isSameUnit(player) ? "true" : "false"
        );

        return std::string(buf);
    });
}

std::string handle(const std::string& cmd)
{
    if (cmd == "testUnitPointers")
        return testUnitPointers();

    if (cmd == "testUnitStates")
        return testUnitStates();

    return "{\"ok\":false,\"error\":\"unknown debug command\"}";
}

}