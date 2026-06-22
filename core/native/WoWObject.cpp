#include "core/native/WoWObject.h"
#include "memory/MemReader.h"
#include "offsets/OffsetsUnit.h"

WoWObject::WoWObject(uintptr_t base)
    : m_base(base)
{
    if (base)
        m_objectData = Memory::safeRead<uintptr_t>(base + Offsets::Object::OBJECT_DATA_PTR);
}

bool WoWObject::isValid() const
{
    return m_base != 0 && m_objectData != 0;
}

uintptr_t WoWObject::getBase() const
{
    return m_base;
}

WoWGUID WoWObject::getGUID() const
{
    return WoWGUID{
        Memory::safeRead<uint32_t>(m_base + Offsets::Object::GUID_LOW),
        Memory::safeRead<uint32_t>(m_base + Offsets::Object::GUID_HIGH)
    };
}

uint32_t WoWObject::getTypeMask() const
{
    if (!m_objectData) return 0;
    return Memory::safeRead<uint32_t>(m_objectData + Offsets::Object::OBJECT_DATA_MASK);
}

bool WoWObject::isUnit() const
{
    uint32_t mask = getTypeMask();
    return (mask & static_cast<uint32_t>(WoW::ObjectType::Unit))   != 0
        || (mask & static_cast<uint32_t>(WoW::ObjectType::Player)) != 0;
}

bool WoWObject::isPlayer() const
{
    return (getTypeMask() & static_cast<uint32_t>(WoW::ObjectType::Player)) != 0;
}
