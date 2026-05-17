#include "Unit.h"
#include "memory/MemReader.h"
#include "OffsetsUnit.h"
#include "OffsetsObjectMgr.h"

Unit::Unit(uintptr_t base)
    : m_base(base)
{
    if (base)
        m_descBase = Memory::safeRead<uintptr_t>(base + Offsets::Unit::DESCRIPTOR_BASE);
}

bool Unit::isValid() const
{
    return m_base != 0 && m_descBase != 0;
}

int Unit::getHealth() const
{
    return Memory::safeRead<int>(m_base + Offsets::Unit::HEALTH);
}

int Unit::getMaxHealth() const
{
    return Memory::safeRead<int>(m_base + Offsets::Unit::HEALTH_MAX);
}

int Unit::getMana() const
{
    return Memory::safeRead<int>(m_base + Offsets::Unit::MANA);
}

int Unit::getMaxMana() const
{
    return Memory::safeRead<int>(m_base + Offsets::Unit::MANA_MAX);
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
    uint32_t flags = Memory::safeRead<uint32_t>(m_descBase + Offsets::Unit::Desc::FLAGS);
    return (flags & Offsets::Unit::Flags::COMBAT) != 0;
}

bool Unit::isDead() const
{
    uint32_t dynFlags = Memory::safeRead<uint32_t>(m_descBase + Offsets::Unit::Desc::DYNAMIC_FLAGS);
    return (dynFlags & Offsets::Unit::Flags::DYNFLAG_DEAD) != 0;
}

bool Unit::isCasting() const
{
    return getCastingSpellId() != 0 || getChannelingSpellId() != 0;
}

int Unit::getCastingSpellId() const
{
    return Memory::safeRead<int>(m_base + Offsets::Unit::CASTING_SPELL_ID);
}

int Unit::getChannelingSpellId() const
{
    return Memory::safeRead<int>(m_base + Offsets::Unit::CHANNELING_SPELL_ID);
}