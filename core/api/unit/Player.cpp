#include "Player.h"
#include "memory/MemReader.h"
#include "object/ObjectManager.h"
#include "OffsetsPlayer.h"
#include "OffsetsObjectMgr.h"

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

    return Memory::safeRead<int>(m_unitData + Offsets::ObjectMgr::OBJ_DESC_END + Offsets::Player::Desc::XP);
}

int Player::getMaxXP() const
{
    if (!isValid())
        return 0;

    return Memory::safeRead<int>(m_unitData + Offsets::ObjectMgr::OBJ_DESC_END + Offsets::Player::Desc::NEXT_LEVEL_XP);
}

float Player::getRotation() const
{
    if (!isValid())
        return 0.0f;

    return Memory::safeRead<float>(m_base + Offsets::Player::ROT_OFFSET);
}