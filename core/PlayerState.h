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
    bool isLoading;
    bool isReady;
};

bool  available();
Info  read();
char* toJson();

}