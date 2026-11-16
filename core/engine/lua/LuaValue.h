#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace LuaEngine {

enum class LuaType { Nil, Bool, Number, String, Table, Function, Userdata, Thread, Ref };

struct LuaValue {
    LuaType type = LuaType::Nil;
    bool boolVal = false;
    double numVal = 0.0;
    std::string strVal;
    std::vector<LuaValue> keys;
    std::vector<LuaValue> items;
    bool isArray = false;
    bool cycle = false;
    bool truncated = false;
    int ref = 0;
};

struct ReadLimits {
    int maxDepth = 4;
    int maxEntries = 500;
};

struct BlockedAction {
    std::string event;
    std::string function;
};

struct LuaResult {
    bool ok = false;
    std::string error;
    std::string stack;
    std::vector<LuaValue> values;
    std::vector<BlockedAction> blocked;
    bool truncated = false;
};

struct CallTarget {
    std::string path;
    int ref = 0;
    std::string method;
};

enum class BridgeKind { Emit, Hook };

struct BridgeMessage {
    BridgeKind kind = BridgeKind::Emit;
    std::string name;
    std::string addon;
    std::vector<LuaValue> args;
    bool truncated = false;
};

struct BridgeBatch {
    std::vector<BridgeMessage> messages;
    uint32_t dropped = 0;
};

}
