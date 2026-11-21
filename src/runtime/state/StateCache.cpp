#include "runtime/state/StateCache.h"
#include <Windows.h>
#include "runtime/events/EventBus.h"
#include "runtime/events/EventPipe.h"
#include "runtime/events/EventTypes.h"
#include "runtime/state/GlueState.h"
#include "core/api/PlayerAPI.h"
#include "core/api/WorldAPI.h"
#include "core/native/NetClient.h"
#include "core/native/Performance.h"
#include "memory/MemReader.h"
#include "offsets/OffsetsPlayer.h"

namespace Runtime
{
    static constexpr uint64_t PERFORMANCE_INTERVAL_MS = 500;

    void StateCache::setEventBus(EventBus* bus)
    {
        m_eventBus = bus;
    }

    PlayerCacheEntry StateCache::readPlayer() const
    {
        PlayerCacheEntry e;
        CoreAPI::PlayerRef p = CoreAPI::Player::GetLocalPlayer();
        if (!p.exists()) return e;

        e.guid       = p.getGUID();
        e.health     = p.getHealth();
        e.maxHealth  = p.getMaxHealth();
        e.power      = p.getPower();
        e.maxPower   = p.getMaxPower();
        e.level      = p.getLevel();
        e.race       = p.getRace();
        e.class_     = p.getClass();
        e.xp         = p.getXP();
        e.maxXp      = p.getMaxXP();
        e.x          = p.getPosition().x;
        e.y          = p.getPosition().y;
        e.z          = p.getPosition().z;
        e.rotation   = p.getRotation();
        e.targetGuid = Memory::safeRead<WoWGUID>(Offsets::Player::CURRENT_TARGET_GUID);
        e.inCombat   = p.isInCombat();
        e.dead       = p.isDead();
        e.ghost      = p.isGhost();
        e.mounted    = p.isMounted();
        e.flying     = p.isFlying();
        e.swimming   = p.isSwimming();
        e.afk        = p.isAfk();
        e.dnd        = p.isDnd();
        e.underwater = p.isUnderwater();
        e.casting    = p.isCasting();
        e.channeling = false;
        e.valid      = true;
        return e;
    }

    WorldCacheEntry StateCache::readWorld() const
    {
        WorldCacheEntry e;
        e.mapId        = CoreAPI::World::GetMapId();
        e.zoneId       = CoreAPI::World::GetZoneId();
        e.continentId  = CoreAPI::World::GetContinentId();
        e.zoneName     = CoreAPI::World::GetZoneName();
        e.subZoneName  = CoreAPI::World::GetSubZoneName();
        e.mapAreaId    = CoreAPI::World::GetLuaMapId();
        e.instance     = CoreAPI::World::GetInstanceInfo();
        e.inWorld      = CoreAPI::World::IsInWorld();
        e.loading      = CoreAPI::World::IsOnLoadingScreen();
        e.valid        = true;
        return e;
    }

    PerformanceCacheEntry StateCache::readPerformance(bool inWorld) const
    {
        PerformanceCacheEntry e;
        e.framerate = WoW::Performance::GetFramerate();

        WoW::Net::NetStats stats;
        e.connected = inWorld && WoW::Net::GetNetStats(stats);
        e.latencyMs = stats.latencyMs;
        e.bandwidthIn = stats.bandwidthIn;
        e.bandwidthOut = stats.bandwidthOut;
        e.updatedMs = GetTickCount64();
        e.valid = true;
        return e;
    }

    void StateCache::update()
    {
        PlayerCacheEntry prevPlayer;
        WorldCacheEntry  prevWorld;

        {
            std::lock_guard<std::mutex> lock(m_mutex);
            prevPlayer = m_player;
            prevWorld  = m_world;
        }

        PlayerCacheEntry freshPlayer = readPlayer();
        WorldCacheEntry  freshWorld  = readWorld();

        bool refreshPerformance = GetTickCount64() - m_performance.updatedMs >= PERFORMANCE_INTERVAL_MS;
        PerformanceCacheEntry freshPerformance = refreshPerformance ? readPerformance(freshWorld.inWorld) : PerformanceCacheEntry{};

        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_player = freshPlayer;
            m_world  = freshWorld;
            if (refreshPerformance)
                m_performance = freshPerformance;
        }

        if (!m_eventBus) return;

        if (prevPlayer.valid && freshPlayer.valid)
        {
            if (prevPlayer.health != freshPlayer.health || prevPlayer.maxHealth != freshPlayer.maxHealth)
            {
                Json data;
                data["old"] = prevPlayer.health;
                data["new"] = freshPlayer.health;
                data["maxHealth"] = freshPlayer.maxHealth;
                m_eventBus->emit(EVENT_PLAYER_HEALTH, data);
            }

            if (prevPlayer.targetGuid.raw() != freshPlayer.targetGuid.raw())
            {
                Json data;
                data["old"] = Json::array({prevPlayer.targetGuid.high, prevPlayer.targetGuid.low});
                data["new"] = Json::array({freshPlayer.targetGuid.high, freshPlayer.targetGuid.low});
                m_eventBus->emit(EVENT_PLAYER_TARGET, data);
            }

            if (prevPlayer.inCombat != freshPlayer.inCombat)
            {
                Json data;
                m_eventBus->emit(
                    freshPlayer.inCombat ? EVENT_COMBAT_STARTED : EVENT_COMBAT_ENDED,
                    data
                );
            }
        }

        if (prevWorld.valid && freshWorld.valid)
        {
            if (prevWorld.mapId != freshWorld.mapId || prevWorld.zoneId != freshWorld.zoneId)
            {
                Json data;
                data["oldMapId"] = prevWorld.mapId;
                data["newMapId"] = freshWorld.mapId;
                data["oldZoneId"] = prevWorld.zoneId;
                data["newZoneId"] = freshWorld.zoneId;
                m_eventBus->emit(EVENT_WORLD_ZONE, data);
            }
        }

        Screen currentScreen = Glue::getScreen();
        const char* screenName = Glue::getScreenName();
        std::string currentName = screenName ? screenName : "";
        if (currentScreen != m_lastScreen || currentName != m_lastScreenName)
        {
            Json data;
            data["old"] = static_cast<int>(m_lastScreen);
            data["new"] = static_cast<int>(currentScreen);
            data["oldName"] = m_lastScreenName;
            data["newName"] = currentName;
            m_lastScreenName = currentName;
            m_eventBus->emit(EVENT_SCREEN_CHANGED, data);
            EventPipe::PushEvent("client.screenChanged", data, GetTickCount64());
            m_lastScreen = currentScreen;
        }
    }

    bool StateCache::getPlayer(PlayerCacheEntry& out) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        out = m_player;
        return out.valid;
    }

    bool StateCache::getWorld(WorldCacheEntry& out) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        out = m_world;
        return out.valid;
    }

    bool StateCache::getPerformance(PerformanceCacheEntry& out) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        out = m_performance;
        return out.valid;
    }
}
