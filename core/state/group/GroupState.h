#pragma once
#include "GroupMemberState.h"
#include <cstdint>
#include <vector>

namespace GroupState {

enum class GroupType {
    NONE,
    PARTY,
    RAID
};

struct Info {
    GroupType                       type;
    uint64_t                        leaderGuid;
    std::vector<GroupMemberState::Info> members;
    bool                            valid;
};

Info read();

}