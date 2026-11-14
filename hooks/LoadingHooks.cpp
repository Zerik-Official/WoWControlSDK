#include "LoadingHooks.h"
#include <Windows.h>
#include "core/native/ClientState.h"
#include "core/native/LoadingScreen.h"
#include "runtime/events/EventPipe.h"
#include "utils/json/Json.h"

namespace Hooks::Loading
{
    static bool     s_active              = false;
    static int      s_mapId               = -1;
    static float    s_lastEmittedProgress = -1.0f;
    static uint64_t s_lastProgressTime    = 0;
    static bool     s_awaitingWorld       = false;
    static uint64_t s_closedTime          = 0;

    static constexpr uint64_t PROGRESS_INTERVAL_MS = 100;
    static constexpr uint64_t WORLD_GRACE_MS       = 3000;

    static SDK::Json backgroundJson(int mapId)
    {
        LoadingScreen::Background background = LoadingScreen::getBackground(mapId);
        if (!background.found)
            return SDK::Json();

        SDK::Json data;
        data["id"]            = background.id;
        data["name"]          = background.name;
        data["file"]          = background.file;
        data["hasWideScreen"] = background.hasWideScreen;
        return data;
    }

    static void pushEnd(const char* eventName, uint64_t now)
    {
        SDK::Json data;
        data["mapId"] = s_mapId;
        Runtime::EventPipe::PushEvent(eventName, data, now);
        s_awaitingWorld = false;
        s_mapId = -1;
    }

    void PollFlag()
    {
        bool     active  = LoadingScreen::isActive();
        bool     inWorld = WoW::IsInWorld();
        uint64_t now     = GetTickCount64();

        if (active && !s_active)
        {
            s_awaitingWorld       = false;
            s_mapId               = LoadingScreen::getMapId();
            s_lastEmittedProgress = -1.0f;
            s_lastProgressTime    = 0;

            SDK::Json data;
            data["mapId"]      = s_mapId;
            data["background"] = backgroundJson(s_mapId);
            Runtime::EventPipe::PushEvent("loading.startup", data, now);
        }

        if (active && now - s_lastProgressTime >= PROGRESS_INTERVAL_MS)
        {
            float progress = LoadingScreen::getProgress();
            if (progress != s_lastEmittedProgress)
            {
                s_lastEmittedProgress = progress;
                s_lastProgressTime    = now;
                SDK::Json data;
                data["progress"] = progress;
                Runtime::EventPipe::PushEvent("loading.progress", data, now);
            }
        }

        if (!active && s_active)
        {
            if (inWorld)
                pushEnd("loading.finished", now);
            else
            {
                s_awaitingWorld = true;
                s_closedTime    = now;
            }
        }

        if (s_awaitingWorld && !active)
        {
            if (inWorld)
                pushEnd("loading.finished", now);
            else if (now - s_closedTime >= WORLD_GRACE_MS)
                pushEnd("loading.cancelled", now);
        }

        s_active = active;
    }

    void Initialize() {}
    void Shutdown() {}
}
