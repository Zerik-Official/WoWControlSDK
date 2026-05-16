#pragma once
#include "GroupState.h"

namespace GroupCombatState {

struct Info {
    bool anyMemberInCombat;
    bool anyMemberDead;
    bool anyMemberCasting;
    bool anyMemberEngaging;
    int  membersInCombat;
    int  membersDead;
};

Info evaluate(const GroupState::Info& group);

}