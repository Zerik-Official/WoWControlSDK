#include "BuildInfo.h"
#include "BuildVersion.h"

namespace SDK::BuildInfo
{
    const char* version()
    {
        return SDK_VERSION;
    }

    const char* commitShort()
    {
        return SDK_COMMIT_SHORT;
    }

    const char* commitDate()
    {
        return SDK_COMMIT_DATE;
    }

    bool dirty()
    {
        return SDK_DIRTY != 0;
    }

    Json toJson()
    {
        Json j;
        j["version"] = SDK_VERSION;
        if (SDK_COMMIT[0] != '\0')
        {
            Json commit;
            commit["hash"] = SDK_COMMIT;
            commit["shortHash"] = SDK_COMMIT_SHORT;
            commit["date"] = SDK_COMMIT_DATE;
            commit["timestamp"] = static_cast<int64_t>(SDK_COMMIT_TIME);
            commit["dirty"] = SDK_DIRTY != 0;
            j["commit"] = commit;
        }
        else
        {
            j["commit"] = nullptr;
        }
        return j;
    }
}
