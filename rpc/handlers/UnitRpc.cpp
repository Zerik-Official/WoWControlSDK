#include "UnitRpc.h"
#include "hooks/GlueHooks.h"
#include "core/api/UnitAPI.h"
#include "utils/json/Json.h"

namespace Rpc
{
    using SDK::Json;

    static Json handleGet(const Json& params)
    {
        std::string token = params.value("token", "player");

        std::string raw = Hooks::Glue::Execute([token]() -> std::string {
            CoreAPI::UnitRef unit = CoreAPI::Unit::Get(token.c_str());
            Json j;
            j["exists"] = unit.exists();
            if (!unit.exists()) return j.dump();

            j["name"] = unit.getName();
            j["level"] = unit.getLevel();
            j["race"] = unit.getRace();
            j["class"] = unit.getClass();
            j["health"] = unit.getHealth();
            j["maxHealth"] = unit.getMaxHealth();
            j["power"] = unit.getPower();
            j["maxPower"] = unit.getMaxPower();
            j["dead"] = unit.isDead();
            j["inCombat"] = unit.isInCombat();
            j["moving"] = unit.isMoving();
            j["mounted"] = unit.isMounted();
            j["flying"] = unit.isFlying();
            j["swimming"] = unit.isSwimming();
            j["casting"] = unit.isCasting();
            j["channeling"] = unit.isChanneling();
            auto pos = unit.getPosition();
            Json p;
            p["x"] = pos.x;
            p["y"] = pos.y;
            p["z"] = pos.z;
            j["position"] = p;
            WoWGUID guid = unit.getGUID();
            j["guid"] = Json::array({guid.high, guid.low});
            WoWGUID target = unit.getTargetGUID();
            j["targetGuid"] = Json::array({target.high, target.low});
            return j.dump();
        });

        return Json::parse(raw);
    }

    static Json handleGetState(const Json& params)
    {
        std::string token = params.value("token", "player");

        std::string raw = Hooks::Glue::Execute([token]() -> std::string {
            CoreAPI::UnitRef unit = CoreAPI::Unit::Get(token.c_str());
            Json j;
            j["exists"] = unit.exists();
            if (!unit.exists()) return j.dump();

            j["dead"] = unit.isDead();
            j["inCombat"] = unit.isInCombat();
            j["moving"] = unit.isMoving();
            j["mounted"] = unit.isMounted();
            j["flying"] = unit.isFlying();
            j["swimming"] = unit.isSwimming();
            j["casting"] = unit.isCasting();
            j["channeling"] = unit.isChanneling();
            return j.dump();
        });

        return Json::parse(raw);
    }

    static Json handleGetPosition(const Json& params)
    {
        std::string token = params.value("token", "player");

        std::string raw = Hooks::Glue::Execute([token]() -> std::string {
            CoreAPI::UnitRef unit = CoreAPI::Unit::Get(token.c_str());
            Json j;
            j["exists"] = unit.exists();
            if (!unit.exists()) return j.dump();

            auto pos = unit.getPosition();
            Json p;
            p["x"] = pos.x;
            p["y"] = pos.y;
            p["z"] = pos.z;
            j["position"] = p;
            return j.dump();
        });

        return Json::parse(raw);
    }

    void registerUnitMethods(Runtime::MethodRegistry& registry)
    {
        registry.registerMethod("unit.get", handleGet);
        registry.registerMethod("unit.state", handleGetState);
        registry.registerMethod("unit.position", handleGetPosition);
    }
}
