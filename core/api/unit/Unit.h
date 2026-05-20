#pragma once

#include "object/WoWObject.h"
#include "types/PowerType.h"
#include <cstdint>
#include <string>

class Unit : public WoWObject
{
public:
    explicit Unit(uintptr_t base);

    static Unit fromToken(const char* token);

    bool isValid() const;
    bool exists() const;

    int   getHealth()    const;
    int   getMaxHealth() const;
    int   getPower()     const;
    int   getMaxPower()  const;

    int            getLevel()     const;
    int            getRace()      const;
    int            getClass()     const;
    int            getGender()    const;
    WoW::PowerType getPowerType() const;

    WoWGUID     getTargetGUID() const;
    Unit        getTarget()     const;
    bool        hasTarget()     const;
    bool        isSameUnit(const Unit& other) const;

    bool        getWorldPosition(float* outX, float* outY) const;
    bool        getWorldPosition(float* outX, float* outY, float* outZ) const;
    bool        getMapPosition(float* outX, float* outY) const;

    std::string getName() const;

    bool isInCombat()      const;
    bool isDead()          const;
    bool isCasting()       const;
    bool isChanneling()    const;
    bool isAutoAttacking() const;
    bool isMoving()        const;
    bool isSwimming()      const;
    bool isFlying()        const;
    bool isMounted()       const;

    int  getCastingSpellId()    const;
    int  getChannelingSpellId() const;

protected:
    uintptr_t m_unitData = 0;

private:
    uint32_t  readRCGP() const;
};