#include "core/native/FrameScript.h"
#include "hooks/EventHooks.h"
#include "offsets/OffsetsSocial.h"

namespace FrameScript
{

void FireEventById(int eventId)
{
    if (eventId < 0)
        return;

    using FireEventFn = void(__cdecl*)(int, const char*, ...);
    reinterpret_cast<FireEventFn>(Offsets::FrameScript::FIRE_EVENT)(eventId, "");
}

bool FireEvent(const char* eventName)
{
    int eventId = Hooks::Events::GetEventId(eventName);
    if (eventId < 0)
        return false;

    FireEventById(eventId);
    return true;
}

}
