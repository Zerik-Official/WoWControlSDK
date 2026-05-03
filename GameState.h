#pragma once
#include <Windows.h>
#include <cstdint>

namespace GameState {

    enum class Screen {
        UNKNOWN,
        LOGIN,
        CHARSELECT,
        WORLD
    };

    void        initialize();

    char*       getStateJson();

    const char* getRealmList();

}