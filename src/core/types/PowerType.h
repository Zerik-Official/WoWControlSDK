#pragma once
#include <cstdint>

namespace WoW {

enum class PowerType : uint8_t
{
    Mana       = 0,
    Rage       = 1,
    Focus      = 2,
    Energy     = 3,
    Happiness  = 4,
    Rune       = 5,
    RunicPower = 6,
    Unknown    = 0xFF
};

inline const char* powerTypeName(PowerType type)
{
    switch (type)
    {
    case PowerType::Mana:       return "MANA";
    case PowerType::Rage:       return "RAGE";
    case PowerType::Focus:      return "FOCUS";
    case PowerType::Energy:     return "ENERGY";
    case PowerType::Happiness:  return "HAPPINESS";
    case PowerType::Rune:       return "RUNE";
    case PowerType::RunicPower: return "RUNIC_POWER";
    default:                    return "UNKNOWN";
    }
}

}