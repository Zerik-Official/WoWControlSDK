#pragma once
#include <cstdint>

namespace PlayerState {

struct Info {
    float    x;
    float    y;
    float    z;
    float    rotation;
    int      health;
    int      healthMax;
    int      mana;
    int      manaMax;
    int      rage;
    int      rageMax;
    int      energy;
    int      energyMax;
    int      runicPower;
    int      runicPowerMax;
    int      level;
    int      race;
    int      class_;
    int      gender;
    int      powerType;
    int      xp;
    int      xpMax;
    int      mapId;
    int      zoneId;
    int      tick;
    bool     isIngame;
    bool     isWorld;
    char     name[32];
    bool     isLoading;
    bool     isReady;
    bool     isDead;
    bool     isGhost;
    bool     isMounted;
    bool     isFlying;
    bool     isSwimming;
    bool     isUnderwater;
    bool     isAfk;
    bool     isDnd;
    bool     isInCombat;
};

bool  available();
Info  read();
char* toJson();

}