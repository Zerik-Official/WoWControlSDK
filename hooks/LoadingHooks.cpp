#include "LoadingHooks.h"
#include <Windows.h>
#include "offsets/OffsetsLoading.h"
#include "offsets/OffsetsClient.h"
#include "runtime/events/EventPipe.h"
#include "utils/json/Json.h"

namespace Hooks::Loading
{
    static bool     s_loading             = false;
    static int      s_lastFlag           = -1;
    static float    s_lastEmittedProgress = -1.0f;
    static uint64_t s_lastProgressTime    = 0;
    static bool     s_lastInWorld         = false;
    static uint64_t s_startupTime         = 0;
    static float    s_firstProgress       = -1.0f;
    static bool     s_hadRealProgress     = false;
    static float    s_idleProgress        = -1.0f;

    void PollFlag()
    {
        int flag = *(int*)Offsets::Loading::LOADING_FLAG;
        bool inWorld = *(bool*)Offsets::Client::IN_WORLD_FLAG;
        uint64_t now = GetTickCount64();

        if (!s_loading && flag != -1 && !inWorld)
        {
            bool startup = false;

            if (flag != s_lastFlag)
                startup = true;

            else if (s_lastInWorld)
                startup = true;

            else
            {
                float prog = *(float*)Offsets::Loading::PROGRESS_VALUE;
                if (prog < 1.0f && prog != s_idleProgress && s_idleProgress != -1.0f)
                    startup = true;
                s_idleProgress = prog;
            }

            if (startup)
            {
                s_loading = true;
                s_startupTime = now;
                s_lastEmittedProgress = -1.0f;
                s_lastProgressTime = now;
                s_firstProgress = *(float*)Offsets::Loading::PROGRESS_VALUE;
                s_hadRealProgress = false;
                s_idleProgress = -1.0f;
                SDK::Json data;
                Runtime::EventPipe::PushEvent("loading.startup", data, now);
            }
        }

        if (s_loading && now - s_lastProgressTime >= 100)
        {
            float progress = *(float*)Offsets::Loading::PROGRESS_VALUE;
            if (progress != s_lastEmittedProgress)
            {
                if (progress != s_firstProgress)
                    s_hadRealProgress = true;
                s_lastEmittedProgress = progress;
                s_lastProgressTime = now;
                SDK::Json data;
                data["progress"] = progress;
                Runtime::EventPipe::PushEvent("loading.progress", data, now);
            }
        }

        if (inWorld && !s_lastInWorld && s_loading)
        {
            s_loading = false;
            s_lastEmittedProgress = -1.0f;
            SDK::Json data;
            Runtime::EventPipe::PushEvent("loading.finished", data, now);
        }

        if (s_loading && !inWorld && now - s_startupTime > 5000 && !s_hadRealProgress)
        {
            s_loading = false;
            s_lastFlag = -1;
            s_lastEmittedProgress = -1.0f;
        }

        s_lastFlag = flag;
        s_lastInWorld = inWorld;
    }

    void Initialize() {}
    void Shutdown() {}
}
