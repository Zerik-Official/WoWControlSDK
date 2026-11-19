#pragma once
#include <Windows.h>

bool injectDll(HANDLE hProcess, const char* dllPath);
