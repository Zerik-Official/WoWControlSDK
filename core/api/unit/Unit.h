#pragma once

#include "types/GUID.h"
#include <cstdint>

class Unit
{
public:
    explicit Unit(uintptr_t base);

    bool isValid() const;

    int   getHealth()    const;
    int   getMaxHealth() const;
    int   getMana()      const;
    int   getMaxMana()   const;

    float getX() const;
    float getY() const;
    float getZ() const;

    WoWGUID getGUID()       const;
    WoWGUID getTargetGUID() const;

    bool isInCombat()    const;
    bool isDead()        const;
    bool isCasting()     const;

    int getCastingSpellId()    const;
    int getChannelingSpellId() const;

protected:
    uintptr_t  m_base       = 0;
    uintptr_t  m_descBase   = 0;
};