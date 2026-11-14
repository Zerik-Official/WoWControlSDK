#include "core/api/WorldAPI.h"
#include "core/native/ClientState.h"
#include "core/native/ClientDb.h"
#include "core/native/WorldReader.h"
#include "memory/MemReader.h"
#include "offsets/OffsetsMap.h"
#include "offsets/OffsetsLoading.h"

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

        InstanceInfo GetInstanceInfo()
        {
            namespace I = Offsets::Instance;
            namespace D = Offsets::MapDifficultyDb;

            static const char* const INSTANCE_TYPES[] = { "none", "party", "raid", "pvp", "arena" };

            InstanceInfo info;
            info.mapId             = Memory::safeRead<int>(I::MAP_ID);
            info.difficultyIndex   = Memory::safeRead<int>(I::DIFFICULTY);
            info.difficulty        = info.difficultyIndex + 1;
            info.playerDifficulty  = Memory::safeRead<int>(I::PLAYER_DIFFICULTY);
            info.dungeonDifficulty = Memory::safeRead<int>(I::DUNGEON_DIFFICULTY) + 1;
            info.raidDifficulty    = Memory::safeRead<int>(I::RAID_DIFFICULTY) + 1;

            uintptr_t mapRow = ClientDb::row(Offsets::MapDb::MIN_ID, Offsets::MapDb::MAX_ID,
                                             Offsets::MapDb::ROWS, info.mapId);
            if (!mapRow)
                return info;

            info.mapName        = ClientDb::readStringField(mapRow, I::MAP_ROW_NAME);
            info.mapDirectory   = ClientDb::readStringField(mapRow, I::MAP_ROW_DIRECTORY);
            info.instanceTypeId = Memory::safeRead<int>(mapRow + I::MAP_ROW_INSTANCE_TYPE);
            if (info.instanceTypeId >= 0 && info.instanceTypeId < 5)
                info.instanceType = INSTANCE_TYPES[info.instanceTypeId];
            info.inInstance     = info.instanceTypeId != 0;
            info.dynamic        = (Memory::safeRead<uint32_t>(mapRow + I::MAP_ROW_FLAGS) & I::MAP_FLAG_DYNAMIC) != 0;

            int rows = ClientDb::count(D::COUNT);
            for (int i = 0; i < rows; i++)
            {
                uintptr_t row = ClientDb::indexedRow(D::COUNT, D::ROWS, D::ROW_SIZE, i);
                if (!row) continue;
                if (Memory::safeRead<int>(row + D::ROW_MAP_ID) != info.mapId) continue;
                if (Memory::safeRead<int>(row + D::ROW_DIFFICULTY) != info.difficultyIndex) continue;

                info.mapDifficultyId    = Memory::safeRead<int>(row + D::ROW_ID);
                info.difficultyKey      = ClientDb::readStringField(row, D::ROW_DIFFICULTY_KEY);
                info.maxPlayers         = Memory::safeRead<int>(row + D::ROW_MAX_PLAYERS);
                info.resetSeconds       = Memory::safeRead<int>(row + D::ROW_RESET_SECONDS);
                info.requirementMessage = ClientDb::readStringField(row, D::ROW_MESSAGE);
                break;
            }

            return info;
        }
    }
}
