#pragma once
#include <cstdint>

namespace GroupMemberState {

struct Info {
    uint64_t  guid;
    uintptr_t objectBase;
    uintptr_t descPtr;
    char      name[32];
    int       level;
    int       race;
    int       class_;
    int       gender;
    int       powerType;
    int       health;
    int       healthMax;
    int       power;
    int       powerMax;
    uint64_t  targetGuid;
    bool      isInCombat;
    bool      isDead;
    bool      isCasting;
    bool      isChanneling;
    bool      isAutoAttacking;
    bool      valid;
};

Info read(uint64_t guid);

}