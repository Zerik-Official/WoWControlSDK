#include "LuaRpc.h"
#include "LuaJson.h"
#include "core/engine/LuaEngine.h"
#include "hooks/GlueHooks.h"
#include "runtime/storage/AddonStorage.h"
#include "utils/json/Json.h"
#include <functional>
#include <string>
#include <unordered_map>

namespace Rpc
{
    using SDK::Json;

    using LuaRunner = std::function<Json(const Json&)>;

    static constexpr size_t MAX_BATCH_OPERATIONS = 32;

    static Json errorJson(const std::string& message)
    {
        Json j;
        j["ok"] = false;
        j["error"] = message;
        return j;
    }

    static std::string dumpSafe(const Json& j)
    {
        return j.dump(-1, ' ', false, Json::error_handler_t::replace);
    }

    static Json blockedJson(const std::vector<LuaEngine::BlockedAction>& blocked)
    {
        Json array = Json::array();
        for (const auto& action : blocked)
        {
            Json item;
            item["event"] = action.event;
            item["function"] = action.function;
            array.push_back(item);
        }
        return array;
    }

    static Json resultToJson(const LuaEngine::LuaResult& r)
    {
        if (!r.ok)
        {
            Json j = errorJson(r.error);
            if (!r.stack.empty())
                j["stack"] = r.stack;
            if (!r.blocked.empty())
                j["blocked"] = blockedJson(r.blocked);
            return j;
        }

        Json values = makeLuaValuesJson(r.values);

        Json j;
        j["ok"] = true;
        if (values.size() == 1)
            j["result"] = values[0];
        else if (values.size() > 1)
            j["result"] = values;
        else
            j["result"] = nullptr;
        j["results"] = values;
        if (r.truncated)
            j["truncated"] = true;
        return j;
    }

    static std::string stringParam(const Json& params, const char* name)
    {
        if (!params.contains(name) || !params[name].is_string())
            return "";
        return params[name].get<std::string>();
    }

    static bool secureParam(const Json& params)
    {
        return params.contains("secure") && params["secure"].is_boolean() && params["secure"].get<bool>();
    }

    static Json pathResultJson(const LuaEngine::LuaResult& r)
    {
        if (!r.ok)
            return resultToJson(r);
        return SDK::okJson();
    }

    static Json runExecute(const Json& params)
    {
        std::string code = stringParam(params, "code");
        if (code.empty())
            return errorJson("code is required");

        std::vector<LuaEngine::BlockedAction> blocked =
            LuaEngine::execute(code, stringParam(params, "chunkName"), secureParam(params));
        if (blocked.empty())
            return SDK::okJson();

        Json j = errorJson("blocked by the game: " + blocked.front().function);
        j["blocked"] = blockedJson(blocked);
        return j;
    }

    static Json runEvaluate(const Json& params)
    {
        std::string code = stringParam(params, "code");
        if (code.empty())
            return errorJson("code is required");

        return resultToJson(LuaEngine::evaluate(code, stringParam(params, "chunkName"), parseLuaReadLimits(params), secureParam(params)));
    }

    static Json runCall(const Json& params)
    {
        LuaEngine::CallTarget target;
        target.path = stringParam(params, "function");
        target.method = stringParam(params, "method");
        target.addon = stringParam(params, "addon");

        if (params.contains("addon"))
        {
            if (target.addon.empty())
                return errorJson("addon must be the name of an addon or script");
            if (!LuaEngine::isIdentifier(target.path))
                return errorJson("function must be the name of a function the addon exposed with SDK.Expose");
        }
        else if (params.contains("ref"))
        {
            if (!params["ref"].is_number_integer() || params["ref"].get<int>() <= 0)
                return errorJson("ref must be a positive integer");
            target.ref = params["ref"].get<int>();
            if (!target.method.empty() && !LuaEngine::isIdentifier(target.method))
                return errorJson("method must be a Lua identifier");
        }
        else if (!LuaEngine::isPath(target.path, true))
        {
            return errorJson("function must be a path such as GetItemInfo, Frame.field or Frame:Method");
        }

        std::vector<LuaEngine::LuaValue> args;
        if (params.contains("args"))
        {
            if (!params["args"].is_array())
                return errorJson("args must be an array");
            for (const auto& item : params["args"])
            {
                LuaEngine::LuaValue arg;
                if (!parseLuaValueJson(item, arg))
                    return errorJson("args are nested too deeply");
                args.push_back(std::move(arg));
            }
        }

        return resultToJson(LuaEngine::call(target, args, parseLuaReadLimits(params), secureParam(params)));
    }

