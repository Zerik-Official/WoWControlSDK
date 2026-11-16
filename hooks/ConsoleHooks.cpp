#include "ConsoleHooks.h"
#include "base/DetourHelper.h"
#include "offsets/OffsetsConsole.h"

namespace Hooks::Console
{

static PrintMessageFn s_original = nullptr;
static PrintMessageCallback s_callback = nullptr;
static bool s_initialized = false;
static thread_local bool t_inCallback = false;

static void __cdecl PrintMessage_Hook(const char* text, int style)
{
    s_original(text, style);

    if (t_inCallback || !s_callback || !text)
        return;

    t_inCallback = true;
    s_callback(text, style);
    t_inCallback = false;
}

void Initialize()
{
    if (s_initialized) return;

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
