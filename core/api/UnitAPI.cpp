#include "core/api/UnitAPI.h"
#include "core/native/Unit.h"
#include "core/native/Player.h"
#include "runtime/resolvers/UnitResolver.h"
#include "core/native/ObjectManager.h"

namespace
{
    ::Unit resolve(CoreAPI::UnitHandle handle)
    {
        return ::Unit(handle.base);
    }
}

namespace CoreAPI
{
    UnitRef::UnitRef(UnitHandle handle)
        : m_handle(handle)
    {
    }

    UnitHandle  UnitRef::getHandle()       const { return m_handle; }

    bool        UnitRef::exists()          const { return Unit::Exists(m_handle); }
    bool        UnitRef::isDead()          const { return Unit::IsDead(m_handle); }
    bool        UnitRef::isGhost()         const { return Unit::IsGhost(m_handle); }
    bool        UnitRef::isInCombat()      const { return Unit::IsInCombat(m_handle); }
    bool        UnitRef::isMoving()        const { return Unit::IsMoving(m_handle); }
    bool        UnitRef::isMounted()       const { return Unit::IsMounted(m_handle); }
    bool        UnitRef::isFlying()        const { return Unit::IsFlying(m_handle); }
    bool        UnitRef::isSwimming()      const { return Unit::IsSwimming(m_handle); }
    bool        UnitRef::isCasting()       const { return Unit::IsCasting(m_handle); }
    bool        UnitRef::isChanneling()    const { return Unit::IsChanneling(m_handle); }
    bool        UnitRef::isAutoAttacking() const { return Unit::IsAutoAttacking(m_handle); }

    int         UnitRef::getHealth()       const { return Unit::GetHealth(m_handle); }
    int         UnitRef::getMaxHealth()    const { return Unit::GetMaxHealth(m_handle); }
    int         UnitRef::getPower()        const { return Unit::GetPower(m_handle); }
    int         UnitRef::getMaxPower()     const { return Unit::GetMaxPower(m_handle); }
    int         UnitRef::getLevel()        const { return Unit::GetLevel(m_handle); }
    int         UnitRef::getRace()         const { return Unit::GetRace(m_handle); }
    int         UnitRef::getClass()        const { return Unit::GetClass(m_handle); }

    Position    UnitRef::getPosition()     const { return Unit::GetPosition(m_handle); }

    WoWGUID     UnitRef::getGUID()         const { return Unit::GetGUID(m_handle); }
    WoWGUID     UnitRef::getTargetGUID()   const { return Unit::GetTargetGUID(m_handle); }

    std::string UnitRef::getName()         const { return Unit::GetName(m_handle); }

    namespace Unit
    {
        UnitRef Get(const char* token)
        {
            WoWGUID guid;
            if (!WoW::ResolveUnitToken(token, &guid))
                return UnitRef(NullHandle());

            uintptr_t base = WoW::GetObjectByGUID(guid);
            if (!base)
                return UnitRef(NullHandle());

            return UnitRef(UnitHandle{ base, guid });
        }

        bool Exists(UnitHandle handle)
        {
            if (handle.isNull()) return false;
            return resolve(handle).exists();
        }

        bool IsDead(UnitHandle handle)
        {
            if (handle.isNull()) return true;
            return resolve(handle).isDead();
        }

        bool IsGhost(UnitHandle handle)
        {
            if (handle.isNull()) return false;
            return resolve(handle).isGhost();
        }

        bool IsInCombat(UnitHandle handle)
        {
            if (handle.isNull()) return false;
            return resolve(handle).isInCombat();
        }

        bool IsMoving(UnitHandle handle)
        {
            if (handle.isNull()) return false;
            return resolve(handle).isMoving();
        }

        bool IsMounted(UnitHandle handle)
        {
            if (handle.isNull()) return false;
            return resolve(handle).isMounted();
        }

        bool IsFlying(UnitHandle handle)
        {
            if (handle.isNull()) return false;
            return resolve(handle).isFlying();
        }

        bool IsSwimming(UnitHandle handle)
        {
            if (handle.isNull()) return false;
            return resolve(handle).isSwimming();
        }

        bool IsCasting(UnitHandle handle)
        {
            if (handle.isNull()) return false;
            return resolve(handle).isCasting();
        }

        bool IsChanneling(UnitHandle handle)
        {
            if (handle.isNull()) return false;
            return resolve(handle).isChanneling();
        }

        bool IsAutoAttacking(UnitHandle handle)
        {
            if (handle.isNull()) return false;
            return resolve(handle).isAutoAttacking();
        }

        int GetHealth(UnitHandle handle)
        {
            if (handle.isNull()) return 0;
            return resolve(handle).getHealth();
        }

        int GetMaxHealth(UnitHandle handle)
        {
            if (handle.isNull()) return 0;
            return resolve(handle).getMaxHealth();
        }

        int GetPower(UnitHandle handle)
        {
            if (handle.isNull()) return 0;
            return resolve(handle).getPower();
        }

        int GetMaxPower(UnitHandle handle)
        {
            if (handle.isNull()) return 0;
            return resolve(handle).getMaxPower();
        }

        int GetLevel(UnitHandle handle)
        {
            if (handle.isNull()) return 0;
            return resolve(handle).getLevel();
        }

        int GetRace(UnitHandle handle)
        {
            if (handle.isNull()) return 0;
            return resolve(handle).getRace();
        }

        int GetClass(UnitHandle handle)
        {
            if (handle.isNull()) return 0;
            return resolve(handle).getClass();
        }

        Position GetPosition(UnitHandle handle)
        {
            if (handle.isNull()) return { 0.f, 0.f, 0.f };
            ::Player local = ::Player::local();
            if (local.exists() && local.getBase() == handle.base)
                return { local.getX(), local.getY(), local.getZ() };
            return { 0.f, 0.f, 0.f };
        }

        WoWGUID GetGUID(UnitHandle handle)
        {
            if (handle.isNull()) return NULL_GUID;
            return handle.guid;
        }

        WoWGUID GetTargetGUID(UnitHandle handle)
        {
            if (handle.isNull()) return NULL_GUID;
            return resolve(handle).getTargetGUID();
        }

        std::string GetName(UnitHandle handle)
        {
            if (handle.isNull()) return {};
            return resolve(handle).getName();
        }

        Position GetUnitMapPosition(UnitHandle handle)
        {
            if (handle.isNull()) return { 0.f, 0.f, 0.f };
            float x = 0.f, y = 0.f;
            resolve(handle).getMapPosition(&x, &y);
            return { x, y, 0.f };
        }

        Position GetUnitWorldPosition(UnitHandle handle)
        {
            if (handle.isNull()) return { 0.f, 0.f, 0.f };
            float x = 0.f, y = 0.f, z = 0.f;
            resolve(handle).getWorldPosition(&x, &y, &z);
            return { x, y, z };
        }
    }
}
