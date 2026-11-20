#include "Hooks.h"

void Hooks::initialize()
{
    Hooks::Console::Initialize();
    Hooks::Events::Initialize();
    Hooks::Loading::Initialize();
    Hooks::WindowStart::Initialize();
    Hooks::Addons::Initialize();
    Hooks::Disconnect::Initialize();
}
