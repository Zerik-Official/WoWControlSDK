#pragma once

#include <Windows.h>
#include <string>
#include <functional>

namespace Hooks::Glue
{

void Initialize();
void Shutdown();

void Post(std::function<void()> task);
std::string Execute(std::function<std::string()> task, DWORD timeoutMs = 15000);

void setLoginPending();
const char* getCapturedLoginResult();

}
