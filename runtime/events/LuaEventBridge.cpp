#include "LuaEventBridge.h"
#include "EventPipe.h"
#include "GameEventBridge.h"
#include "core/api/SummonAPI.h"
#include "core/engine/LuaEngine.h"
#include "hooks/glue/GlueLoadHooks.h"
#include "offsets/OffsetsSocial.h"
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

        static bool argTrue(const LuaEngine::BridgeMessage& message, size_t index)
        {
            return index < message.args.size()
                && message.args[index].type == LuaEngine::LuaType::Bool
                && message.args[index].boolVal;
        }

        static void onInternalMessage(const LuaEngine::BridgeMessage& message)
        {
            const std::string& name = message.name;
            if (name == "GLUE_LOADED")                Hooks::GlueXML::notifyPostLoad();
            else if (name == "CHARACTER_LIST_UPDATE") Hooks::GlueXML::notifyCharEnum();
            else if (name == "AcceptGroup")
            {
                uint32_t roles = 0;
                if (argTrue(message, 0)) roles |= Offsets::Lfg::ROLE_TANK;
                if (argTrue(message, 1)) roles |= Offsets::Lfg::ROLE_HEALER;
                if (argTrue(message, 2)) roles |= Offsets::Lfg::ROLE_DAMAGE;
                GameEventBridge::OnInviteAnswered(true, roles);
            }
            else if (name == "DeclineGroup")  GameEventBridge::OnInviteAnswered(false, 0);
            else if (name == "ConfirmSummon")
            {
                if (CoreAPI::Summon::CanTeleport())
                    GameEventBridge::OnSummonAnswered(true);
            }
            else if (name == "CancelSummon")  GameEventBridge::OnSummonAnswered(false);
            else if (name == "BeginTrade")    GameEventBridge::OnTradeRequestAnswered(true);
            else if (name == "CancelTrade")   GameEventBridge::OnTradeRequestAnswered(false);
            else if (name == "AcceptDuel")    GameEventBridge::OnDuelAnswered(true);
            else if (name == "CancelDuel")    GameEventBridge::OnDuelAnswered(false);
        }

        static void pushScriptEvents()
        {
            for (const auto& event : LuaEngine::pollScripts())
            {
                SDK::Json data;
                data["name"] = event.name;
                data["status"] = event.loaded ? "loaded" : "failed";
                if (!event.loaded)
                {
                    data["error"] = event.error;
                    if (!event.stack.empty())
                        data["stack"] = event.stack;
                }
                EventPipe::PushEvent("lua.script", sanitize(data), static_cast<uint64_t>(GetTickCount64()));
            }
        }

        void Poll()
        {
            pushScriptEvents();
            LuaEngine::BridgeBatch batch = LuaEngine::drainBridge(EVENT_LIMITS);
            if (batch.messages.empty() && batch.dropped == 0)
                return;

            uint64_t timeMs = static_cast<uint64_t>(GetTickCount64());

            for (const auto& message : batch.messages)
            {
                if (message.kind == LuaEngine::BridgeKind::Internal)
                {
                    onInternalMessage(message);
                    continue;
                }

                if (message.kind == LuaEngine::BridgeKind::Error)
                {
                    SDK::Json data;
                    data["message"] = message.name;
                    data["addon"] = message.addon.empty() ? SDK::Json() : SDK::Json(message.addon);
                    data["stack"] = !message.args.empty() && message.args[0].type == LuaEngine::LuaType::String
                        ? SDK::Json(message.args[0].strVal)
                        : SDK::Json();
                    EventPipe::PushEvent("lua.error", sanitize(data), timeMs);
                    continue;
                }

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
