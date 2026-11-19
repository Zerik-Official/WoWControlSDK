#pragma once

#include "core/api/UnitHandle.h"
#include "core/api/UnitAPI.h"

namespace CoreAPI
{
    class PlayerRef : public UnitRef
    {
    public:
        explicit PlayerRef(UnitHandle handle);

        float getRotation()   const;
        int   getXP()         const;
        int   getMaxXP()      const;

        bool  isGhost()       const;
        bool  isUnderwater()  const;
        bool  isAfk()         const;
        bool  isDnd()         const;

        Position    getPosition()   const;

        UnitRef getTarget()   const;
    };

    namespace Player
    {
        PlayerRef   GetLocalPlayer();
        UnitRef     GetTarget();

        bool        IsInCombat();
        bool        IsDead();
        bool        IsGhost();
        bool        IsMounted();
        bool        IsFlying();
        bool        IsSwimming();
        bool        IsUnderwater();
        bool        IsAfk();
        bool        IsDnd();
        bool        IsCasting();

        int         GetHealth();
        int         GetMaxHealth();
        int         GetPower();
        int         GetMaxPower();
        int         GetLevel();
        int         GetRace();
        int         GetClass();
        int         GetXP();
        int         GetMaxXP();

        Position    GetPosition();
        float       GetRotation();

        WoWGUID     GetGUID();
        WoWGUID     GetTargetGUID();

        std::string GetName();
    }
}
