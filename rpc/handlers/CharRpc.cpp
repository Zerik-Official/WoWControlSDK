#include "CharRpc.h"
#include "client/login/LoginAPI.h"
#include "hooks/GlueHooks.h"

namespace Rpc
{
    using SDK::Json;

    static Json handleGetCharacters(const Json&)
    {
        WoW::Login::CharVector* chars = WoW::Login::GetChars();

        Json result;
        result["count"] = chars ? chars->size : 0;
        result["characters"] = Json::array();

        if (!chars || chars->size <= 0)
            return result;

        const char* base = reinterpret_cast<const char*>(chars->buf);
        for (int i = 0; i < chars->size; i++)
        {
            const WoW::Login::CharData* d = reinterpret_cast<const WoW::Login::CharData*>(
                base + i * 0x198);
            Json c;
            c["index"]  = i;
            c["name"]   = d->name;
            c["level"]  = (int)(unsigned char)d->level;
            c["race"]   = (int)(unsigned char)d->race;
            c["class"]  = (int)(unsigned char)d->class_;
            c["gender"] = (int)(unsigned char)d->gender;
            c["map"]    = d->map;
            c["zone"]   = d->zone;
            result["characters"].push_back(c);
        }
        return result;
    }

    static Json handleRefreshCharacters(const Json&)
    {
        Hooks::Glue::Post([]() {
            WoW::Login::RequestCharacterList();
        });
        Json ok;
        ok["ok"] = true;
        return ok;
    }

    void registerCharMethods(Runtime::MethodRegistry& registry)
    {
        registry.registerMethod("client.getCharacters", handleGetCharacters);
        registry.registerMethod("client.refreshCharacters", handleRefreshCharacters);
    }
}
