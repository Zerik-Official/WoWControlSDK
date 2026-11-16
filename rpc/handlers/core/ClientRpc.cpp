#include "ClientRpc.h"
#include "hooks/GlueHooks.h"
#include "runtime/Runtime.h"
#include "runtime/diagnostics/AddonMemoryMonitor.h"
#include "runtime/diagnostics/CrashMonitor.h"
#include "utils/json/Json.h"
#include <Windows.h>
#include <cmath>

namespace Rpc
{
    using SDK::Json;

    static Json handlePing(const Json&)
    {
        Json result;
        result["status"] = "ok";
        return result;
    }

    static double rounded(double value, double scale)
    {
        if (!std::isfinite(value))
            return 0.0;
        return std::round(value * scale) / scale;
    }

    static int64_t ageMs(uint64_t updatedMs)
    {
        return static_cast<int64_t>(GetTickCount64() - updatedMs);
    }

    static bool getPerformance(Runtime::PerformanceCacheEntry& out, Json& err)
    {
        if (Runtime::cache().getPerformance(out))
            return true;
        err = SDK::makeErrorJson("performance not available");
        return false;
    }

    static Json handleGetNetStats(const Json&)
    {
        Runtime::PerformanceCacheEntry p;
        Json err;
        if (!getPerformance(p, err))
            return err;

        Json result;
        result["connected"] = p.connected;
        result["latencyMs"] = p.latencyMs;
        result["bandwidthIn"] = rounded(p.bandwidthIn, 1000.0);
        result["bandwidthOut"] = rounded(p.bandwidthOut, 1000.0);
        result["ageMs"] = ageMs(p.updatedMs);
        return result;
    }

    static Json handleGetFramerate(const Json&)
    {
        Runtime::PerformanceCacheEntry p;
        Json err;
        if (!getPerformance(p, err))
            return err;

        Json result;
        result["fps"] = rounded(p.framerate, 10.0);
        result["ageMs"] = ageMs(p.updatedMs);
        return result;
    }

    static Json addonMemoryJson()
    {
        Runtime::AddonMemoryMonitor::Report report;
        if (!Runtime::AddonMemoryMonitor::Read(report))
            return SDK::makeErrorJson("lua not available");

        Json addons = Json::array();
        for (const auto& addon : report.addons)
        {
            Json item;
            item["name"] = addon.name;
            item["kilobytes"] = rounded(addon.kilobytes, 10.0);
            addons.push_back(item);
        }

        Json result;
        result["addons"] = addons;
        result["addonsKilobytes"] = rounded(report.addonsKilobytes, 10.0);
        result["totalKilobytes"] = rounded(report.totalKilobytes, 10.0);
        result["ageMs"] = ageMs(report.updatedMs);
        return result;
    }

    static Json handleGetAddonMemory(const Json&)
    {
        std::string raw = Hooks::Glue::Execute([]() -> std::string {
            return addonMemoryJson().dump(-1, ' ', false, Json::error_handler_t::replace);
        });
        return Json::parse(raw);
    }

    static Json handleGetLastCrash(const Json&)
    {
        Json crash;
        if (!Runtime::CrashMonitor::GetLast(crash))
        {
            Json result;
            result["crashed"] = false;
            return result;
        }

        crash["crashed"] = true;
        return crash;
    }

    void registerClientMethods(Runtime::MethodRegistry& registry)
    {
        registry.registerMethod("client.ping", handlePing);
        registry.registerMethod("client.getNetStats", handleGetNetStats);
        registry.registerMethod("client.getFramerate", handleGetFramerate);
        registry.registerMethod("client.getAddonMemory", handleGetAddonMemory);
        registry.registerMethod("client.getLastCrash", handleGetLastCrash);
    }
}
