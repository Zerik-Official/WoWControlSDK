#include "CharRpc.h"
#include "core/native/CharacterReader.h"
#include "hooks/GlueHooks.h"
#include "client/login/LoginAPI.h"
#include "utils/json/Json.h"

namespace Rpc
{
    using SDK::Json;

    static Json handleGetCharacters(const Json&)
    {
        auto chars = WoW::Characters::GetList();

        Json result;
        result["count"] = (int)chars.size();
        result["characters"] = Json::array();

        for (const auto& c : chars)
        {
            Json jc;
            jc["index"]  = c.index;
            jc["name"]   = c.name;
            jc["level"]  = c.level;
            jc["race"]   = c.race;
            jc["class"]  = c.class_;
            jc["gender"] = c.gender;
            jc["map"]    = c.map;
            jc["zone"]   = c.zone;
            result["characters"].push_back(jc);
        }
        return result;
    }

    static Json handleRefreshCharacters(const Json&)
    {
        Hooks::Glue::Post([]() {
            WoW::Login::RequestCharacterList();
        });
        return SDK::okJson();
    }

    void registerCharMethods(Runtime::MethodRegistry& registry)
    {
        registry.registerMethod("client.getCharacters", handleGetCharacters);
        registry.registerMethod("client.refreshCharacters", handleRefreshCharacters);
    }
}
