#include "Unit.h"
#include "memory/MemReader.h"
#include "OffsetsUnit.h"
#include "OffsetsObjectMgr.h"
#include "internal/UnitResolver.h"
#include "object/ObjectManager.h"
#include "internal/NameResolver.h"

Unit::Unit(uintptr_t base)
    : WoWObject(base)
{
    if (m_objectData)
        m_unitData = m_objectData + Offsets::Object::UNIT_DATA_OFFSET;
}

bool Unit::isValid() const
{
    return m_base != 0 && m_objectData != 0 && m_unitData != 0;
}

uint32_t Unit::readRCGP() const
{
    return Memory::safeRead<uint32_t>(m_unitData + Offsets::Unit::Desc::RACE_CLASS_GENDER_POWERTYPE);
}

Unit Unit::fromToken(const char* token)
{
    WoWGUID guid;

    if (!WoW::ResolveUnitToken(token, &guid))
        return Unit(0);

    return Unit(WoW::GetObjectByGUID(guid));
}

bool Unit::exists() const
{
    return isValid() && getGUID().isValid() && isUnit();
}

std::string Unit::getName() const
{
    char buf[64] = {};

    if (!isValid())
        return "";

    if (!NameResolver::readName(getGUID().raw(), buf, sizeof(buf)))
        return "";

    return buf;
}

int Unit::getHealth() const
{
    return Memory::safeRead<int>(m_unitData + Offsets::Unit::Desc::HEALTH);
}

int Unit::getMaxHealth() const
{
    return Memory::safeRead<int>(m_unitData + Offsets::Unit::Desc::MAX_HEALTH);
}

int Unit::getPower() const
{
    switch (getPowerType())
    {
    case WoW::PowerType::Mana:       return Memory::safeRead<int>(m_unitData + Offsets::Unit::Desc::POWER1);
    case WoW::PowerType::Rage:       return Memory::safeRead<int>(m_unitData + Offsets::Unit::Desc::POWER2) / 10;
    case WoW::PowerType::Energy:     return Memory::safeRead<int>(m_unitData + Offsets::Unit::Desc::POWER4);
    case WoW::PowerType::RunicPower: return Memory::safeRead<int>(m_unitData + Offsets::Unit::Desc::POWER7) / 10;
    default:                         return 0;
    }
}

int Unit::getMaxPower() const
{
    switch (getPowerType())
    {
    case WoW::PowerType::Mana:       return Memory::safeRead<int>(m_unitData + Offsets::Unit::Desc::MAX_POWER1);
    case WoW::PowerType::Rage:       return Memory::safeRead<int>(m_unitData + Offsets::Unit::Desc::MAX_POWER2) / 10;
    case WoW::PowerType::Energy:     return Memory::safeRead<int>(m_unitData + Offsets::Unit::Desc::MAX_POWER4);
    case WoW::PowerType::RunicPower: return Memory::safeRead<int>(m_unitData + Offsets::Unit::Desc::MAX_POWER7) / 10;
    default:                         return 0;
    }
}

int Unit::getLevel() const
{
    return Memory::safeRead<int>(m_unitData + Offsets::Unit::Desc::LEVEL_DESC);
}

int Unit::getRace() const
{
    return (readRCGP() >> 0) & 0xFF;
}

int Unit::getClass() const
{
    return (readRCGP() >> 8) & 0xFF;
}

int Unit::getGender() const
{
    return (readRCGP() >> 16) & 0xFF;
}

WoW::PowerType Unit::getPowerType() const
{
    uint8_t raw = (readRCGP() >> 24) & 0xFF;

    switch (raw)
    {
    case 0: return WoW::PowerType::Mana;
    case 1: return WoW::PowerType::Rage;
    case 2: return WoW::PowerType::Focus;
    case 3: return WoW::PowerType::Energy;
    case 4: return WoW::PowerType::Happiness;
    case 5: return WoW::PowerType::Rune;
    case 6: return WoW::PowerType::RunicPower;
    default: return WoW::PowerType::Unknown;
    }
}

float Unit::getX() const
{
    return Memory::safeRead<float>(m_base + Offsets::ObjectMgr::OBJECT_POS_X);
}

float Unit::getY() const
{
    return Memory::safeRead<float>(m_base + Offsets::ObjectMgr::OBJECT_POS_Y);
}

float Unit::getZ() const
{
    return Memory::safeRead<float>(m_base + Offsets::ObjectMgr::OBJECT_POS_Z);
}

WoWGUID Unit::getTargetGUID() const
{
    return Memory::safeRead<WoWGUID>(m_unitData + Offsets::Unit::Desc::TARGET_GUID);
}

Unit Unit::getTarget() const
{
    WoWGUID targetGuid = getTargetGUID();
    if (!targetGuid.isValid())
        return Unit(0);

    return Unit(WoW::GetObjectByGUID(targetGuid));
}

bool Unit::hasTarget() const
{
    return getTargetGUID().isValid();
}

bool Unit::isSameUnit(const Unit& other) const
{
    if (!exists() || !other.exists())
        return false;

    return getGUID() == other.getGUID();
}

bool Unit::isInCombat() const
{
    uint32_t flags = Memory::safeRead<uint32_t>(m_unitData + Offsets::Unit::Desc::FLAGS);
    return (flags & Offsets::Unit::Flags::COMBAT) != 0;
}

bool Unit::isDead() const
{
    if (getHealth() <= 0) return true;
    uint32_t dynFlags = Memory::safeRead<uint32_t>(m_unitData + Offsets::Unit::Desc::DYNAMIC_FLAGS);
    return (dynFlags >> 5) & 1;
}

bool Unit::isCasting() const
{
    return getCastingSpellId() != 0 || getChannelingSpellId() != 0;
}

bool Unit::isChanneling() const
{
    return getChannelingSpellId() != 0;
}

bool Unit::isAutoAttacking() const
{
    return Memory::safeRead<int>(m_base + Offsets::Unit::IS_AUTO_ATTACKING) != 0;
}

bool Unit::isMoving() const
{
    uint32_t flags = Memory::safeRead<uint32_t>(m_base + Offsets::Unit::MOVE_FLAGS);
    return (flags & 0x1FF) != 0;
}

bool Unit::isSwimming() const
{
    uint32_t flags = Memory::safeRead<uint32_t>(m_base + Offsets::Unit::MOVE_FLAGS);
    return (flags & Offsets::Unit::Flags::MOVE_SWIMMING) != 0;
}

bool Unit::isFlying() const
{
    uintptr_t movePtr = Memory::safeRead<uintptr_t>(m_base + Offsets::Unit::MOVE_DATA_PTR);
    if (!movePtr) return false;
    uint32_t flags = Memory::safeRead<uint32_t>(movePtr + Offsets::Unit::MOVE_DATA_FLAGS);
    return (flags & Offsets::Unit::MOVEFLAG_FLYING) != 0;
}

bool Unit::isMounted() const
{
    uint32_t flags = Memory::safeRead<uint32_t>(m_unitData + Offsets::Unit::Desc::FLAGS);
    return (flags & Offsets::Unit::Flags::MOUNTED) != 0;
}

int Unit::getCastingSpellId() const
{
    return Memory::safeRead<int>(m_base + Offsets::Unit::CASTING_SPELL_ID);
}

int Unit::getChannelingSpellId() const
{
    return Memory::safeRead<int>(m_base + Offsets::Unit::CHANNELING_SPELL_ID);
}