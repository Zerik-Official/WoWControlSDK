#include "core/api/FriendsAPI.h"
#include "core/native/ClientDb.h"
#include "core/native/NameCache.h"
#include "memory/MemReader.h"
#include "offsets/OffsetsSocial.h"

namespace CoreAPI
{
    namespace Friends
    {
        FriendList GetList()
        {
            namespace F = Offsets::Friends;

            FriendList list = {};
            uintptr_t base = Memory::safeRead<uintptr_t>(F::LIST_PTR);
            if (!base)
                return list;

            list.loaded = true;
            for (int i = 0; i < F::MAX_ENTRIES; i++)
            {
                uintptr_t entry = base + static_cast<uintptr_t>(i) * F::ENTRY_SIZE;
                WoWGUID guid = Memory::safeRead<WoWGUID>(entry + F::ENTRY_GUID);
                if (!guid.isValid())
                    continue;

                FriendInfo f = {};
                f.index         = i + 1;
                f.guid          = guid;
                f.name          = ClientDb::readCString(Memory::safeRead<uintptr_t>(entry + F::ENTRY_NAME), 48);
                if (f.name.empty())
                    f.name = NameCache::lookup(guid).name;
                f.online        = Memory::safeRead<uint8_t>(entry + F::ENTRY_ONLINE) != 0;
                f.flags         = Memory::safeRead<uint8_t>(entry + F::ENTRY_FLAGS);
                f.afk           = f.online && (f.flags & F::FLAG_AFK) != 0;
                f.dnd           = f.online && (f.flags & F::FLAG_DND) != 0;
                f.referAFriend  = (f.flags & F::FLAG_REFER_A_FRIEND) != 0;
                f.note          = ClientDb::readCString(entry + F::ENTRY_NOTE, F::ENTRY_NOTE_LENGTH);
                f.level         = Memory::safeRead<int>(entry + F::ENTRY_LEVEL);
                f.classId       = Memory::safeRead<int>(entry + F::ENTRY_CLASS);
                f.className     = ClientDb::className(f.classId);
                f.classFileName = ClientDb::classFileName(f.classId);
                f.areaId        = Memory::safeRead<int>(entry + F::ENTRY_AREA);
                f.areaName      = ClientDb::areaName(f.areaId);

                list.count++;
                if (f.online)
                    list.onlineCount++;
                list.friends.push_back(f);
            }
            return list;
        }
    }
}
