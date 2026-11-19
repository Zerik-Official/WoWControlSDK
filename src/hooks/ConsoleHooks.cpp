#include "ConsoleHooks.h"
#include "base/GameScheduler.h"
#include "memory/MemReader.h"
#include "offsets/OffsetsConsole.h"
#include <string>
#include <utility>
#include <vector>

namespace Hooks::Console
{

static constexpr float CONSOLE_PRIORITY = 1.0f;
static constexpr size_t MAX_LINES_PER_POLL = 512;

static PrintMessageCallback s_callback = nullptr;
static bool s_initialized = false;

static uintptr_t s_lastLine = 0;
static uintptr_t s_lastText = 0;

static bool isNode(uintptr_t link)
{
    return link != 0 && (link & 1) == 0;
}

static std::string readText(uintptr_t text)
{
    std::string result;
    for (size_t i = 0; i < 4096; ++i)
    {
        char c = Memory::safeRead<char>(text + i);
        if (c == 0) break;
        result.push_back(c);
    }
    return result;
}

static void PollConsole()
{
    if (!s_callback || Memory::safeRead<uint32_t>(Offsets::Console::CONSOLE_READY) == 0)
        return;

    auto lock = reinterpret_cast<LPCRITICAL_SECTION>(Offsets::Console::LINES_LOCK);
    std::vector<std::pair<std::string, int>> fresh;

    EnterCriticalSection(lock);
    uintptr_t input = Memory::safeRead<uintptr_t>(Offsets::Console::INPUT_LINE);
    uintptr_t newest = 0;
    uintptr_t newestText = 0;
    uintptr_t line = Memory::safeRead<uintptr_t>(Offsets::Console::LINES_HEAD);
    for (size_t count = 0; isNode(line) && count < MAX_LINES_PER_POLL; ++count)
    {
        if (line != input)
        {
            uintptr_t text = Memory::safeRead<uintptr_t>(line + Offsets::Console::LINE_TEXT);
            if (line == s_lastLine && text == s_lastText)
                break;
            if (!newest)
            {
                newest = line;
                newestText = text;
            }
            int style = Memory::safeRead<int>(line + Offsets::Console::LINE_STYLE);
            if (text && style != Offsets::Console::STYLE_INPUT)
                fresh.emplace_back(readText(text), style);
        }
        line = Memory::safeRead<uintptr_t>(line + Offsets::Console::LINE_NEXT);
    }
    if (newest)
    {
        s_lastLine = newest;
        s_lastText = newestText;
    }
    LeaveCriticalSection(lock);

    for (auto it = fresh.rbegin(); it != fresh.rend(); ++it)
        s_callback(it->first.c_str(), it->second);
}

void Initialize()
{
    if (s_initialized) return;

    GameScheduler::Initialize();
    GameScheduler::addIdleHandler(PollConsole, CONSOLE_PRIORITY);

    s_initialized = true;
}

void Shutdown()
{
    s_initialized = false;
}

void SetCallback(PrintMessageCallback callback)
{
    s_callback = callback;
}

}
