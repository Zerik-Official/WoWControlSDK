#pragma once

#include <cstdint>
#include <string>

namespace Runtime
{
    struct ConsoleInfo
    {
        bool        initialized;
        bool        visible;
        uint64_t    uptimeMs;
        std::string version;
    };

    namespace ConsoleManager
    {
        void    Initialize();
        void    Shutdown();
        void    Show();
        void    Hide();
        void    Toggle();
        bool    IsInitialized();
        bool    IsVisible();
        ConsoleInfo GetInfo();
        void    Print(const char* text);
    }
}
