#pragma once
#include "LuaValue.h"
#include <string>
#include <vector>

namespace LuaBlocked {

class Capture {
public:
    Capture();
    ~Capture();

    Capture(const Capture&) = delete;
    Capture& operator=(const Capture&) = delete;

    std::vector<LuaEngine::BlockedAction> take();

private:
    std::vector<LuaEngine::BlockedAction> m_actions;
    Capture* m_previous;

    friend void report(const std::string& event, const std::string& function);
};

void report(const std::string& event, const std::string& function);
void collect();
std::vector<LuaEngine::BlockedAction> takeReported();
void apply(LuaEngine::LuaResult& result, Capture& capture);

void setShowNotice(bool show);
bool showNotice();

}
