#include "Handlers.h"
#include "core/ClientRpc.h"
#include "player/PlayerRpc.h"
#include "core/WorldRpc.h"
#include "auth/AuthRpc.h"
#include "core/CharRpc.h"
#include "core/UnitRpc.h"
#include "core/GroupRpc.h"
#include "core/ObjectRpc.h"
#include "core/LuaRpc.h"
#include "control/ControlRpc.h"
#include "event/EventRpc.h"
#include "window/WindowRpc.h"

namespace Rpc
{
    void registerAllMethods(Runtime::MethodRegistry& registry)
    {
        registerClientMethods(registry);
        registerPlayerMethods(registry);
        registerWorldMethods(registry);
        registerAuthMethods(registry);
        registerCharMethods(registry);
        registerUnitMethods(registry);
        registerGroupMethods(registry);
        registerObjectMethods(registry);
        registerLuaMethods(registry);
        registerControlMethods(registry);
        registerEventMethods(registry);
        RPC::Window::registerWindowMethods(registry);
    }
}
