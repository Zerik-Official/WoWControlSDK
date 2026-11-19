#include "VTableHook.h"
#include <Windows.h>

namespace Hooks::Detail
{

static void** vptrOf(void* object)
{
    return *reinterpret_cast<void***>(object);
}

bool VTableHook::attach(void* object, const void* expectedVTable, size_t slotCount)
{
    if (!object || slotCount == 0) return false;

    void** current = vptrOf(object);
    if (m_current && current == m_current) return true;
    if (current != expectedVTable) return false;

    auto copy = std::make_unique<void*[]>(slotCount + 1);
    for (size_t i = 0; i <= slotCount; ++i)
        copy[i] = current[static_cast<ptrdiff_t>(i) - 1];
    for (const auto& [slot, function] : m_replacements)
        if (slot < slotCount)
            copy[slot + 1] = function;

    m_originalVTable = current;
    m_current = copy.get() + 1;
    m_object = object;
    m_copies.push_back(std::move(copy));
    InterlockedExchangePointer(reinterpret_cast<void**>(object), m_current);
    return true;
}

void VTableHook::detach()
{
    if (m_object && vptrOf(m_object) == m_current)
        InterlockedExchangePointer(reinterpret_cast<void**>(m_object), m_originalVTable);
    m_object = nullptr;
    m_current = nullptr;
}

bool VTableHook::replace(size_t slot, void* function)
{
    m_replacements.emplace_back(slot, function);
    return true;
}

void* VTableHook::original(size_t slot) const
{
    return m_originalVTable ? m_originalVTable[slot] : nullptr;
}

bool VTableHook::attachedTo(const void* object) const
{
    return object && object == m_object;
}

}
