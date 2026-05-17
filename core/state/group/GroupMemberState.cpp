#include "GroupMemberState.h"
#include "core/api/unit/Unit.h"
#include "core/api/object/ObjectManager.h"
#include <cstring>

namespace GroupMemberState {

Info read(uint64_t guid)
{
    Info info = {};
    info.valid = false;
    info.guid  = guid;

    if (!guid) return info;

    WoWGUID wowGuid;
    wowGuid.low  = static_cast<uint32_t>(guid & 0xFFFFFFFF);
    wowGuid.high = static_cast<uint32_t>(guid >> 32);

    uintptr_t obj = WoW::GetObjectByGUID(wowGuid);
    if (!obj) return info;

    Unit unit(obj);
    if (!unit.exists()) return info;

    info.objectBase = unit.getBase();

    std::string name = unit.getName();
    strncpy_s(info.name, sizeof(info.name), name.empty() ? "unknown" : name.c_str(), _TRUNCATE);

    info.level     = unit.getLevel();
    info.race      = unit.getRace();
    info.class_    = unit.getClass();
    info.gender    = unit.getGender();
    info.powerType = static_cast<int>(unit.getPowerType());
    info.health    = unit.getHealth();
    info.healthMax = unit.getMaxHealth();
    info.power     = unit.getPower();
    info.powerMax  = unit.getMaxPower();

    info.targetGuid = unit.getTargetGUID().raw();

    info.isInCombat      = unit.isInCombat();
    info.isDead          = unit.isDead();
    info.isCasting       = unit.isCasting();
    info.isChanneling    = unit.isChanneling();
    info.isAutoAttacking = unit.isAutoAttacking();

    info.valid = true;
    return info;
}

}