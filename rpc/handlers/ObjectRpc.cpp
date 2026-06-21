#include "ObjectRpc.h"
#include "hooks/GlueHooks.h"
#include "core/api/ObjectAPI.h"
#include "core/api/UnitAPI.h"
#include "core/api/UnitHandle.h"
#include "core/native/ObjectType.h"
#include "utils/json/Json.h"

namespace Rpc
{
    using SDK::Json;

    static Json handleExists(const Json& params)
    {
        std::string token = params.value("token", "player");

        std::string raw = Hooks::Glue::Execute([token]() -> std::string {
            Json j;
            CoreAPI::UnitRef unit = CoreAPI::Unit::Get(token.c_str());
            j["exists"] = CoreAPI::Object::Exists(unit.getHandle());
            return j.dump();
        });

        return Json::parse(raw);
    }

    static Json handleGetType(const Json& params)
    {
        std::string token = params.value("token", "player");

        std::string raw = Hooks::Glue::Execute([token]() -> std::string {
            Json j;
            CoreAPI::UnitRef unit = CoreAPI::Unit::Get(token.c_str());
            CoreAPI::UnitHandle h = unit.getHandle();

            if (!CoreAPI::Object::Exists(h))
            {
                j["exists"] = false;
                j["type"] = "none";
                j["typeId"] = 0;
                return j.dump();
            }

            WoW::ObjectType t = CoreAPI::Object::GetType(h);
            j["exists"] = true;
            j["typeId"] = static_cast<int>(t);

            switch (t)
            {
                case WoW::ObjectType::Object:        j["type"] = "object";        break;
                case WoW::ObjectType::Item:          j["type"] = "item";          break;
                case WoW::ObjectType::Container:     j["type"] = "container";     break;
                case WoW::ObjectType::Unit:          j["type"] = "unit";          break;
                case WoW::ObjectType::Player:        j["type"] = "player";        break;
                case WoW::ObjectType::GameObject:    j["type"] = "gameobject";    break;
                case WoW::ObjectType::DynamicObject: j["type"] = "dynamicobject"; break;
                case WoW::ObjectType::Corpse:        j["type"] = "corpse";        break;
                default:                             j["type"] = "unknown";       break;
            }
            return j.dump();
        });

        return Json::parse(raw);
    }

    static Json handleIsUnit(const Json& params)
    {
        std::string token = params.value("token", "player");

        std::string raw = Hooks::Glue::Execute([token]() -> std::string {
            Json j;
            CoreAPI::UnitRef unit = CoreAPI::Unit::Get(token.c_str());
            j["isUnit"] = CoreAPI::Object::IsUnit(unit.getHandle());
            return j.dump();
        });

        return Json::parse(raw);
    }

    static Json handleIsPlayer(const Json& params)
    {
        std::string token = params.value("token", "player");

        std::string raw = Hooks::Glue::Execute([token]() -> std::string {
            Json j;
            CoreAPI::UnitRef unit = CoreAPI::Unit::Get(token.c_str());
            j["isPlayer"] = CoreAPI::Object::IsPlayer(unit.getHandle());
            return j.dump();
        });

        return Json::parse(raw);
    }

    void registerObjectMethods(Runtime::MethodRegistry& registry)
    {
        registry.registerMethod("object.exists",  handleExists);
        registry.registerMethod("object.getType", handleGetType);
        registry.registerMethod("object.isUnit",  handleIsUnit);
        registry.registerMethod("object.isPlayer", handleIsPlayer);
    }
}
