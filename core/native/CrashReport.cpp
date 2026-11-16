#include "core/native/CrashReport.h"
#include "offsets/OffsetsClient.h"

namespace WoW::CrashReport
{
    using RegisterHandlerFn = void(__cdecl*)(FatalErrorHandler handler);
    using GetLogPathFn      = bool(__cdecl*)(char* buffer, size_t size);

    void RegisterHandler(FatalErrorHandler handler)
    {
        auto registerHandler = reinterpret_cast<RegisterHandlerFn>(Offsets::Errors::REGISTER_HANDLER);
        registerHandler(handler);
    }

    std::string GetLogPath()
    {
        char buffer[MAX_PATH] = {};
        auto getLogPath = reinterpret_cast<GetLogPathFn>(Offsets::Errors::GET_LOG_PATH);
        if (!getLogPath(buffer, sizeof(buffer)))
            return {};
        return buffer;
    }

    const EXCEPTION_POINTERS* GetExceptionPointers()
    {
        return *reinterpret_cast<EXCEPTION_POINTERS**>(Offsets::Errors::EXCEPTION_POINTERS);
    }
}
