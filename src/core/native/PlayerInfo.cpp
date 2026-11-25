#include "core/native/PlayerInfo.h"
#include "core/native/NameCache.h"
#include "core/api/FriendsAPI.h"
#include "core/api/GroupAPI.h"
#include <cctype>
#include <cstdlib>

namespace PlayerInfo
{

namespace
{
    bool sameName(const std::string& left, const std::string& right)
    {
        if (left.size() != right.size())
            return false;
        for (size_t i = 0; i < left.size(); ++i)
        {
            if (std::tolower(static_cast<unsigned char>(left[i])) != std::tolower(static_cast<unsigned char>(right[i])))
                return false;
        }
        return true;
    }
}

WoWGUID parseGuid(const std::string& text)
{
    if (text.empty())
        return NULL_GUID;
    uint64_t raw = std::strtoull(text.c_str(), nullptr, 16);
    WoWGUID guid;
    guid.low  = static_cast<uint32_t>(raw & 0xFFFFFFFFull);
    guid.high = static_cast<uint32_t>(raw >> 32);
    return guid;
}

WoWGUID findByName(const std::string& name)
{
    std::string bare = name.substr(0, name.find('-'));
    if (bare.empty())
        return NULL_GUID;

    for (const CoreAPI::FriendInfo& f : CoreAPI::Friends::GetList().friends)
    {
        if (sameName(f.name, bare))
            return f.guid;
    }
    for (const CoreAPI::GroupMember& m : CoreAPI::Group::GetGroup().members)
    {
        if (m.nameKnown && sameName(m.name, bare))
            return m.guid;
    }
    return NULL_GUID;
}

SDK::Json toJson(const WoWGUID& guid)
{
    if (!guid.isValid() || !guid.isPlayer())
        return nullptr;

    NameCache::Entry entry = NameCache::lookup(guid);
    if (!entry.found || entry.race <= 0)
        return nullptr;

    SDK::Json j;
    j["race"]    = entry.race;
    j["gender"]  = entry.gender;
    j["classId"] = entry.class_;
    return j;
}

SDK::Json toJsonByName(const std::string& name)
{
    return toJson(findByName(name));
}

}
