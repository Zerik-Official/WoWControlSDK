#include "ConsoleHooks.h"
#include "base/DetourHelper.h"
#include "offsets/OffsetsConsole.h"
#include <cstdio>

namespace Hooks::Console
{

static PrintMessageFn s_original = nullptr;
static PrintMessageCallback s_callback = nullptr;
static bool s_initialized = false;
static CRITICAL_SECTION s_logLock;
static bool s_lockInitialized = false;
static volatile bool s_inCallback = false;

static void __cdecl PrintMessage_Hook(const char* text, int style)
{
    if (s_inCallback) {
        s_original(text, style);
        return;
    }

    s_original(text, style);
    if (s_callback && text)
    {
        s_inCallback = true;
        EnterCriticalSection(&s_logLock);
        __try {
            s_callback(text, style);
        }
        __finally {
            LeaveCriticalSection(&s_logLock);
            s_inCallback = false;
        }
    }
}

void Initialize()
{
    if (s_initialized) return;

    if (!s_lockInitialized) {
        InitializeCriticalSection(&s_logLock);
        s_lockInitialized = true;
    }

    s_original = (PrintMessageFn)Offsets::Console::PRINT_MESSAGE;

    Detail::attach(s_original, PrintMessage_Hook);

    s_initialized = true;
}

void Shutdown()
{
    if (!s_initialized) return;

    Detail::detach(s_original, PrintMessage_Hook);

    s_initialized = false;
}

void SetCallback(PrintMessageCallback callback)
{
    s_callback = callback;
}

}
