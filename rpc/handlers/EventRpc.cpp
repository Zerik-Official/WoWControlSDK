#include "EventRpc.h"
#include "hooks/EventHooks.h"
#include "runtime/EventPipe.h"
#include "utils/json/Json.h"
#include <unordered_set>

namespace Rpc
{
    using SDK::Json;

    static Json handleSubscribe(const Json& params)
    {
        if (!params.contains("events") || !params["events"].is_array())
            return SDK::makeErrorJson("events must be an array");

        int count = 0;
        for (const auto& e : params["events"])
        {
            std::string name = e.get<std::string>();
            Hooks::Events::Subscribe(name.c_str());
            count++;
        }

        Json j;
        j["ok"] = true;
        j["subscribed"] = count;
        return j;
    }

    static Json handleUnsubscribe(const Json& params)
    {
        if (!params.contains("events") || !params["events"].is_array())
            return SDK::makeErrorJson("events must be an array");

        int count = 0;
        for (const auto& e : params["events"])
        {
            std::string name = e.get<std::string>();
            Hooks::Events::Unsubscribe(name.c_str());
            count++;
        }

        Json j;
        j["ok"] = true;
        j["unsubscribed"] = count;
        return j;
    }

    static Json handleList(const Json&)
    {
        std::vector<std::string> subs;
        Hooks::Events::GetSubscriptions(subs);
        Json arr = Json::array();
        for (const auto& s : subs)
            arr.push_back(s);
        Json j;
        j["events"] = arr;
        return j;
    }

    static Json handleClear(const Json&)
    {
        Hooks::Events::ClearSubscriptions();
        return SDK::okJson();
    }

    static Json handleGetHistory(const Json& params)
    {
        int offset = params.value("offset", 0);
        int count = params.value("count", 50);

        auto entries = Runtime::EventPipe::GetHistory(offset, count);
        Json arr = Json::array();
        for (const auto& e : entries)
        {
            Json entry;
            entry["event"] = e.eventName;
            entry["args"]  = e.args;
            entry["time"]  = static_cast<int64_t>(e.timestampMs);
            arr.push_back(entry);
        }

        Json j;
        j["total"] = Runtime::EventPipe::GetBufferCount();
        j["returned"] = (int)entries.size();
        j["offset"] = offset;
        j["entries"] = arr;
        return j;
    }

    static Json handleClearHistory(const Json&)
    {
        Runtime::EventPipe::ClearHistory();
        return SDK::okJson();
    }

    static Json handleSetMaxBuffer(const Json& params)
    {
        int max = params.value("maxBuffer", 256);
        Runtime::EventPipe::SetMaxBuffer(max);
        Json j;
        j["ok"] = true;
        j["maxBuffer"] = Runtime::EventPipe::GetBufferCount() > 0 ? max : max;
        return j;
    }

    static Json handleIsReady(const Json&)
    {
        Json j;
        j["ready"] = Hooks::Events::IsReady();
        return j;
    }

    void registerEventMethods(Runtime::MethodRegistry& registry)
    {
        registry.registerMethod("events.subscribe", handleSubscribe);
        registry.registerMethod("events.unsubscribe", handleUnsubscribe);
        registry.registerMethod("events.list", handleList);
        registry.registerMethod("events.clear", handleClear);
        registry.registerMethod("events.getHistory", handleGetHistory);
        registry.registerMethod("events.clearHistory", handleClearHistory);
        registry.registerMethod("events.setMaxBuffer", handleSetMaxBuffer);
        registry.registerMethod("events.isReady", handleIsReady);
    }
}
