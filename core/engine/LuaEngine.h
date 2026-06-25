#pragma once
#include <string>
#include <vector>

namespace LuaEngine {

enum class LuaType { Nil, Bool, Number, String };

struct LuaValue {
    LuaType type = LuaType::Nil;
    bool boolVal = false;
    double numVal = 0.0;
    std::string strVal;
};

struct LuaResult {
    bool ok = false;
    std::string error;
    std::vector<LuaValue> values;
};

bool     initialize();
void     shutdown();

void     execute(const std::string& code);

LuaResult evaluate(const std::string& code);

LuaResult getGlobal(const std::string& name);
void     setGlobal(const std::string& name, const LuaValue& value);
void     createTable(const std::string& name);

}
