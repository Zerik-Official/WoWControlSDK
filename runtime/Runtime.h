#pragma once

#include "MethodRegistry.h"
#include "EventBus.h"
#include "GlueState.h"

namespace Runtime
{
    void initialize();
    void shutdown();

    MethodRegistry& registry();
    EventBus&       events();
}
