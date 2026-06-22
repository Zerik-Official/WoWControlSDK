#pragma once
#include <string>
#include <vector>

namespace Hooks::Events
{
    void    Initialize();
    void    Shutdown();
    bool    IsReady();
    bool    IsSubscribed(const char* eventName);
    void    Subscribe(const char* eventName);
    void    Unsubscribe(const char* eventName);
    void    ClearSubscriptions();
    void    GetSubscriptions(std::vector<std::string>& out);
}
