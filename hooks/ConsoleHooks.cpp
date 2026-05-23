#include "ConsoleHooks.h"
#include <deps/Detours/detours.h>
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

    s_original = (PrintMessageFn)0x00765270;

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(&(void*&)s_original, PrintMessage_Hook);
    DetourTransactionCommit();

    s_initialized = true;
}

void Shutdown()
{
    if (!s_initialized) return;

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourDetach(&(void*&)s_original, PrintMessage_Hook);
    DetourTransactionCommit();

    s_initialized = false;
}

void SetCallback(PrintMessageCallback callback)
{
    s_callback = callback;
}

void PrintMessage(const char* text, int style)
{
    if (s_original && text)
        s_original(text, style);
}

PrintMessageFn GetOriginal()
{
    return s_original;
}

}
