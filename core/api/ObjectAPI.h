#pragma once

#include "core/api/UnitHandle.h"
#include "core/native/ObjectType.h"

namespace CoreAPI
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
