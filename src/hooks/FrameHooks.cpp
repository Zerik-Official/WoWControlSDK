#include "FrameHooks.h"
#include "base/GameScheduler.h"
#include "utils/TaskQueue.h"

namespace Hooks::Frame
{

static constexpr float FRAME_PRIORITY = 1.0f;

static std::function<void()> s_onFrame = nullptr;
static bool s_initialized = false;

static Utils::TaskQueue s_queue;

static void OnIdle()
{
    if (s_onFrame)
        s_onFrame();

    s_queue.drainAll();
}

void Initialize()
{
    if (s_initialized) return;

    GameScheduler::Initialize();
    GameScheduler::addIdleHandler(OnIdle, FRAME_PRIORITY);

    s_initialized = true;
}

void Shutdown()
{
    if (!s_initialized) return;

    GameScheduler::Shutdown();

    s_initialized = false;
}

void SetOnFrame(std::function<void()> callback)
{
    s_onFrame = callback;
}

std::string Execute(Task task, DWORD timeoutMs)
{
    return s_queue.execute(std::move(task), timeoutMs);
}

}
