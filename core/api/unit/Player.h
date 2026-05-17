#pragma once

#include "Unit.h"

class Player : public Unit
{
public:
    using Unit::Unit;

    static Player local();

    int   getXP()        const;
    int   getMaxXP()     const;
    float getRotation()  const;
};