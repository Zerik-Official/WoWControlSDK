#include "Hooks.h"

void Hooks::initialize()
{
    Hooks::Lua::Initialize();
    Hooks::GlueLoad::Initialize();
    Hooks::Console::Initialize();
    Hooks::Events::Initialize();
    Hooks::Loading::Initialize();
}
