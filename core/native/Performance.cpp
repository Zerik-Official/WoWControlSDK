#include "core/native/Performance.h"
#include "offsets/OffsetsClient.h"

namespace WoW::Performance
{
    using GetFramerateFn = float(__cdecl*)();

    float GetFramerate()
    {
        auto getFramerate = reinterpret_cast<GetFramerateFn>(Offsets::Performance::GET_FRAMERATE);
        return getFramerate();
    }
}
