#pragma once

#include "core/types/GUID.h"
#include "runtime/state/GlueState.h"
#include <string>
#include <mutex>

namespace Runtime
{
    class EventBus;

    struct PlayerCacheEntry
    {
        WoWGUID guid;
        int     health = 0;
        int     maxHealth = 0;
        int     power = 0;
        int     maxPower = 0;
        int     level = 0;
        int     race = 0;
        int     class_ = 0;
        int     xp = 0;
        int     maxXp = 0;
        float   x = 0.f, y = 0.f, z = 0.f;
        float   rotation = 0.f;
        WoWGUID targetGuid;
        bool    inCombat = false;
        bool    dead = false;
        bool    ghost = false;
        bool    mounted = false;
        bool    flying = false;
        bool    swimming = false;
        bool    afk = false;
        bool    dnd = false;
        bool    underwater = false;
        bool    casting = false;
        bool    channeling = false;
        bool    valid = false;
    };

    struct WorldCacheEntry
    {
        int         mapId = 0;
        int         zoneId = 0;
        int         continentId = 0;
        std::string zoneName;
        std::string subZoneName;
        bool        inWorld = false;
        bool        loading = true;
        bool        valid = false;
    };

    class StateCache
    {
    public:
        void update();
        void setEventBus(EventBus* bus);

        bool getPlayer(PlayerCacheEntry& out) const;
        bool getWorld(WorldCacheEntry& out) const;

    private:
        PlayerCacheEntry m_player;
        WorldCacheEntry  m_world;
        Screen           m_lastScreen = Screen::UNKNOWN;
        mutable std::mutex m_mutex;
        EventBus* m_eventBus = nullptr;

        PlayerCacheEntry readPlayer() const;
        WorldCacheEntry  readWorld()  const;
    };
}
