#pragma once

#include "core/api/game/UnitHandle.h"
#include "core/api/object/ObjectType.h"

namespace GameAPI
{
    namespace Object
    {
        bool            Exists(UnitHandle handle);
        WoWGUID         GetGUID(UnitHandle handle);
        WoW::ObjectType GetType(UnitHandle handle);
        bool            IsUnit(UnitHandle handle);
        bool            IsPlayer(UnitHandle handle);
    }
}
