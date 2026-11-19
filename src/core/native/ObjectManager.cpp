#include "core/native/ObjectManager.h"
#include "memory/MemReader.h"
#include "offsets/OffsetsObjectMgr.h"

namespace WoW
{
    uintptr_t GetObjectManager()
    {
        uintptr_t clientConnection = Memory::safeRead<uintptr_t>(Offsets::ObjectMgr::STATIC_CLIENT_CONNECTION);
        if (!clientConnection)
            return 0;

        return Memory::safeRead<uintptr_t>(clientConnection + Offsets::ObjectMgr::OBJECT_MANAGER_OFFSET);
    }

    WoWGUID GetLocalGUID()
    {
        uintptr_t objMgr = GetObjectManager();
        if (!objMgr)
            return NULL_GUID;

        return Memory::safeRead<WoWGUID>(objMgr + Offsets::ObjectMgr::LOCAL_GUID_OFFSET);
    }

    WoWGUID GetMouseoverGUID()
    {
        return Memory::safeRead<WoWGUID>(Offsets::ObjectMgr::MOUSEOVER_GUID);
    }

    uintptr_t GetObjectByGUID(const WoWGUID& guid)
    {
        if (!guid.isValid())
            return 0;

        using FindObjFn = uintptr_t(__cdecl*)(uint32_t, uint32_t, uint32_t);
        auto findObj = reinterpret_cast<FindObjFn>(Offsets::ObjectMgr::FIND_OBJ_BY_GUID);

        return findObj(guid.low, guid.high, Offsets::ObjectMgr::OBJ_TYPE_MASK_UNIT);
    }
}
