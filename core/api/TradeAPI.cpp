#include "core/api/TradeAPI.h"
#include "core/native/ClientDb.h"
#include "core/native/FrameScript.h"
#include "core/native/NameCache.h"
#include "core/native/ObjectManager.h"
#include "memory/MemReader.h"
#include "offsets/OffsetsSocial.h"
#include <Windows.h>
#include <cstdlib>

namespace
{
    namespace T = Offsets::Trade;
    namespace I = Offsets::ItemCache;

    SRWLOCK               s_requestLock = SRWLOCK_INIT;
    CoreAPI::TradeRequest s_request     = { false, {}, 0 };

    uintptr_t itemRecord(int itemId)
    {
        if (itemId <= 0) return 0;
        using GetRecordFn = uintptr_t(__thiscall*)(uintptr_t, int, uint32_t*, uintptr_t, int, int);
        uint32_t guid[2] = { 0, 0 };
        return reinterpret_cast<GetRecordFn>(I::GET_RECORD)(I::INSTANCE, itemId, guid, 0, 0, 0);
    }

    void parseLink(CoreAPI::TradeItem& item)
    {
        size_t start = item.link.find("|Hitem:");
        if (start != std::string::npos)
        {
            const char* p = item.link.c_str() + start + 7;
            int values[9] = {};
            for (int i = 0; i < 9 && *p && *p != '|'; i++)
            {
                values[i] = std::atoi(p);
                while (*p && *p != ':' && *p != '|') p++;
                if (*p == ':') p++;
            }
            item.itemId    = values[0];
            item.enchantId = values[1];
            item.gemIds[0] = values[2];
            item.gemIds[1] = values[3];
            item.gemIds[2] = values[4];
            item.suffixId  = values[6];
            item.uniqueId  = values[7];
        }

        size_t nameStart = item.link.find("|h[");
        size_t nameEnd = item.link.find("]|h", nameStart == std::string::npos ? 0 : nameStart);
        if (nameStart != std::string::npos && nameEnd != std::string::npos)
            item.name = item.link.substr(nameStart + 3, nameEnd - nameStart - 3);
    }

    CoreAPI::TradeItem emptyItem(int slot)
    {
        CoreAPI::TradeItem item = {};
        item.slot = slot;
        item.willNotBeTraded = slot == T::NOT_TRADED_SLOT;
        return item;
    }

    CoreAPI::TradeItem readPlayerItem(int index)
    {
        CoreAPI::TradeItem item = emptyItem(index + 1);
        WoWGUID guid = Memory::safeRead<WoWGUID>(T::PLAYER_ITEM_GUIDS + static_cast<uintptr_t>(index) * sizeof(WoWGUID));
        if (!guid.isValid()) return item;

        uintptr_t object = WoW::GetObjectByGUID(guid);
        if (!object) return item;

        item.exists = true;
        item.guid   = guid;

        uintptr_t descriptors = Memory::safeRead<uintptr_t>(object + I::OBJECT_DESCRIPTORS);
        item.itemId = Memory::safeRead<int>(descriptors + I::OBJECT_FIELD_ENTRY);
        uintptr_t fields = Memory::safeRead<uintptr_t>(object + I::ITEM_FIELDS);
        item.count  = Memory::safeRead<int>(fields + I::ITEM_FIELD_STACK);

        uintptr_t record = itemRecord(item.itemId);
        item.cached  = record != 0;
        item.quality = record ? Memory::safeRead<int>(record + I::RECORD_QUALITY) : -1;
        if (record)
        {
            using BuildLinkFn = const char*(__cdecl*)(uintptr_t);
            const char* link = reinterpret_cast<BuildLinkFn>(I::BUILD_OBJECT_LINK)(object);
            item.link = ClientDb::readCString(reinterpret_cast<uintptr_t>(link), 1024);
            parseLink(item);
        }
        return item;
    }

