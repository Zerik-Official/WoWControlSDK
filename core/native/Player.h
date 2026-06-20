#pragma once

#include "core/native/Unit.h"

class Player : public Unit
{
public:
    using Unit::Unit;

    static Player local();

    int   getXP()        const;
    int   getMaxXP()     const;
    float getX()         const;
    float getY()         const;
    float getZ()         const;
    float getRotation()  const;
};
