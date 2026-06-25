#pragma once
#include <string>
#include <vector>

namespace Hooks::Events
{
    void    Initialize();
    void    Shutdown();
    bool    IsReady();
    void    Subscribe(const char* eventName);
    void    Unsubscribe(const char* eventName);
    void    ClearSubscriptions();
    void    GetSubscriptions(std::vector<std::string>& out);
}
