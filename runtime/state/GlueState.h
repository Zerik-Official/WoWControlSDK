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

    enum class LoginResult
    {
        NONE = 0,
        OK,
        FAILED,
        SERVER_DOWN,
        BANNED,
        SUSPENDED,
        LOCKED,
        ALREADYONLINE,
        BADVERSION,
        TIMEOUT,
        DISCONNECTED,
        PARENTALCONTROL,
        CHARGEDBACK,
        CONVERSION_REQUIRED,
        UNKNOWN_ACCOUNT,
        INCORRECT_PASSWORD,
        NO_TIME,
        DB_BUSY,
        TRIAL_EXPIRED,
        ACCOUNT_CONVERTED,
        GAME_ACCOUNT_LOCKED,
        UNLOCKABLE_LOCK
    };

    namespace Glue
    {
        Screen  getScreen();
        const char* getScreenName();
        bool    isGameplayReady();
        bool    isLoginLatched();
        void    setLoginLatch(bool value);
        LoginResult waitForLoginResult(int timeoutMs);

        void    initialize();
    }

    const char* loginResultString(LoginResult result);
}
