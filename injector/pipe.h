#pragma once
#include <Windows.h>
#include <string>

void waitForPipe(const char* pipeName, DWORD timeoutMs);
bool sendPipeCommand(const char* pipeName, const std::string& json, std::string& responseOut);
