#pragma once
#include <cstdint>

constexpr uintptr_t WOW_LUA_EXECUTE          = 0x00819210;

constexpr uintptr_t LUA_STATE_PTR_ADDR       = 0x00D3F78C;
constexpr uintptr_t LUA_PCALL_ADDR           = 0x0084EC50;
constexpr uintptr_t LUA_LOADBUFFER_ADDR      = 0x0084F860;
constexpr uintptr_t LUA_GETTOP_ADDR          = 0x0084DBD0;
constexpr uintptr_t LUA_SETTOP_ADDR          = 0x0084DBF0;
constexpr uintptr_t LUA_TOLSTRING_ADDR       = 0x0084E0E0;
constexpr uintptr_t LUA_TONUMBER_ADDR        = 0x0084E030;
constexpr uintptr_t LUA_TOBOOLEAN_ADDR       = 0x0084E0B0;
constexpr uintptr_t LUA_ISNUMBER_ADDR        = 0x0084DF20;
constexpr uintptr_t LUA_TYPE_ADDR            = 0x0084DEB0;

constexpr uintptr_t STATIC_CLIENT_CONNECTION = 0x00C79CE0;
constexpr uintptr_t OBJECT_MANAGER_OFFSET    = 0x2ED0;
constexpr uintptr_t LOCAL_GUID_OFFSET        = 0xC0;

constexpr uintptr_t FIND_OBJ_BY_GUID        = 0x004D4DB0;

constexpr uintptr_t OBJECT_POS_X            = 0x9B8;
constexpr uintptr_t OBJECT_POS_Y            = 0x9BC;
constexpr uintptr_t OBJECT_POS_Z            = 0x9C0;

constexpr uintptr_t UNIT_HEALTH             = 0x1044;
constexpr uintptr_t UNIT_HEALTH_MAX         = 0x1048;
constexpr uintptr_t UNIT_MANA              = 0x1054;
constexpr uintptr_t UNIT_MANA_MAX          = 0x1068;

constexpr uintptr_t UNIT_DESCRIPTOR_BASE   = 0x8;

constexpr uintptr_t DESC_LEVEL             = 0x36 * 4;
constexpr uintptr_t DESC_RACE_CLASS_GENDER = 0x37 * 4;

constexpr uintptr_t NAME_STORE   = 0x00C5D940;
constexpr uintptr_t NAME_BASE    = 0x1C;
constexpr uintptr_t NAME_MASK    = 0x24;
constexpr uintptr_t NAME_STRING  = 0x20;

static constexpr uintptr_t ADDR_MAP_ID       = 0xADFBC4;
static constexpr uintptr_t ADDR_ZONE_ID      = 0xBD080C;
static constexpr uintptr_t ADDR_IS_INGAME    = 0xBEBAA4;
static constexpr uintptr_t ADDR_IS_WORLD     = 0xBEBA40;
static constexpr uintptr_t ADDR_TICK         = 0xB499A4;

static constexpr uintptr_t OBJ_DESC_END  = 24;

static constexpr uintptr_t UDESC_RACE_CLASS_GENDER_POWERTYPE = 0x44;
static constexpr uintptr_t UDESC_HEALTH       = 0x48;
static constexpr uintptr_t UDESC_POWER1       = 0x4C;
static constexpr uintptr_t UDESC_POWER2       = 0x50;
static constexpr uintptr_t UDESC_POWER4       = 0x58;
static constexpr uintptr_t UDESC_POWER7       = 0x64;
static constexpr uintptr_t UDESC_MAXHEALTH    = 0x68;
static constexpr uintptr_t UDESC_MAXPOWER1    = 0x6C;
static constexpr uintptr_t UDESC_MAXPOWER2    = 0x70;
static constexpr uintptr_t UDESC_MAXPOWER4    = 0x78;
static constexpr uintptr_t UDESC_MAXPOWER7    = 0x84;

static constexpr uintptr_t UDESC_LEVEL        = 0xC0;

static constexpr uintptr_t UDESC_FLAGS        = 0xD4;
static constexpr uintptr_t UDESC_FLAGS2       = 0xD8;
static constexpr uintptr_t UDESC_DYNAMIC_FLAGS = 0x124;

static constexpr uint32_t UNIT_FLAG_MOUNTED   = 0x08000000;

static constexpr uint32_t UNIT_FLAG2_FEIGN_DEATH = 0x1;

static constexpr uint32_t DYNFLAG_DEAD        = 0x00000020;

static constexpr uintptr_t MOVE_FLAGS_OFFSET          = 0xA30;
static constexpr uintptr_t FLY_FLAGS_POINTER_OFFSET   = 0xD8;
static constexpr uintptr_t FLY_FLAGS_OFFSET           = 0x44;

static constexpr uint32_t MOVE_FLAG_SWIMMING  = 0x00200000;
static constexpr uint32_t MOVE_FLAG_FLYING    = 0x02000000;

static constexpr uintptr_t ADDR_BREATH_TIMER  = 0xBD0BA0;

static constexpr int GHOST_SPELL_ID           = 8326;

static constexpr uintptr_t PDESC_XP           = 0x798;
static constexpr uintptr_t PDESC_NEXTLEVELXP  = 0x79C;

static constexpr uintptr_t POS_OFFSET        = 0x798;
static constexpr uintptr_t ROT_OFFSET        = 0x7A8;