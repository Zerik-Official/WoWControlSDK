#pragma once

#include <functional>

namespace Hooks::GameScheduler
{

using IdleHandler = void(*)();

void Initialize();
void Shutdown();

void addIdleHandler(IdleHandler handler, float priority);

bool onMainThread();

}
