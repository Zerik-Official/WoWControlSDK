#include "GlueState.h"
#include <cstring>

namespace Runtime
{
    namespace Glue
    {
        static bool s_loginLatch = false;

        static const char* readScreenName()
        {
            return (const char*)0x00B6A9E0;
        }

        Screen getScreen()
        {
            if (*(bool*)0x00BD0792)
                return Screen::WORLD;

            const char* name = readScreenName();
            if (!name || name[0] == '\0')
                return Screen::UNKNOWN;

            if (strcmp(name, "login") == 0)
                return Screen::LOGIN;

            if (strcmp(name, "charselect") == 0)
                return Screen::CHARSELECT;

            return Screen::UNKNOWN;
        }

        const char* getScreenName()
        {
            return readScreenName();
        }

        bool isGameplayReady() { return getScreen() == Screen::WORLD; }
        bool isLoginLatched() { return s_loginLatch; }
        void setLoginLatch(bool v) { s_loginLatch = v; }

        void initialize()
        {
            s_loginLatch = false;
        }
    }
}
