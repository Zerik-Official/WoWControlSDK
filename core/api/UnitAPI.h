#pragma once

#include "core/api/UnitHandle.h"
#include <string>

namespace CoreAPI
{
    struct Position
    {
        float x;
        float y;
        float z;
    };

    class UnitRef
    {
    public:
        explicit UnitRef(UnitHandle handle);

        UnitHandle  getHandle()     const;

        bool        exists()        const;
        bool        isDead()        const;
        bool        isGhost()       const;
        bool        isInCombat()    const;
        bool        isMoving()      const;
        bool        isMounted()     const;
        bool        isFlying()      const;
        bool        isSwimming()    const;
        bool        isCasting()     const;
        bool        isChanneling()  const;
        bool        isAutoAttacking() const;

        int         getHealth()     const;
        int         getMaxHealth()  const;
        int         getPower()      const;
        int         getMaxPower()   const;
        int         getLevel()      const;
        int         getRace()       const;
        int         getClass()      const;

        Position    getPosition()       const;
        Position    getMapPosition()    const;
        Position    getWorldPosition()  const;
        float       getRotation()       const;

        WoWGUID     getGUID()           const;
        WoWGUID     getTargetGUID() const;

        std::string getName()       const;

    private:
        UnitHandle m_handle;
    };

    namespace Unit
    {
        UnitRef     Get(const char* token);

        bool        Exists(UnitHandle handle);
        bool        IsDead(UnitHandle handle);
        bool        IsGhost(UnitHandle handle);
        bool        IsInCombat(UnitHandle handle);
        bool        IsMoving(UnitHandle handle);
        bool        IsMounted(UnitHandle handle);
        bool        IsFlying(UnitHandle handle);
        bool        IsSwimming(UnitHandle handle);
        bool        IsCasting(UnitHandle handle);
        bool        IsChanneling(UnitHandle handle);
        bool        IsAutoAttacking(UnitHandle handle);

        int         GetHealth(UnitHandle handle);
        int         GetMaxHealth(UnitHandle handle);
        int         GetPower(UnitHandle handle);
        int         GetMaxPower(UnitHandle handle);
        int         GetLevel(UnitHandle handle);
        int         GetRace(UnitHandle handle);
        int         GetClass(UnitHandle handle);

        Position    GetPosition(UnitHandle handle);

        WoWGUID     GetGUID(UnitHandle handle);
        WoWGUID     GetTargetGUID(UnitHandle handle);

        std::string GetName(UnitHandle handle);

        float       GetRotation(UnitHandle handle);

        Position    GetUnitMapPosition(UnitHandle handle);
        Position    GetUnitWorldPosition(UnitHandle handle);
    }
}