    CoreAPI::TradeItem readTargetItem(int index)
    {
        CoreAPI::TradeItem item = emptyItem(index + 1);
        uintptr_t i = static_cast<uintptr_t>(index);
        item.itemId = Memory::safeRead<int>(T::TARGET_ITEM_IDS + i * 4);
        if (item.itemId == 0) return item;

        item.exists    = true;
        item.count     = Memory::safeRead<int>(T::TARGET_ITEM_COUNTS + i * 4);
        item.enchantId = Memory::safeRead<int>(T::TARGET_ITEM_ENCHANTS + i * 4);
        int gems[3] = {};
        for (int g = 0; g < T::GEM_SOCKETS; g++)
        {
            gems[g] = Memory::safeRead<int>(T::TARGET_ITEM_GEMS + (i * T::GEM_SOCKETS + g) * 4);
            item.gemIds[g] = gems[g];
        }
        item.suffixId = Memory::safeRead<int>(T::TARGET_ITEM_SUFFIXES + i * 4);
        int suffixFactor = Memory::safeRead<int>(T::TARGET_ITEM_SUFFIX_FACTORS + i * 4);

        uintptr_t record = itemRecord(item.itemId);
        item.cached  = record != 0;
        item.quality = record ? Memory::safeRead<int>(record + I::RECORD_QUALITY) : -1;
        if (record)
        {
            using BuildLinkFn = const char*(__cdecl*)(int, int, int, int*, int, int);
            const char* link = reinterpret_cast<BuildLinkFn>(I::BUILD_LINK)(
                item.itemId, item.quality, item.enchantId, gems, item.suffixId, suffixFactor);
            item.link = ClientDb::readCString(reinterpret_cast<uintptr_t>(link), 1024);
            int count = item.count;
            int enchant = item.enchantId;
            parseLink(item);
            item.count = count;
            item.enchantId = enchant;
        }
        return item;
    }

    bool inWorld()
    {
        return WoW::GetLocalGUID().isValid();
    }

    bool trading()
    {
        return Memory::safeRead<WoWGUID>(T::TRADER_GUID).isValid();
    }

    void callScriptFunction(uintptr_t address)
    {
        using ScriptFn = int(__cdecl*)(void*);
        reinterpret_cast<ScriptFn>(address)(nullptr);
    }

    void callVoid(uintptr_t address)
    {
        using VoidFn = void(__cdecl*)();
        reinterpret_cast<VoidFn>(address)();
    }
}

namespace CoreAPI
{
    namespace Trade
    {
        TradeRequest GetRequest()
        {
            AcquireSRWLockShared(&s_requestLock);
            TradeRequest request = s_request;
            ReleaseSRWLockShared(&s_requestLock);
            return request;
        }

        void SetRequest(const std::string& requesterName, uint64_t receivedAtMs)
        {
            AcquireSRWLockExclusive(&s_requestLock);
            s_request = TradeRequest{ true, requesterName, receivedAtMs };
            ReleaseSRWLockExclusive(&s_requestLock);
        }

        void ClearRequest()
        {
            AcquireSRWLockExclusive(&s_requestLock);
            s_request = TradeRequest{ false, {}, 0 };
            ReleaseSRWLockExclusive(&s_requestLock);
        }

        TradeInfo Get()
        {
            TradeInfo info = {};
            info.request    = GetRequest();
            info.traderGuid = Memory::safeRead<WoWGUID>(T::TRADER_GUID);
            info.open       = info.traderGuid.isValid();
            if (!info.open)
                return info;

            info.traderName     = NameCache::lookup(info.traderGuid).name;
            info.playerAccepted = Memory::safeRead<int>(T::PLAYER_ACCEPTED) != 0;
            info.targetAccepted = Memory::safeRead<int>(T::TARGET_ACCEPTED) != 0;
            info.playerMoney    = Memory::safeRead<uint32_t>(T::PLAYER_MONEY);
            info.targetMoney    = Memory::safeRead<uint32_t>(T::TARGET_MONEY);

            for (int i = 0; i < T::SLOT_COUNT; i++)
            {
                info.playerItems.push_back(readPlayerItem(i));
                info.targetItems.push_back(readTargetItem(i));
            }
            return info;
        }

        TradeResult AcceptRequest()
        {
            if (!inWorld()) return TradeResult::NotInWorld;
            if (!GetRequest().pending) return TradeResult::NoRequest;

            callVoid(T::BEGIN);
            FrameScript::FireEvent("TRADE_REQUEST_CANCEL");
            return TradeResult::Ok;
        }

        TradeResult DeclineRequest()
        {
            if (!inWorld()) return TradeResult::NotInWorld;
            if (!GetRequest().pending) return TradeResult::NoRequest;

            callVoid(T::CANCEL);
            FrameScript::FireEvent("TRADE_REQUEST_CANCEL");
            return TradeResult::Ok;
        }

        TradeResult Accept()
        {
            if (!inWorld()) return TradeResult::NotInWorld;
            if (!trading()) return TradeResult::NotTrading;

            callScriptFunction(T::ACCEPT);
            return TradeResult::Ok;
        }

        TradeResult Unaccept()
        {
            if (!inWorld()) return TradeResult::NotInWorld;
            if (!trading()) return TradeResult::NotTrading;

            callScriptFunction(T::CANCEL_ACCEPT);
            return TradeResult::Ok;
        }

        TradeResult Cancel()
        {
            if (!inWorld()) return TradeResult::NotInWorld;
            if (!trading()) return TradeResult::NotTrading;

            callVoid(T::CANCEL);
            return TradeResult::Ok;
        }
    }
}
