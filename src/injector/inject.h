#pragma once
#include <Windows.h>

bool injectDll(HANDLE hProcess, const char* dllPath);
bool injectManualMap(HANDLE hProcess, const char* dllPath);
bool injectManualMapFromBuffer(HANDLE hProcess, const void* buffer, SIZE_T size, const char* label);