    static Json runGetGlobal(const Json& params)
    {
        std::string name = stringParam(params, "name");
        if (!LuaEngine::isPath(name, false))
            return errorJson("name must be a path such as UnitName or MyAddon.config.volume");

        return resultToJson(LuaEngine::getPath(name, parseLuaReadLimits(params)));
    }

    static Json runSetGlobal(const Json& params)
    {
        std::string name = stringParam(params, "name");
        if (!LuaEngine::isPath(name, false))
            return errorJson("name must be a path such as MyAddon.config.volume");

        LuaEngine::LuaValue value;
        if (params.contains("value") && !parseLuaValueJson(params["value"], value))
            return errorJson("value is nested too deeply");

        return pathResultJson(LuaEngine::setPath(name, value));
    }

    static Json runCreateNamespace(const Json& params)
    {
        std::string name = stringParam(params, "name");
        if (!LuaEngine::isPath(name, false))
            return errorJson("name must be a path such as MyAddon or MyAddon.cache");

        LuaEngine::LuaValue table;
        table.type = LuaEngine::LuaType::Table;
        return pathResultJson(LuaEngine::setPath(name, table));
    }

    static Json runGetRef(const Json& params)
    {
        std::string path = stringParam(params, "path");
        if (!LuaEngine::isPath(path, false))
            return errorJson("path must be a path such as PlayerFrame or MyAddon.db");

        LuaEngine::LuaResult r = LuaEngine::createRef(path);
        if (!r.ok || r.values.size() < 2)
            return resultToJson(r);

        Json j;
        j["ok"] = true;
        j["ref"] = makeLuaNumberJson(r.values[0].numVal);
        j["type"] = r.values[1].strVal;
        return j;
    }

    static Json runReleaseRef(const Json& params)
    {
        if (!params.contains("ref") || !params["ref"].is_number_integer())
            return errorJson("ref is required");

        LuaEngine::LuaResult r = LuaEngine::releaseRef(params["ref"].get<int>());
        if (!r.ok)
            return resultToJson(r);

        Json j;
        j["ok"] = true;
        j["released"] = !r.values.empty() && r.values[0].boolVal;
        return j;
    }

    static Json runHook(const Json& params)
    {
        std::string path = stringParam(params, "function");
        if (!LuaEngine::isPath(path, true))
            return errorJson("function must be a path such as AcceptQuest or StaticPopup1Button1:Click");

        LuaEngine::LuaResult r = LuaEngine::hook(path);
        if (!r.ok)
            return resultToJson(r);

        Json j;
        j["ok"] = true;
        j["function"] = path;
        j["hooked"] = true;
        return j;
    }

    static Json runUnhook(const Json& params)
    {
        std::string path = stringParam(params, "function");
        if (!LuaEngine::isPath(path, true))
            return errorJson("function is required");

        LuaEngine::LuaResult r = LuaEngine::unhook(path);
        if (!r.ok)
            return resultToJson(r);

        Json j;
        j["ok"] = true;
        j["function"] = path;
        j["unhooked"] = !r.values.empty() && r.values[0].boolVal;
        return j;
    }

    static Json runGetHooks(const Json&)
    {
        LuaEngine::LuaResult r = LuaEngine::listHooks();
        if (!r.ok)
            return resultToJson(r);

        Json j;
        j["ok"] = true;
        j["hooks"] = r.values.empty() ? Json::array() : makeLuaValueJson(r.values[0]);
        if (!j["hooks"].is_array())
            j["hooks"] = Json::array();
        return j;
    }

