#pragma once

#include "types/GUID.h"
#include <cstdint>
#include <string>

class Unit
{
public:
    explicit Unit(uintptr_t base);

    static Unit fromToken(const char* token);

    bool isValid() const;
    bool exists()  const;

    uintptr_t getBase() const;

    int   getHealth()    const;
    int   getMaxHealth() const;
    int   getMana()      const;
    int   getMaxMana()   const;
    int   getPower()     const;
    int   getMaxPower()  const;

    int      getLevel()     const;
    int      getRace()      const;
    int      getClass()     const;
    int      getGender()    const;
    uint8_t  getPowerType() const;

    float getX() const;
    float getY() const;
    float getZ() const;

    WoWGUID getGUID()       const;
    WoWGUID getTargetGUID() const;

    std::string getName() const;

    bool isInCombat()     const;
    bool isDead()         const;
    bool isCasting()      const;
    bool isChanneling()   const;
    bool isAutoAttacking() const;

    int getCastingSpellId()    const;
    int getChannelingSpellId() const;

protected:
    uintptr_t m_base     = 0;
    uintptr_t m_descBase = 0;

private:
    uintptr_t unitDesc() const;
    uint32_t  readRCGP() const;
};