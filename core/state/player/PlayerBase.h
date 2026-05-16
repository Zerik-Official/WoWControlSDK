#pragma once
#include <cstdint>

namespace PlayerBase {

struct Info {
    uintptr_t objectBase;
    uintptr_t descPtr;
    uint64_t  guid;
    char      name[32];
    int       level;
    int       race;
    int       class_;
    int       gender;
    int       powerType;
    int       xp;
    int       xpMax;
    bool      valid;
};

Info read();

}