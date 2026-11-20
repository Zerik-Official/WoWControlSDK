#pragma once

#include <string>

namespace LoadingScreen
{
    struct Background
    {
        bool        found         = false;
        int         id            = 0;
        std::string name;
        std::string file;
        bool        hasWideScreen = false;
    };

    bool       isActive();
    float      getProgress();
    int        getMapId();
    Background getBackground(int mapId);
}
