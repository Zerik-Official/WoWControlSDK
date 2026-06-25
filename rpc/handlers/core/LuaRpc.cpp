#include "LuaRpc.h"
#include "core/engine/LuaEngine.h"
#include "utils/json/Json.h"

namespace Rpc
{
    using SDK::Json;

    static Json valueToJson(const LuaEngine::LuaValue& v)
    {
        switch (v.type)
        {
            case LuaEngine::LuaType::Bool:   return v.boolVal;
            case LuaEngine::LuaType::Number: return v.numVal;
            case LuaEngine::LuaType::String: return v.strVal;
            default:                         return Json();
        }
    }

    static Json resultToJson(const LuaEngine::LuaResult& r)
    {
        Json j;
        j["ok"] = r.ok;
        if (!r.ok)
        {
            j["error"] = r.error;
        }
        else
        {
            if (r.values.size() == 1)
            {
                j["result"] = valueToJson(r.values[0]);
            }
            else if (r.values.size() > 1)
            {
                Json arr = Json::array();
                for (const auto& v : r.values)
                    arr.push_back(valueToJson(v));
                j["result"] = arr;
            }
            else
            {
                j["result"] = nullptr;
            }
        }
        return j;
    }

    static Json handleExecute(const Json& params)
    {
        std::string code = params.value("code", "");
        if (code.empty())
            return SDK::makeErrorJson("code is required");

        LuaEngine::execute(code);
        return SDK::okJson();
    }

    static Json handleEvaluate(const Json& params)
    {
        std::string code = params.value("code", "");
        if (code.empty())
            return SDK::makeErrorJson("code is required");

        return resultToJson(LuaEngine::evaluate(code));
    }

    static Json handleGetGlobal(const Json& params)
    {
        std::string name = params.value("name", "");
        if (name.empty())
            return SDK::makeErrorJson("name is required");

        return resultToJson(LuaEngine::evaluate("return " + name));
    }

    static Json handleSetGlobal(const Json& params)
    {
        std::string name = params.value("name", "");
        if (name.empty())
            return SDK::makeErrorJson("name is required");

        LuaEngine::LuaValue v;
        if (params.contains("value"))
        {
            const auto& val = params["value"];
            if (val.is_boolean())
            {
                v.type = LuaEngine::LuaType::Bool;
                v.boolVal = val.get<bool>();
            }
            else if (val.is_number())
            {
                v.type = LuaEngine::LuaType::Number;
                v.numVal = val.get<double>();
            }
            else if (val.is_string())
            {
                v.type = LuaEngine::LuaType::String;
                v.strVal = val.get<std::string>();
            }
        }

        LuaEngine::setGlobal(name, v);
        return SDK::okJson();
    }

    static Json handleCreateNamespace(const Json& params)
    {
        std::string name = params.value("name", "");
        if (name.empty())
            return SDK::makeErrorJson("name is required");

        LuaEngine::createTable(name);
        return SDK::okJson();
    }

    void registerLuaMethods(Runtime::MethodRegistry& registry)
    {
        registry.registerMethod("lua.execute", handleExecute);
        registry.registerMethod("lua.evaluate", handleEvaluate);
        registry.registerMethod("lua.getGlobal", handleGetGlobal);
        registry.registerMethod("lua.setGlobal", handleSetGlobal);
        registry.registerMethod("lua.createNamespace", handleCreateNamespace);
    }
}
