#pragma once

namespace Runtime
{
    enum class Screen
    {
        UNKNOWN,
        LOGIN,
        CHARSELECT,
        WORLD
    };

    namespace Glue
    {
        Screen  getScreen();
        const char* getScreenName();
        bool    isGameplayReady();
        bool    isLoginLatched();
        void    setLoginLatch(bool value);

        void    initialize();
    }
}
