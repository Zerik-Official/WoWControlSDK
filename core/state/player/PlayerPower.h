#pragma once
#include <cstdint>

namespace PlayerPower {

enum class Type {
    MANA        = 0,
    RAGE        = 1,
    ENERGY      = 3,
    RUNIC_POWER = 6,
    UNKNOWN     = -1
};

struct Info {
    Type powerType;
    int  current;
    int  max;
};

Info read(uintptr_t unitDesc, int powerType);

}