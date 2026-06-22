#include "ControlRpc.h"
#include "core/native/GlueAPI.h"
#include "offsets/OffsetsLua.h"
#include "runtime/console/ConsoleManager.h"
#include "runtime/console/LogCapture.h"
#include "utils/json/Json.h"

namespace
{

using LuaExecFn = void(__cdecl*)(const char* code, const char* name, int);

void dismissAgreementDialog()
{
    auto exec = (LuaExecFn)Offsets::Lua::WOW_LUA_EXECUTE;
    exec(
        "AcceptEULA();AcceptTOS();AcceptTerminationWithoutNotice();AcceptScanning();AcceptContest();"
        "if TOSFrame and TOSFrame:IsShown() then TOSFrame:Hide() TOSNotice:Hide() AccountLoginUI:Show() end",
        "sdk_accept", 0);
}

}

namespace Rpc
{
    using SDK::Json;

    static Json handleAcceptEULA(const Json&)
    {
        WoW::Glue::AcceptEULA();
        dismissAgreementDialog();
        return SDK::okJson();
    }

    static Json handleAcceptTOS(const Json&)
    {
        WoW::Glue::AcceptTOS();
        dismissAgreementDialog();
        return SDK::okJson();
    }

    static Json handleAcceptTermination(const Json&)
    {
        WoW::Glue::AcceptTermination();
        dismissAgreementDialog();
        return SDK::okJson();
    }

    static Json handleAcceptScanning(const Json&)
    {
        WoW::Glue::AcceptScanning();
        dismissAgreementDialog();
        return SDK::okJson();
    }

    static Json handleAcceptContest(const Json&)
    {
        WoW::Glue::AcceptContest();
        dismissAgreementDialog();
        return SDK::okJson();
    }

    static Json handleAcceptAll(const Json&)
    {
        WoW::Glue::AcceptAll();
        dismissAgreementDialog();
        return SDK::okJson();
    }

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
        registry.registerMethod("client.acceptEULA", handleAcceptEULA);
        registry.registerMethod("client.acceptTOS", handleAcceptTOS);
        registry.registerMethod("client.acceptTermination", handleAcceptTermination);
        registry.registerMethod("client.acceptScanning", handleAcceptScanning);
        registry.registerMethod("client.acceptContest", handleAcceptContest);
        registry.registerMethod("client.acceptAll", handleAcceptAll);
        registry.registerMethod("dll.toggleConsole", handleToggleConsole);
        registry.registerMethod("dll.getInfo", handleGetInfo);
        registry.registerMethod("dll.getLogs", handleGetLogs);
        registry.registerMethod("dll.setLogConfig", handleSetLogConfig);
    }
}
