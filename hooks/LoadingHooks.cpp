#include "LoadingHooks.h"
#include <Windows.h>
#include <deps/Detours/detours.h>
#include "offsets/OffsetsLoading.h"
#include "runtime/events/EventPipe.h"
#include "utils/json/Json.h"

namespace Hooks::Loading
{
    static bool s_initialized = false;

    // Tracked loading state
    static int      s_lastFlag          = -1;
    static float    s_lastEmittedProgress = -1.0f;
    static uint64_t s_lastProgressTime  = 0;

    using RenderFrameFn = void(__cdecl*)();
    static RenderFrameFn s_renderFrame_orig = nullptr;

    static void __cdecl RenderFrame_hook()
    {
        PollFlag();
        s_renderFrame_orig();
    }

    void PollFlag()
    {
        int flag = *(int*)Offsets::Loading::LOADING_FLAG;
        uint64_t now = GetTickCount64();

        if (flag != -1 && s_lastFlag == -1)
        {
            SDK::Json data;
            Runtime::EventPipe::PushEvent("loading.startup", data, now);
        }

        if (flag == -1 && s_lastFlag != -1)
        {
            SDK::Json data;
            Runtime::EventPipe::PushEvent("loading.finished", data, now);
        }

        if (flag != -1)
        {
            if (now - s_lastProgressTime >= 100)
            {
                float progress = *(float*)Offsets::Loading::PROGRESS_VALUE;
                if (progress != s_lastEmittedProgress)
                {
                    s_lastEmittedProgress = progress;
                    s_lastProgressTime = now;
                    SDK::Json data;
                    data["progress"] = progress;
                    Runtime::EventPipe::PushEvent("loading.progress", data, now);
                }
            }
        }

        s_lastFlag = flag;
    }

    void Initialize()
    {
        if (s_initialized) return;

        s_renderFrame_orig = (RenderFrameFn)Offsets::Loading::RENDER_FRAME;

        DetourAttach(&(void*&)s_renderFrame_orig, RenderFrame_hook);

        s_initialized = true;
    }

    void Shutdown()
    {
        if (!s_initialized) return;

        DetourDetach(&(void*&)s_renderFrame_orig, RenderFrame_hook);

        s_initialized = false;
    }
}
