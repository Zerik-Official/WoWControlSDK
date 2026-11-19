#include "LuaBlocked.h"
#include "LuaBridge.h"
#include <atomic>
#include <mutex>

namespace LuaBlocked {

static Capture* s_active = nullptr;
static std::atomic<bool> s_showNotice{false};
static std::mutex s_pendingMutex;
static std::vector<LuaEngine::BlockedAction> s_pending;

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
    collect();
    return std::move(m_actions);
}

void collect()
{
    for (auto& action : LuaBridge::takeBlocked())
    {
        report(action.event, action.function);
        std::lock_guard<std::mutex> lock(s_pendingMutex);
        s_pending.push_back(std::move(action));
    }
}

std::vector<LuaEngine::BlockedAction> takeReported()
{
    std::lock_guard<std::mutex> lock(s_pendingMutex);
    return std::move(s_pending);
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
