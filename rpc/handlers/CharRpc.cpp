#include "CharRpc.h"
#include "client/login/LoginAPI.h"

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

        // stride is 0x198 bytes per entry (CharData is 0x188, plus 0x10 padding)
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

    void registerCharMethods(Runtime::MethodRegistry& registry)
    {
        registry.registerMethod("client.getCharacters", handleGetCharacters);
    }
}
