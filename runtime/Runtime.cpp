#include "Runtime.h"
#include "rpc/handlers/ClientRpc.h"

namespace Runtime
{
    static MethodRegistry s_registry;
    static EventBus s_eventBus;
    static bool s_initialized = false;

    void initialize()
    {
        if (s_initialized) return;

        Glue::initialize();
        Rpc::registerClientMethods(s_registry);

        s_initialized = true;
    }

    void shutdown()
    {
        s_initialized = false;
    }

    MethodRegistry& registry() { return s_registry; }
    EventBus&       events()   { return s_eventBus; }
}
