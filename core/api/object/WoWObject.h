#pragma once
#include "ObjectType.h"
#include "types/GUID.h"
#include <cstdint>

class WoWObject
{
public:
    explicit WoWObject(uintptr_t base);

    bool      isValid()    const;
    uintptr_t getBase()    const;
    WoWGUID   getGUID()    const;

    uint32_t        getTypeMask() const;
    bool            isUnit()      const;
    bool            isPlayer()    const;

protected:
    uintptr_t m_base       = 0;
    uintptr_t m_objectData = 0;
};