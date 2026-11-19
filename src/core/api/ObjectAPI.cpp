#include "core/api/ObjectAPI.h"
#include "core/native/WoWObject.h"

namespace
{
    WoWObject resolve(CoreAPI::UnitHandle handle)
    {
        return WoWObject(handle.base);
    }
}

namespace CoreAPI
{
    namespace Object
    {
        bool Exists(UnitHandle handle)
        {
            if (handle.isNull()) return false;
            return resolve(handle).isValid();
        }

        WoWGUID GetGUID(UnitHandle handle)
        {
            if (handle.isNull()) return NULL_GUID;
            return handle.guid;
        }

        WoW::ObjectType GetType(UnitHandle handle)
        {
            if (handle.isNull()) return WoW::ObjectType::Object;

            uint32_t mask = resolve(handle).getTypeMask();

            if (mask & static_cast<uint32_t>(WoW::ObjectType::Player))        return WoW::ObjectType::Player;
            if (mask & static_cast<uint32_t>(WoW::ObjectType::Unit))          return WoW::ObjectType::Unit;
            if (mask & static_cast<uint32_t>(WoW::ObjectType::GameObject))    return WoW::ObjectType::GameObject;
            if (mask & static_cast<uint32_t>(WoW::ObjectType::Item))          return WoW::ObjectType::Item;
            if (mask & static_cast<uint32_t>(WoW::ObjectType::Container))     return WoW::ObjectType::Container;
            if (mask & static_cast<uint32_t>(WoW::ObjectType::DynamicObject)) return WoW::ObjectType::DynamicObject;
            if (mask & static_cast<uint32_t>(WoW::ObjectType::Corpse))        return WoW::ObjectType::Corpse;

            return WoW::ObjectType::Object;
        }

        bool IsUnit(UnitHandle handle)
        {
            if (handle.isNull()) return false;
            return resolve(handle).isUnit();
        }

        bool IsPlayer(UnitHandle handle)
        {
            if (handle.isNull()) return false;
            return resolve(handle).isPlayer();
        }
    }
}
