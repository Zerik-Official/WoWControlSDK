#include "Unit.h"
#include "memory/MemReader.h"
#include "OffsetsUnit.h"
#include "OffsetsObjectMgr.h"
#include "internal/UnitResolver.h"
#include "object/ObjectManager.h"
#include "internal/NameResolver.h"

Unit::Unit(uintptr_t base)
    : m_base(base)
{
    if (base)
        m_descBase = Memory::safeRead<uintptr_t>(base + Offsets::Unit::DESCRIPTOR_BASE);
}

uintptr_t Unit::unitDesc() const
{
    return m_descBase + Offsets::ObjectMgr::OBJ_DESC_END;
}

uint32_t Unit::readRCGP() const
{
    return Memory::safeRead<uint32_t>(unitDesc() + Offsets::Unit::Desc::RACE_CLASS_GENDER_POWERTYPE);
}

Unit Unit::fromToken(const char* token)
{
    WoWGUID guid;

    if (!WoW::ResolveUnitToken(token, &guid))
        return Unit(0);

    return Unit(WoW::GetObjectByGUID(guid));
}

bool Unit::isValid() const
{
    return m_base != 0 && m_descBase != 0;
}

bool Unit::exists() const
{
    return isValid() && getGUID().isValid();
}

uintptr_t Unit::getBase() const
{
    return m_base;
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
    return Memory::safeRead<int>(unitDesc() + Offsets::Unit::Desc::HEALTH);
}

int Unit::getMaxHealth() const
{
    return Memory::safeRead<int>(unitDesc() + Offsets::Unit::Desc::MAX_HEALTH);
}

int Unit::getMana() const
{
    return Memory::safeRead<int>(m_base + Offsets::Unit::MANA);
}

int Unit::getMaxMana() const
{
    return Memory::safeRead<int>(m_base + Offsets::Unit::MANA_MAX);
}

int Unit::getPower() const
{
    uint8_t pt = getPowerType();
    switch (pt)
    {
    case 0: return Memory::safeRead<int>(unitDesc() + Offsets::Unit::Desc::POWER1);
    case 1: return Memory::safeRead<int>(unitDesc() + Offsets::Unit::Desc::POWER2) / 10;
    case 3: return Memory::safeRead<int>(unitDesc() + Offsets::Unit::Desc::POWER4);
    case 6: return Memory::safeRead<int>(unitDesc() + Offsets::Unit::Desc::POWER7) / 10;
    default: return 0;
    }
}

int Unit::getMaxPower() const
{
    uint8_t pt = getPowerType();
    switch (pt)
    {
    case 0: return Memory::safeRead<int>(unitDesc() + Offsets::Unit::Desc::MAX_POWER1);
    case 1: return Memory::safeRead<int>(unitDesc() + Offsets::Unit::Desc::MAX_POWER2) / 10;
    case 3: return Memory::safeRead<int>(unitDesc() + Offsets::Unit::Desc::MAX_POWER4);
    case 6: return Memory::safeRead<int>(unitDesc() + Offsets::Unit::Desc::MAX_POWER7) / 10;
    default: return 0;
    }
}

int Unit::getLevel() const
{
    return Memory::safeRead<int>(unitDesc() + Offsets::Unit::Desc::LEVEL_DESC);
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

uint8_t Unit::getPowerType() const
{
    return (readRCGP() >> 24) & 0xFF;
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

WoWGUID Unit::getGUID() const
{
    return Memory::safeRead<WoWGUID>(m_descBase);
}

WoWGUID Unit::getTargetGUID() const
{
    return Memory::safeRead<WoWGUID>(m_descBase + Offsets::Unit::Desc::TARGET_GUID);
}

bool Unit::isInCombat() const
{
    uint32_t flags = Memory::safeRead<uint32_t>(unitDesc() + Offsets::Unit::Desc::FLAGS);
    return (flags & Offsets::Unit::Flags::COMBAT) != 0;
}

bool Unit::isDead() const
{
    uint32_t dynFlags = Memory::safeRead<uint32_t>(unitDesc() + Offsets::Unit::Desc::DYNAMIC_FLAGS);
    return getHealth() == 0 || (dynFlags & Offsets::Unit::Flags::DYNFLAG_DEAD) != 0;
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

int Unit::getCastingSpellId() const
{
    return Memory::safeRead<int>(m_base + Offsets::Unit::CASTING_SPELL_ID);
}

int Unit::getChannelingSpellId() const
{
    return Memory::safeRead<int>(m_base + Offsets::Unit::CHANNELING_SPELL_ID);
}