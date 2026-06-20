#include "Runtime.h"
#include "rpc/handlers/ClientRpc.h"

namespace Runtime
{
    static MethodRegistry s_registry;
    static EventBus s_eventBus;
    static StateCache s_cache;
    static bool s_initialized = false;

    void initialize()
    {
        if (s_initialized) return;

        Glue::initialize();
        s_cache.setEventBus(&s_eventBus);
        Rpc::registerClientMethods(s_registry);

        s_initialized = true;
    }

    void shutdown()
    {
        s_initialized = false;
    }

    void onFrame()
    {
        s_cache.update();
    }

    MethodRegistry& registry() { return s_registry; }
    EventBus&       events()   { return s_eventBus; }
    StateCache&     cache()    { return s_cache; }
}
