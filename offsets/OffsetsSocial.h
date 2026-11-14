#pragma once

#include <cstddef>
#include <cstdint>

namespace Offsets
{
    namespace NameCache
    {
        constexpr uintptr_t INSTANCE              = 0xC5D938;
        constexpr uintptr_t RESOLVE               = 0x67D770;
        constexpr uintptr_t QUERY_CALLBACK        = 0x5EEB70;

        constexpr uintptr_t NAME_RACE             = 0x140;
        constexpr uintptr_t NAME_GENDER           = 0x144;
        constexpr uintptr_t NAME_CLASS            = 0x148;
    }

    namespace Timing
    {
        constexpr uintptr_t GET_TIME_MS           = 0x86AE20;
    }

    namespace FrameScript
    {
        constexpr uintptr_t FIRE_EVENT            = 0x81B530;
    }

    namespace Friends
    {
        constexpr uintptr_t LIST_PTR              = 0xC79F98;
        constexpr int       MAX_ENTRIES           = 100;
        constexpr uintptr_t ENTRY_SIZE            = 0x220;

        constexpr uintptr_t ENTRY_ONLINE          = 0x000;
        constexpr uintptr_t ENTRY_FLAGS           = 0x001;
        constexpr uintptr_t ENTRY_NAME            = 0x004;
        constexpr uintptr_t ENTRY_NOTE            = 0x008;
        constexpr size_t    ENTRY_NOTE_LENGTH     = 0x200;
        constexpr uintptr_t ENTRY_GUID            = 0x208;
        constexpr uintptr_t ENTRY_LEVEL           = 0x210;
        constexpr uintptr_t ENTRY_CLASS           = 0x214;
        constexpr uintptr_t ENTRY_AREA            = 0x218;

        constexpr uint8_t   FLAG_AFK              = 0x02;
        constexpr uint8_t   FLAG_DND              = 0x04;
        constexpr uint8_t   FLAG_REFER_A_FRIEND   = 0x08;
    }

    namespace Summon
    {
        constexpr uintptr_t EXPIRES_AT_MS         = 0xBD0864;
        constexpr uintptr_t AREA_ID               = 0xBD0868;
        constexpr uintptr_t SUMMONER_GUID         = 0xBD0870;

        constexpr uintptr_t CONFIRM               = 0x51B050;
        constexpr uintptr_t CANCEL                = 0x51B110;
    }

    namespace Lfg
    {
        constexpr uintptr_t PROPOSAL_DUNGEON      = 0xBEA980;
        constexpr uintptr_t PROPOSAL_STATE        = 0xBEA984;
        constexpr uintptr_t PROPOSAL_ID           = 0xBEA988;
        constexpr uintptr_t PROPOSAL_BOSSES_MASK  = 0xBEA98C;
        constexpr uintptr_t PROPOSAL_MEMBERS      = 0xBEA994;
        constexpr int       PROPOSAL_MAX_MEMBERS  = 5;
        constexpr uintptr_t MEMBER_SIZE           = 0x18;

        constexpr uintptr_t MEMBER_FLAGS          = 0x00;
        constexpr uintptr_t MEMBER_IS_PLAYER      = 0x04;
        constexpr uintptr_t MEMBER_RESPONDED      = 0x10;
        constexpr uintptr_t MEMBER_ACCEPTED       = 0x14;

        constexpr uint32_t  MEMBER_FLAG_LEADER    = 0x01;
        constexpr uint32_t  ROLE_TANK             = 0x02;
        constexpr uint32_t  ROLE_HEALER           = 0x04;
        constexpr uint32_t  ROLE_DAMAGE           = 0x08;

        constexpr uintptr_t ACCEPT_PROPOSAL       = 0x5545E0;
        constexpr uintptr_t REJECT_PROPOSAL       = 0x554600;
    }

    namespace LfgDungeonsDb
    {
        constexpr uintptr_t MIN_ID                = 0xAD4050;
        constexpr uintptr_t MAX_ID                = 0xAD404C;
        constexpr uintptr_t ROWS                  = 0xAD4060;

