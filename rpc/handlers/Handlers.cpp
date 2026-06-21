#include "Handlers.h"
#include "ClientRpc.h"
#include "PlayerRpc.h"
#include "WorldRpc.h"
#include "AuthRpc.h"
#include "CharRpc.h"
#include "UnitRpc.h"

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
    }
}
