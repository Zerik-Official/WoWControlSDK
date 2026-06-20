#include "Handlers.h"
#include "ClientRpc.h"
#include "PlayerRpc.h"
#include "WorldRpc.h"

namespace Rpc
{
    void registerAllMethods(Runtime::MethodRegistry& registry)
    {
        registerClientMethods(registry);
        registerPlayerMethods(registry);
        registerWorldMethods(registry);
    }
}
