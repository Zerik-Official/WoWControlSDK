#include "runtime/StateCache.h"
#include "runtime/EventBus.h"
#include "runtime/EventTypes.h"
#include "core/api/PlayerAPI.h"
#include "core/api/WorldAPI.h"

namespace Runtime
{
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
        e.targetGuid = p.getTargetGUID();
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
        e.inWorld      = CoreAPI::World::IsInWorld();
        e.loading      = CoreAPI::World::IsOnLoadingScreen();
        e.valid        = true;
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

        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_player = freshPlayer;
            m_world  = freshWorld;
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
}
