#pragma once

#include "source/BridgeSourceCore.h"
#include "source/BridgeSourceStorage.h"
#include "source/BridgeSourceApi.h"
#include "source/BridgeSourceScripts.h"
#include "source/BridgeSourceEvents.h"
#include "source/BridgeSourceAuth.h"

namespace LuaBridge {

inline constexpr const char* SOURCE_PARTS[] = {
    SOURCE_CORE,
    SOURCE_STORAGE,
    SOURCE_API,
    SOURCE_SCRIPTS,
    SOURCE_EVENTS,
    SOURCE_AUTH,
};

}
