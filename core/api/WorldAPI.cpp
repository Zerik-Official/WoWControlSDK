#include "core/api/WorldAPI.h"
#include "core/native/ClientState.h"
#include "core/native/WorldReader.h"
#include "memory/MemReader.h"
#include "offsets/OffsetsMap.h"

namespace CoreAPI
{
    namespace World
    {
        int GetMapId()
        {
            return WorldReader::read().mapId;
        }

        int GetZoneId()
        {
            return WorldReader::read().zoneId;
        }

        int GetContinentId()
        {
            return WorldReader::read().continentId;
        }

        int GetLuaMapId()
        {
            return WorldReader::read().luaMapId;
        }

        int GetLuaZoneId()
        {
            return WorldReader::read().luaZoneId;
        }

        std::string GetZoneName()
        {
            return WorldReader::read().zoneName;
        }

        std::string GetSubZoneName()
        {
            return WorldReader::read().subZoneName;
        }

        bool IsInWorld()
        {
            return WoW::IsInWorld();
        }

        bool IsOnLoadingScreen()
        {
            return Memory::safeRead<int>(Offsets::Map::IS_INGAME) == 0;
        }

        uint32_t GetGameTick()
        {
            return Memory::safeRead<uint32_t>(Offsets::Map::TICK);
        }
    }
}
