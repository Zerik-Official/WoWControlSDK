#pragma once

#include "rpc/MethodRegistry.h"
#include "runtime/events/EventBus.h"
#include "runtime/state/GlueState.h"
#include "runtime/state/StateCache.h"

namespace Runtime
{
    void initialize();
    void shutdown();
    void onFrame();

    MethodRegistry& registry();
    EventBus&       events();
    StateCache&     cache();
}
