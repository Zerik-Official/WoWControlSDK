#pragma once

#include "core/types/GUID.h"
#include <string>
#include <vector>
#include <cstdint>

namespace CoreAPI
{
    struct FriendInfo
    {
        int         index;
        WoWGUID     guid;
        std::string name;
        bool        online;
        uint8_t     flags;
        bool        afk;
        bool        dnd;
        bool        referAFriend;
        std::string note;
        int         level;
        int         classId;
        std::string className;
        std::string classFileName;
        int         areaId;
        std::string areaName;
    };

    struct FriendList
    {
        bool                    loaded;
        int                     count;
        int                     onlineCount;
        std::vector<FriendInfo> friends;
    };

    namespace Friends
    {
        FriendList GetList();
    }
}
