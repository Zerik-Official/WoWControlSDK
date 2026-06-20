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
        void    setScreen(Screen s);
        bool    isGameplayReady();
        bool    isLoginLatched();
        void    setLoginLatch(bool value);

        void    initialize();
    }
}
