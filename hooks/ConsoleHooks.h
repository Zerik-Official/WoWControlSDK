#pragma once

#include <Windows.h>
#include <functional>
#include <string>

namespace Hooks::Console
{

using PrintMessageFn = void(__cdecl*)(const char* text, int style);
using PrintMessageCallback = std::function<void(const char* text, int style)>;

void Initialize();
void Shutdown();

void SetCallback(PrintMessageCallback callback);

}
