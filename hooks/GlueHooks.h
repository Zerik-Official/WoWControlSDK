#pragma once

#include <Windows.h>
#include <string>
#include <functional>

namespace Hooks::Glue
{

void Initialize();
void Shutdown();

void Post(std::function<void()> task);

}
