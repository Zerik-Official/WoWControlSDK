#pragma once

#include "MethodRegistry.h"
#include "EventBus.h"
#include "GlueState.h"
#include "StateCache.h"

namespace Runtime
{
    void initialize();
    void shutdown();
    void onFrame();

    MethodRegistry& registry();
    EventBus&       events();
    StateCache&     cache();
}