        constexpr uintptr_t ROW_ID                = 0x00;
        constexpr uintptr_t ROW_NAME              = 0x04;
        constexpr uintptr_t ROW_MIN_LEVEL         = 0x08;
        constexpr uintptr_t ROW_MAX_LEVEL         = 0x0C;
        constexpr uintptr_t ROW_TARGET_LEVEL      = 0x10;
        constexpr uintptr_t ROW_TARGET_LEVEL_MIN  = 0x14;
        constexpr uintptr_t ROW_TARGET_LEVEL_MAX  = 0x18;
        constexpr uintptr_t ROW_MAP_ID            = 0x1C;
        constexpr uintptr_t ROW_DIFFICULTY        = 0x20;
        constexpr uintptr_t ROW_FLAGS             = 0x24;
        constexpr uintptr_t ROW_TYPE              = 0x28;
        constexpr uintptr_t ROW_FACTION           = 0x2C;
        constexpr uintptr_t ROW_TEXTURE           = 0x30;
        constexpr uintptr_t ROW_EXPANSION         = 0x34;
        constexpr uintptr_t ROW_ORDER             = 0x38;
        constexpr uintptr_t ROW_GROUP_ID          = 0x3C;
        constexpr uintptr_t ROW_DESCRIPTION       = 0x40;

        constexpr uint32_t  FLAG_HOLIDAY          = 0x04;
        constexpr int       TYPE_RANDOM           = 6;
    }

    namespace DungeonEncounterDb
    {
        constexpr uintptr_t COUNT                 = 0xAD36B8;
        constexpr uintptr_t ROWS                  = 0xAD36CC;
        constexpr uintptr_t ROW_SIZE              = 0x1C;

        constexpr uintptr_t ROW_ID                = 0x00;
        constexpr uintptr_t ROW_MAP_ID            = 0x04;
        constexpr uintptr_t ROW_DIFFICULTY        = 0x08;
        constexpr uintptr_t ROW_ORDER             = 0x0C;
        constexpr uintptr_t ROW_BIT               = 0x10;
        constexpr uintptr_t ROW_NAME              = 0x14;
        constexpr uintptr_t ROW_SPELL_ICON_ID     = 0x18;
    }

    namespace SpellIconDb
    {
        constexpr uintptr_t MIN_ID                = 0xAD489C;
        constexpr uintptr_t MAX_ID                = 0xAD4898;
        constexpr uintptr_t ROWS                  = 0xAD48AC;

        constexpr uintptr_t ROW_TEXTURE           = 0x04;
    }

    namespace Packet
    {
        constexpr uintptr_t DATA_STORE_VTABLE     = 0x9E0E24;
        constexpr uintptr_t DATA_STORE_FREE_SLOT  = 0x9E0E28;
        constexpr uintptr_t WRITE_U8              = 0x47AFE0;
        constexpr uintptr_t WRITE_U32             = 0x47B0A0;
        constexpr uintptr_t WRITE_U64             = 0x47B100;
        constexpr uintptr_t SEND                  = 0x6B0B50;
    }

    namespace ItemCache
    {
        constexpr uintptr_t INSTANCE              = 0xC5D828;
        constexpr uintptr_t GET_RECORD            = 0x67CA30;
        constexpr uintptr_t RECORD_QUALITY        = 0x14;
        constexpr uintptr_t BUILD_LINK            = 0x61E290;
        constexpr uintptr_t BUILD_OBJECT_LINK     = 0x61E3A0;
        constexpr uintptr_t ITEM_FIELDS           = 0xD4;
        constexpr uintptr_t ITEM_FIELD_STACK      = 0x20;
        constexpr uintptr_t OBJECT_DESCRIPTORS    = 0x08;
        constexpr uintptr_t OBJECT_FIELD_ENTRY    = 0x0C;
    }

    namespace Trade
    {
        constexpr int       SLOT_COUNT            = 7;
        constexpr int       NOT_TRADED_SLOT       = 7;

