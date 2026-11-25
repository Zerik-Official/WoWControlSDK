#pragma once

#include "core/types/GUID.h"
#include "core/api/WorldAPI.h"
#include "runtime/state/GlueState.h"
#include <string>
#include <mutex>
#include <cstdint>

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
        int         mapAreaId = 0;
        CoreAPI::InstanceInfo instance;
        bool        inWorld = false;
        bool        loading = true;
        bool        valid = false;
    };

    struct PerformanceCacheEntry
    {
        float    framerate = 0.f;
        bool     connected = false;
        uint32_t latencyMs = 0;
        float    bandwidthIn = 0.f;
        float    bandwidthOut = 0.f;
        uint64_t updatedMs = 0;
        bool     valid = false;
    };

    class StateCache
    {
    public:
        void update();
        void setEventBus(EventBus* bus);

        bool getPlayer(PlayerCacheEntry& out) const;
        bool getWorld(WorldCacheEntry& out) const;
        bool getPerformance(PerformanceCacheEntry& out) const;

    private:
        PlayerCacheEntry m_player;
        WorldCacheEntry  m_world;
        PerformanceCacheEntry m_performance;
        Screen           m_lastScreen = Screen::UNKNOWN;
        std::string      m_lastScreenName;
        mutable std::mutex m_mutex;
        EventBus* m_eventBus = nullptr;

        PlayerCacheEntry readPlayer() const;
        WorldCacheEntry  readWorld()  const;
        PerformanceCacheEntry readPerformance(bool inWorld) const;
    };
}
