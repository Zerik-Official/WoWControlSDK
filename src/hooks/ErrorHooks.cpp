#include "ErrorHooks.h"
#include "core/native/CrashReport.h"
#include "runtime/diagnostics/CrashMonitor.h"
#include <atomic>

namespace Hooks::Errors
{

static bool s_registered = false;
static std::atomic<bool> s_active{false};

static std::string text(const char* value)
{
    return value ? value : "";
}

static int __stdcall FatalError_handler(DWORD code, const char* title, const char* description, int, const char* details)
{
    if (!s_active.exchange(false))
        return 1;

    Runtime::CrashMonitor::FatalError error;
    error.code = code;
    error.title = text(title);
    error.description = text(description);
    error.details = text(details);
    Runtime::CrashMonitor::Report(error);
    return 1;
}

void Initialize()
{
    if (s_registered) return;

    WoW::CrashReport::RegisterHandler(FatalError_handler);
    s_registered = true;
    s_active = true;
}

void Shutdown()
{
    s_active = false;
}

}
