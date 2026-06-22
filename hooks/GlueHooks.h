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

    int getLastLoginState();
    int getLastLoginResult();
    const char* getLastLoginResultStr();

    void setLoginPending();
    bool tryGetCapturedAuthCode(int& outCode);
    bool tryGetLoginFailedResult(int& outCode);
}
