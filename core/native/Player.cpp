#include "core/native/Player.h"
#include "memory/MemReader.h"
#include "core/native/ObjectManager.h"
#include "offsets/OffsetsPlayer.h"

Player Player::local()
{
    WoWGUID guid = WoW::GetLocalGUID();

    if (!guid.isValid())
        return Player(0);

    return Player(WoW::GetObjectByGUID(guid));
}

int Player::getXP() const
{
    if (!isValid())
        return 0;

    uintptr_t ptr = Memory::safeRead<uintptr_t>(m_base + 0x1008);
    if (!ptr)
        return 0;

    return Memory::safeRead<int>(ptr + 0x798);
}

int Player::getMaxXP() const
{
    if (!isValid())
        return 0;

    uintptr_t ptr = Memory::safeRead<uintptr_t>(m_base + 0x1008);
    if (!ptr)
        return 0;

    return Memory::safeRead<int>(ptr + 0x79C);
}

float Player::getRotation() const
{
    if (!isValid())
        return 0.0f;

    return Memory::safeRead<float>(m_base + Offsets::Player::ROT_OFFSET);
}

float Player::getX() const
{
    if (!isValid())
        return 0.0f;

    return Memory::safeRead<float>(m_base + Offsets::Player::POS_OFFSET);
}

float Player::getY() const
{
    if (!isValid())
        return 0.0f;

    return Memory::safeRead<float>(m_base + Offsets::Player::POS_OFFSET + 0x4);
}

float Player::getZ() const
{
    if (!isValid())
        return 0.0f;

    return Memory::safeRead<float>(m_base + Offsets::Player::POS_OFFSET + 0x8);
}
