#include "core/api/PlayerAPI.h"
#include "core/native/Player.h"
#include "core/native/ObjectManager.h"
#include "memory/MemReader.h"
#include "offsets/OffsetsUnit.h"
#include "offsets/OffsetsMap.h"
#include "offsets/OffsetsPlayer.h"

namespace
{
    ::Player resolveLocal()
    {
        return ::Player::local();
    }

    CoreAPI::UnitHandle localHandle()
    {
        ::Player p = resolveLocal();
        if (!p.exists()) return CoreAPI::NullHandle();
        return CoreAPI::UnitHandle{ p.getBase(), p.getGUID() };
    }

    bool readIsGhost(uintptr_t base)
    {
        return ::Unit(base).hasAura(Offsets::Player::GHOST_SPELL_ID);
    }

    bool readIsUnderwater()
    {
        return Memory::safeRead<int>(Offsets::Player::BREATH_TIMER) > 0;
    }

    bool readIsAfk(uintptr_t base)
    {
        uintptr_t presencePtr = Memory::safeRead<uintptr_t>(base + 0x1008);
        if (!presencePtr) return false;
        uint32_t flags = Memory::safeRead<uint32_t>(presencePtr + 0x8);
        return (flags & (1 << 1)) != 0;
    }

    bool readIsDnd(uintptr_t base)
    {
        uintptr_t presencePtr = Memory::safeRead<uintptr_t>(base + 0x1008);
        if (!presencePtr) return false;
        uint32_t flags = Memory::safeRead<uint32_t>(presencePtr + 0x8);
        return (flags & (1 << 2)) != 0;
    }
}

namespace CoreAPI
{
    PlayerRef::PlayerRef(UnitHandle handle)
        : UnitRef(handle)
    {
    }

    float PlayerRef::getRotation()  const { return Player::GetRotation(); }
    int   PlayerRef::getXP()        const { return Player::GetXP(); }
    int   PlayerRef::getMaxXP()     const { return Player::GetMaxXP(); }

    bool PlayerRef::isGhost()      const { return Player::IsGhost(); }
    bool PlayerRef::isUnderwater() const { return Player::IsUnderwater(); }
    bool PlayerRef::isAfk()        const { return Player::IsAfk(); }
    bool PlayerRef::isDnd()        const { return Player::IsDnd(); }

    Position PlayerRef::getPosition() const { return Player::GetPosition(); }

    UnitRef PlayerRef::getTarget()  const { return Player::GetTarget(); }

    namespace Player
    {
        PlayerRef GetLocalPlayer()          { return PlayerRef(localHandle()); }
        UnitRef   GetTarget()               { return Unit::Get("target"); }

        bool IsInCombat()                   { return Unit::IsInCombat(localHandle()); }
        bool IsDead()                       { return Unit::IsDead(localHandle()); }
        bool IsMounted()                    { return Unit::IsMounted(localHandle()); }
        bool IsFlying()                     { return Unit::IsFlying(localHandle()); }
        bool IsSwimming()                   { return Unit::IsSwimming(localHandle()); }
        bool IsCasting()                    { return Unit::IsCasting(localHandle()); }

        bool IsGhost()
        {
            UnitHandle h = localHandle();
            return h.isNull() ? false : readIsGhost(h.base);
        }

        bool IsUnderwater()                 { return readIsUnderwater(); }

        bool IsAfk()
        {
            UnitHandle h = localHandle();
            return h.isNull() ? false : readIsAfk(h.base);
        }

        bool IsDnd()
        {
            UnitHandle h = localHandle();
            return h.isNull() ? false : readIsDnd(h.base);
        }

        int  GetHealth()                    { return Unit::GetHealth(localHandle()); }
        int  GetMaxHealth()                 { return Unit::GetMaxHealth(localHandle()); }
        int  GetPower()                     { return Unit::GetPower(localHandle()); }
        int  GetMaxPower()                  { return Unit::GetMaxPower(localHandle()); }
        int  GetLevel()                     { return Unit::GetLevel(localHandle()); }
        int  GetRace()                      { return Unit::GetRace(localHandle()); }
        int  GetClass()                     { return Unit::GetClass(localHandle()); }

        Position    GetPosition()
        {
            ::Player p = ::Player::local();
            if (!p.exists()) return { 0.f, 0.f, 0.f };
            return { p.getX(), p.getY(), p.getZ() };
        }
        WoWGUID     GetGUID()               { return Unit::GetGUID(localHandle()); }
        WoWGUID     GetTargetGUID()         { return Unit::GetTargetGUID(localHandle()); }
        std::string GetName()               { return Unit::GetName(localHandle()); }

        float GetRotation()
        {
            UnitHandle h = localHandle();
            return h.isNull() ? 0.f : ::Player(h.base).getRotation();
        }

        int GetXP()
        {
            UnitHandle h = localHandle();
            return h.isNull() ? 0 : ::Player(h.base).getXP();
        }

        int GetMaxXP()
        {
            UnitHandle h = localHandle();
            return h.isNull() ? 0 : ::Player(h.base).getMaxXP();
        }
    }
}
