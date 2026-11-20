#pragma once

namespace ExternalApi
{
    bool isGameProcess();
}

extern "C" __declspec(dllexport) int __cdecl WoWControlSDK_Invoke(const char* request, char* response, int responseSize);
