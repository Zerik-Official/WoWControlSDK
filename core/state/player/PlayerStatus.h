#pragma once
#include <cstdint>

namespace PlayerStatus {

struct Info {
    bool isIngame;
    bool isWorld;
    bool isLoading;
    bool isReady;
    bool isDead;
    bool isGhost;
    bool isMounted;
    bool isFlying;
    bool isSwimming;
    bool isUnderwater;
    bool isAfk;
    bool isDnd;
    bool isInCombat;
};

Info read(uintptr_t objectBase, uintptr_t unitDesc);

}