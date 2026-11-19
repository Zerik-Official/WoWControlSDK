#pragma once
#include <Windows.h>
#include <string>

bool waitForPipe(const char* pipeName, DWORD timeoutMs, HANDLE process = nullptr);
bool sendPipeCommand(const char* pipeName, const std::string& json, std::string& responseOut);
