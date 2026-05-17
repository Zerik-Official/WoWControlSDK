#include "ObjectManager.h"
#include "memory/MemReader.h"
#include "OffsetsObjectMgr.h"

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

    uintptr_t GetObjectByGUID(const WoWGUID& guid)
    {
        if (!guid.isValid())
            return 0;

        uintptr_t objMgr = GetObjectManager();
        if (!objMgr)
            return 0;

        using FindObjFn = uintptr_t(__thiscall*)(uintptr_t, const WoWGUID*);
        auto findObj = reinterpret_cast<FindObjFn>(Offsets::ObjectMgr::FIND_OBJ_BY_GUID);

        return findObj(objMgr, &guid);
    }
}