    static Json stringListJson(const std::vector<std::string>& values)
    {
        Json array = Json::array();
        for (const auto& value : values)
            array.push_back(value);
        return array;
    }

    static Json addonsJson()
    {
        Json j;
        j["ok"] = true;
        j["granted"] = stringListJson(LuaEngine::grantedAddons());
        auto allowed = LuaEngine::allowedAddons();
        j["allowed"] = allowed ? stringListJson(*allowed) : Json();
        return j;
    }

    static Json runSetAllowedAddons(const Json& params)
    {
        if (!params.contains("addons"))
            return errorJson("addons is required, use null to allow every addon that opts in");

        const Json& addons = params["addons"];
        if (addons.is_null())
        {
            LuaEngine::setAllowedAddons(std::nullopt);
            return addonsJson();
        }
        if (!addons.is_array())
            return errorJson("addons must be an array of addon names or null");

        std::vector<std::string> names;
        for (const auto& addon : addons)
        {
            if (!addon.is_string() || addon.get<std::string>().empty())
                return errorJson("addons must contain addon names");
            names.push_back(addon.get<std::string>());
        }
        LuaEngine::setAllowedAddons(std::move(names));
        return addonsJson();
    }

    static Json runGetAddons(const Json&)
    {
        return addonsJson();
    }

    static Json runSetBlockedNotice(const Json& params)
    {
        if (!params.contains("show") || !params["show"].is_boolean())
            return errorJson("show must be a boolean");

        LuaEngine::setShowBlockedNotice(params["show"].get<bool>());

        Json j;
        j["ok"] = true;
        j["show"] = LuaEngine::showBlockedNotice();
        return j;
    }

    static bool boolParam(const Json& params, const char* name, bool fallback)
    {
        if (!params.contains(name) || !params[name].is_boolean())
            return fallback;
        return params[name].get<bool>();
    }

    static Json resourcesJson(const LuaEngine::ScriptResources& resources)
    {
        Json j;
        j["frames"] = resources.frames;
        j["hooks"] = resources.hooks;
        j["globals"] = resources.globals;
        j["slashCommands"] = resources.slashCommands;
        return j;
    }

    static Json scriptJson(const LuaEngine::ScriptInfo& info)
    {
        Json j;
        j["name"] = info.name;
        j["bytes"] = info.bytes;
        j["persist"] = info.persist;
        j["loaded"] = info.loaded;
        j["resources"] = info.loaded ? resourcesJson(info.resources) : Json();
        return j;
    }

    static Json runLoadScript(const Json& params)
    {
        std::string name = stringParam(params, "name");
        std::string code = stringParam(params, "code");
        if (name.empty())
            return errorJson("name is required");
        if (code.empty())
            return errorJson("code is required");

        LuaEngine::ScriptInfo info;
        LuaEngine::LuaResult result = LuaEngine::loadScript(name, code, boolParam(params, "persist", true),
            boolParam(params, "replace", true), info);
        if (!result.ok)
            return resultToJson(result);

        Json j = scriptJson(info);
        j["ok"] = true;
        j["pending"] = !info.loaded;
        return j;
    }

    static Json runUnloadScript(const Json& params)
    {
        std::string name = stringParam(params, "name");
        if (name.empty())
            return errorJson("name is required");

        LuaEngine::ScriptResources removed;
        LuaEngine::LuaResult result = LuaEngine::unloadScript(name, removed);
        if (!result.ok)
            return resultToJson(result);

        Json j;
        j["ok"] = true;
        j["name"] = name[0] == '@' ? name : "@" + name;
        j["removed"] = resourcesJson(removed);
        return j;
    }

    static Json runGetScripts(const Json&)
    {
        Json scripts = Json::array();
        for (const auto& info : LuaEngine::listScripts())
            scripts.push_back(scriptJson(info));

        Json j;
        j["ok"] = true;
        j["scripts"] = scripts;
        return j;
    }

