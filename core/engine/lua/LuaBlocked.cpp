#include "LuaBlocked.h"
#include <atomic>

namespace LuaBlocked {

static Capture* s_active = nullptr;
static std::atomic<bool> s_showNotice{false};

Capture::Capture()
    : m_previous(s_active)
{
    s_active = this;
}

Capture::~Capture()
{
    s_active = m_previous;
}

std::vector<LuaEngine::BlockedAction> Capture::take()
{
    return std::move(m_actions);
}

void report(const std::string& event, const std::string& function)
{
    if (s_active)
        s_active->m_actions.push_back({event, function});
}

void apply(LuaEngine::LuaResult& result, Capture& capture)
{
    result.blocked = capture.take();
    if (result.blocked.empty() || !result.ok)
        return;

    result.ok = false;
    result.error = "blocked by the game: " + result.blocked.front().function;
}

void setShowNotice(bool show)
{
    s_showNotice.store(show);
}

bool showNotice()
{
    return s_showNotice.load();
}

}
