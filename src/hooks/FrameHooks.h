#pragma once

#include <Windows.h>
#include <string>
#include <functional>

namespace Hooks::Frame
{

using Task = std::function<std::string()>;

void Initialize();
void Shutdown();

void SetOnFrame(std::function<void()> callback);

std::string Execute(Task task, DWORD timeoutMs = 1000);

}
