#include "ControlRpc.h"
#include "runtime/console/ConsoleManager.h"
#include "runtime/console/LogCapture.h"
#include "utils/json/Json.h"

namespace Rpc
{
    using SDK::Json;

    static Json handleToggleConsole(const Json&)
    {
        Runtime::ConsoleManager::Toggle();
        Json j;
        j["ok"] = true;
        j["visible"] = Runtime::ConsoleManager::IsVisible();
        return j;
    }

    static Json handleGetInfo(const Json&)
    {
        Runtime::ConsoleInfo ci = Runtime::ConsoleManager::GetInfo();
        Runtime::LogConfig lc = Runtime::LogCapture::GetConfig();

        Json j;
        j["version"] = ci.version;
        j["uptimeMs"] = (int64_t)ci.uptimeMs;
        j["console"] = ci.initialized;
        j["consoleVisible"] = ci.visible;
        j["logCount"] = Runtime::LogCapture::GetLogCount();
        j["logMaxEntries"] = lc.maxEntries;
        j["logShowInConsole"] = lc.showInConsole;
        return j;
    }

    static Json handleGetLogs(const Json& params)
    {
        int offset = params.value("offset", 0);
        int count = params.value("count", 50);

        auto entries = Runtime::LogCapture::GetLogs(offset, count);
        Json arr = Json::array();
        for (const auto& e : entries)
        {
            Json entry;
            entry["text"] = e.text;
            entry["style"] = e.style;
            entry["timestampMs"] = (int64_t)e.timestampMs;
            arr.push_back(entry);
        }

        Json j;
        j["total"] = Runtime::LogCapture::GetLogCount();
        j["returned"] = (int)entries.size();
        j["offset"] = offset;
        j["entries"] = arr;
        return j;
    }

    static Json handleSetLogConfig(const Json& params)
    {
        int maxEntries = params.value("maxEntries", 256);
        bool showInConsole = params.value("showInConsole", false);

        Runtime::LogCapture::SetConfig(maxEntries, showInConsole);
        Runtime::LogConfig lc = Runtime::LogCapture::GetConfig();

        Json j;
        j["ok"] = true;
        j["maxEntries"] = lc.maxEntries;
        j["showInConsole"] = lc.showInConsole;
        return j;
    }

    void registerControlMethods(Runtime::MethodRegistry& registry)
    {
        registry.registerMethod("dll.toggleConsole", handleToggleConsole);
        registry.registerMethod("dll.getInfo", handleGetInfo);
        registry.registerMethod("dll.getLogs", handleGetLogs);
        registry.registerMethod("dll.setLogConfig", handleSetLogConfig);
    }
}
