#pragma once
#include <vector>

struct CachedChar {
    char name[49];
    int  level;
    int  class_;
    int  race;
    int  gender;
    int  map;
    int  zone;
};

namespace CharCache {

void               refresh();
void               invalidate();
const std::vector<CachedChar>& get();

}