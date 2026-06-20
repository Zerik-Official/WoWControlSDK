#include "GlueState.h"

namespace Runtime
{
    namespace Glue
    {
        static Screen s_screen = Screen::UNKNOWN;
        static bool   s_loginLatch = false;

        Screen getScreen() { return s_screen; }
        void   setScreen(Screen s) { s_screen = s; }
        bool   isGameplayReady() { return s_screen == Screen::WORLD; }
        bool   isLoginLatched() { return s_loginLatch; }
        void   setLoginLatch(bool v) { s_loginLatch = v; }

        void initialize()
        {
            s_screen = Screen::UNKNOWN;
            s_loginLatch = false;
        }
    }
}