    static Json runGetStorage(const Json& params)
    {
        std::string addon = stringParam(params, "addon");
        if (!Runtime::AddonStorage::isValidOwner(addon))
            return errorJson("addon must be the name of an addon");

        Json data = Runtime::AddonStorage::get(addon);
        Json j;
        j["ok"] = true;
        j["addon"] = addon;
        if (params.contains("key"))
        {
            std::string key = stringParam(params, "key");
            j["key"] = key;
            j["value"] = data.contains(key) ? data[key] : Json();
            j["exists"] = data.contains(key);
        }
        else
        {
            j["data"] = data;
        }
        return j;
    }

    static Json runSetStorage(const Json& params)
    {
        std::string addon = stringParam(params, "addon");
        std::string key = stringParam(params, "key");
        Json value = params.contains("value") ? params["value"] : Json();

        std::string error = Runtime::AddonStorage::set(addon, key, value);
        if (!error.empty())
            return errorJson(error);

        Json j;
        j["ok"] = true;
        j["deleted"] = value.is_null();
        return j;
    }

    static const std::unordered_map<std::string, LuaRunner>& runners()
    {
        static const std::unordered_map<std::string, LuaRunner> table = {
            {"lua.execute", runExecute},
            {"lua.evaluate", runEvaluate},
            {"lua.call", runCall},
            {"lua.getGlobal", runGetGlobal},
            {"lua.setGlobal", runSetGlobal},
            {"lua.createNamespace", runCreateNamespace},
            {"lua.getRef", runGetRef},
            {"lua.releaseRef", runReleaseRef},
            {"lua.hook", runHook},
            {"lua.unhook", runUnhook},
            {"lua.getHooks", runGetHooks},
            {"lua.setAllowedAddons", runSetAllowedAddons},
            {"lua.getAddons", runGetAddons},
            {"lua.setBlockedNotice", runSetBlockedNotice},
            {"lua.loadScript", runLoadScript},
            {"lua.unloadScript", runUnloadScript},
            {"lua.getScripts", runGetScripts},
            {"lua.getStorage", runGetStorage},
            {"lua.setStorage", runSetStorage},
        };
        return table;
    }

    static Json runBatch(const Json& params)
    {
        if (!params.contains("operations") || !params["operations"].is_array())
            return errorJson("operations must be an array");

        const Json& operations = params["operations"];
        if (operations.empty())
            return errorJson("operations is empty");
        if (operations.size() > MAX_BATCH_OPERATIONS)
            return errorJson("operations accepts at most 32 entries");

        bool stopOnError = params.contains("stopOnError") && params["stopOnError"].is_boolean()
            && params["stopOnError"].get<bool>();

        Json results = Json::array();
        bool stopped = false;
        for (const auto& operation : operations)
        {
            Json result;
            std::string method = operation.is_object() ? stringParam(operation, "method") : "";
            auto runner = runners().find(method);
            if (runner == runners().end())
            {
                result = errorJson("unsupported method: " + method);
            }
            else
            {
                Json opParams = operation.contains("params") && operation["params"].is_object()
                    ? operation["params"]
                    : Json::object();
                result = runner->second(opParams);
            }

            bool failed = result.contains("error");
            results.push_back(std::move(result));
            if (failed && stopOnError)
            {
                stopped = true;
                break;
            }
        }

        Json j;
        j["ok"] = true;
        j["results"] = results;
        j["completed"] = results.size();
        if (stopped)
            j["stopped"] = true;
        return j;
    }

    static Json onMainThread(LuaRunner runner, const Json& params)
    {
        std::string raw = Hooks::Glue::Execute([runner, params]() -> std::string {
            return dumpSafe(runner(params));
        });
        return Json::parse(raw);
    }

    void registerLuaMethods(Runtime::MethodRegistry& registry)
    {
        for (const auto& [name, runner] : runners())
        {
            LuaRunner bound = runner;
            registry.registerMethod(name, [bound](const Json& params) {
                return onMainThread(bound, params);
            });
        }

        registry.registerMethod("lua.batch", [](const Json& params) {
            return onMainThread(runBatch, params);
        });
    }
}
