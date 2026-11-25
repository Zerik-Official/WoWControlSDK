#include "LuaEventBridge.h"
#include "EventPipe.h"
#include "GameEventBridge.h"
#include "core/api/SummonAPI.h"
#include "core/engine/LuaEngine.h"
#include "hooks/EventHooks.h"
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

        static SDK::Json argJson(const LuaEngine::BridgeMessage& message, size_t index)
        {
            return index < message.args.size() ? Rpc::makeLuaValueJson(message.args[index]) : SDK::Json();
        }

        static void onLoginScreenMessage(const LuaEngine::BridgeMessage& message)
        {
            const std::string& name = message.name;
            uint64_t timeMs = static_cast<uint64_t>(GetTickCount64());
            SDK::Json data = SDK::Json::object();
            if (name == "AUTH_CHALLENGE" || name == "AUTH_CHALLENGE_CLOSED")
            {
                data["type"] = argJson(message, 0);
                if (name == "AUTH_CHALLENGE" && data["type"] == "pin")
                {
                    data["minDigits"] = argJson(message, 1);
                    data["maxDigits"] = argJson(message, 2);
                }
                EventPipe::PushEvent(name == "AUTH_CHALLENGE" ? "client.authChallenge" : "client.authChallengeClosed",
                                     sanitize(data), timeMs);
            }
            else if (name == "REALM_LIST_OPENED")
            {
                SDK::Json realms = argJson(message, 0);
                data["realms"] = realms.is_array() ? realms : SDK::Json::array();
                EventPipe::PushEvent("client.realmListOpened", sanitize(data), timeMs);
            }
            else if (name == "REALM_LIST_CLOSED")
            {
                EventPipe::PushEvent("client.realmListClosed", data, timeMs);
            }
        }

        static void onInternalMessage(const LuaEngine::BridgeMessage& message)
        {
            const std::string& name = message.name;
            if (name == "AUTH_CHALLENGE" || name == "AUTH_CHALLENGE_CLOSED" || name == "REALM_LIST_OPENED"
                || name == "REALM_LIST_CLOSED")
                onLoginScreenMessage(message);
            else if (name == "GLUE_LOADED")           Hooks::GlueXML::notifyPostLoad();
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
            LuaEngine::configureEvents(Hooks::Events::WantedEvents());
            LuaEngine::BridgeBatch batch = LuaEngine::drainBridge(EVENT_LIMITS);
            uint64_t timeMs = static_cast<uint64_t>(GetTickCount64());

            for (const auto& blocked : LuaEngine::takeBlockedActions())
            {
                SDK::Json data;
                data["event"] = blocked.event;
                data["function"] = blocked.function;
                EventPipe::PushEvent("lua.blocked", sanitize(data), timeMs);
            }

            if (batch.messages.empty() && batch.dropped == 0)
                return;

            for (const auto& message : batch.messages)
            {
                if (message.kind == LuaEngine::BridgeKind::Internal)
                {
                    onInternalMessage(message);
                    continue;
                }

                if (message.kind == LuaEngine::BridgeKind::Event)
                {
                    SDK::Json args = sanitize(Rpc::makeLuaValuesJson(message.args));
                    if (Hooks::Events::IsSubscribed(message.name))
                        EventPipe::PushEvent(message.name.c_str(), args, timeMs);
                    if (GameEventBridge::Handles(message.name))
                        GameEventBridge::OnGameEvent(message.name, args, timeMs);
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
