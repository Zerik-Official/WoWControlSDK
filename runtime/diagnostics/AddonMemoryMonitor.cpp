#include "runtime/diagnostics/AddonMemoryMonitor.h"
#include "core/engine/LuaEngine.h"
#include <Windows.h>
#include <algorithm>

namespace Runtime::AddonMemoryMonitor
{
    namespace
    {
        constexpr uint64_t MIN_INTERVAL_MS = 1000;

        Report s_last;

        double readTotalKilobytes()
        {
            LuaEngine::CallTarget target;
            target.path = "collectgarbage";

            LuaEngine::LuaValue mode;
            mode.type = LuaEngine::LuaType::String;
            mode.strVal = "count";

            LuaEngine::LuaResult result = LuaEngine::call(target, {mode}, LuaEngine::ReadLimits{}, false);
            if (!result.ok || result.values.empty() || result.values[0].type != LuaEngine::LuaType::Number)
                return 0.0;
            return result.values[0].numVal;
        }
    }

    bool Read(Report& out)
    {
        uint64_t now = GetTickCount64();
        if (s_last.valid && now - s_last.updatedMs < MIN_INTERVAL_MS)
        {
            out = s_last;
            return true;
        }

        Report fresh;
        if (!WoW::Addons::ReadMemoryUsage(fresh.addons))
            return false;

        std::sort(fresh.addons.begin(), fresh.addons.end(), [](const auto& a, const auto& b) {
            return a.kilobytes > b.kilobytes;
        });
        for (const auto& addon : fresh.addons)
            fresh.addonsKilobytes += addon.kilobytes;

        for (const auto& [name, key] : LuaEngine::scriptMemoryKeys())
            fresh.scripts.push_back({name, WoW::Addons::ReadKeyKilobytes(key)});

        fresh.totalKilobytes = readTotalKilobytes();
        fresh.updatedMs = now;
        fresh.valid = true;

        s_last = fresh;
        out = fresh;
        return true;
    }
}
