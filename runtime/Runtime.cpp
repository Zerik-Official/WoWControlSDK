#include "Runtime.h"
#include "core/native/GlueAPI.h"
#include "rpc/handlers/Handlers.h"
#include "ipc/CommandPipe.h"
#include "hooks/Hooks.h"
#include <Windows.h>

namespace Runtime
{
    static MethodRegistry s_registry;
    static EventBus s_eventBus;
    static StateCache s_cache;
    static bool s_initialized = false;
    static HANDLE s_cmdThread = nullptr;

    void initialize()
    {
        if (s_initialized) return;

        Hooks::GlueXML::registerPostLoad([]()
    {
        WoW::Glue::AcceptAll();
    });

    Glue::initialize();
        s_cache.setEventBus(&s_eventBus);
        Rpc::registerAllMethods(s_registry);

        s_cmdThread = CreateThread(nullptr, 0, CommandPipe::threadProc, nullptr, 0, nullptr);

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
