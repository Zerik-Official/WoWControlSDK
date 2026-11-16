#include "LuaEventBridge.h"
#include "EventPipe.h"
#include "core/engine/LuaEngine.h"
#include "rpc/handlers/core/LuaJson.h"
#include "utils/json/Json.h"
#include <Windows.h>

namespace Runtime
{
    namespace LuaEventBridge
    {
        namespace
        {
            const LuaEngine::ReadLimits EVENT_LIMITS{3, 200};

            SDK::Json sanitize(const SDK::Json& data)
            {
                return SDK::Json::parse(data.dump(-1, ' ', false, SDK::Json::error_handler_t::replace));
            }
        }

        void Poll()
        {
            LuaEngine::BridgeBatch batch = LuaEngine::drainBridge(EVENT_LIMITS);
            if (batch.messages.empty() && batch.dropped == 0)
                return;

            uint64_t timeMs = static_cast<uint64_t>(GetTickCount64());

            for (const auto& message : batch.messages)
            {
                SDK::Json data;
                bool hooked = message.kind == LuaEngine::BridgeKind::Hook;
                data[hooked ? "function" : "name"] = message.name;
                if (!hooked)
                    data["addon"] = message.addon;
                data["args"] = Rpc::makeLuaValuesJson(message.args);
                if (message.truncated)
                    data["truncated"] = true;

                EventPipe::PushEvent(hooked ? "lua.hooked" : "lua.custom", sanitize(data), timeMs);
            }

            if (batch.dropped > 0)
            {
                SDK::Json data;
                data["count"] = batch.dropped;
                EventPipe::PushEvent("lua.dropped", data, timeMs);
            }
        }
    }
}
