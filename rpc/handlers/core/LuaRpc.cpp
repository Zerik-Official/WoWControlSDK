#include "LuaRpc.h"
#include "core/engine/LuaEngine.h"
#include "utils/json/Json.h"

namespace Rpc
{
    using SDK::Json;

    static Json handleExecute(const Json& params)
    {
        std::string code = params.value("code", "");
        if (code.empty())
            return SDK::makeErrorJson("code is required");

        LuaEngine::execute(code);
        return SDK::okJson();
    }

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

    static Json handleEvaluate(const Json& params)
    {
        std::string code = params.value("code", "");
        if (code.empty())
            return SDK::makeErrorJson("code is required");

        auto result = LuaEngine::evaluate(code);
        if (!result.ok)
        {
            Json err;
            err["error"] = result.error;
            return err;
        }

        Json j;
        j["ok"] = true;
        if (result.values.size() == 1)
        {
            j["result"] = valueToJson(result.values[0]);
        }
        else if (result.values.size() > 1)
        {
            Json arr = Json::array();
            for (const auto& v : result.values)
                arr.push_back(valueToJson(v));
            j["result"] = arr;
        }
        else
        {
            j["result"] = nullptr;
        }
        return j;
    }

    void registerLuaMethods(Runtime::MethodRegistry& registry)
    {
        registry.registerMethod("lua.execute", handleExecute);
        registry.registerMethod("lua.evaluate", handleEvaluate);
    }
}