        constexpr uintptr_t TRADER_GUID           = 0xBFA658;
        constexpr uintptr_t PLAYER_ACCEPTED       = 0xBFA660;
        constexpr uintptr_t TARGET_ACCEPTED       = 0xBFA664;
        constexpr uintptr_t PLAYER_MONEY          = 0xCA0B74;
        constexpr uintptr_t TARGET_MONEY          = 0xCA0B78;

        constexpr uintptr_t PLAYER_ITEM_GUIDS     = 0xBFA620;
        constexpr uintptr_t TARGET_ITEM_IDS       = 0xBFA5C0;
        constexpr uintptr_t TARGET_ITEM_COUNTS    = 0xBFA5A4;
        constexpr uintptr_t TARGET_ITEM_ENCHANTS  = 0xBFA4FC;
        constexpr uintptr_t TARGET_ITEM_GEMS      = 0xBFA550;
        constexpr uintptr_t TARGET_ITEM_SUFFIXES  = 0xBFA454;
        constexpr uintptr_t TARGET_ITEM_SUFFIX_FACTORS = 0xBFA470;
        constexpr int       GEM_SOCKETS           = 3;

        constexpr uintptr_t BEGIN                 = 0x703E00;
        constexpr uintptr_t CANCEL                = 0x704040;
        constexpr uintptr_t ACCEPT                = 0x5866E0;
        constexpr uintptr_t CANCEL_ACCEPT         = 0x586730;
    }

    namespace Duel
    {
        constexpr uintptr_t ARBITER_GUID          = 0xC22B58;
        constexpr uintptr_t CHALLENGER_GUID       = 0xC22B60;

        constexpr uintptr_t ACCEPT                = 0x5CFBD0;
        constexpr uintptr_t CANCEL                = 0x5CFC50;
    }

    namespace Battlefield
    {
        constexpr uintptr_t QUEUES                = 0xBEA4B8;
        constexpr int       MAX_QUEUES            = 2;
        constexpr uintptr_t QUEUE_SIZE            = 0x40;
        constexpr uintptr_t ACTIVE_MAP_ID         = 0xBEA564;
        constexpr uintptr_t STATUS_QUEUE_TYPE     = 0xBEA53C;

        constexpr uintptr_t QUEUE_GUID            = 0x00;
        constexpr uintptr_t QUEUE_INSTANCE_GUID   = 0x08;
        constexpr uintptr_t QUEUE_TYPE_ID         = 0x10;
        constexpr uintptr_t QUEUE_MAP_ID          = 0x14;
        constexpr uintptr_t QUEUE_STATUS          = 0x18;
        constexpr uintptr_t QUEUE_MIN_LEVEL       = 0x1C;
        constexpr uintptr_t QUEUE_MAX_LEVEL       = 0x20;
        constexpr uintptr_t QUEUE_INSTANCE_ID     = 0x24;
        constexpr uintptr_t QUEUE_PORT_EXPIRES_AT = 0x28;
        constexpr uintptr_t QUEUE_ESTIMATED_WAIT  = 0x2C;
        constexpr uintptr_t QUEUE_JOINED_AT       = 0x30;
        constexpr uintptr_t QUEUE_TEAM_SIZE       = 0x34;
        constexpr uintptr_t QUEUE_RATED           = 0x38;

        constexpr int       TYPE_RANDOM           = 32;
        constexpr uint32_t  OPCODE_PORT           = 0x2D5;
        constexpr int       LEAVE_EVENT_ID        = 0x19C;
        constexpr uintptr_t STOP_READY_SOUND      = 0x848380;
    }

    namespace BattlemasterListDb
    {
        constexpr uintptr_t MIN_ID                = 0xAD3264;
        constexpr uintptr_t MAX_ID                = 0xAD3260;
        constexpr uintptr_t ROWS                  = 0xAD3274;

        constexpr uintptr_t ROW_MAP_ID            = 0x04;
        constexpr uintptr_t ROW_NAME              = 0x2C;
    }
}
