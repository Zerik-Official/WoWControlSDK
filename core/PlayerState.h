#pragma once
#include <string>

namespace PlayerState {

struct Info {
    float    x;
    float    y;
    float    z;
    int      health;
    int      healthMax;
    int      mana;
    int      manaMax;
    int      level;
    int      race;
    int      class_;
    int      gender;
};

bool  available();
Info  read();
char* toJson();

}