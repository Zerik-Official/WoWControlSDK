#include "GroupCombatState.h"

namespace GroupCombatState {

Info evaluate(const GroupState::Info& group)
{
    Info info = {};

    if (!group.valid) return info;

    for (const GroupMemberState::Info& member : group.members)
    {
        if (!member.valid) continue;

        if (member.isInCombat)
        {
            info.anyMemberInCombat = true;
            info.membersInCombat++;
        }

        if (member.isDead)
        {
            info.anyMemberDead = true;
            info.membersDead++;
        }

        if (member.isCasting || member.isChanneling)
            info.anyMemberCasting = true;

        if (member.isInCombat || member.isAutoAttacking || member.isCasting)
            info.anyMemberEngaging = true;
    }

    return info;
}

}