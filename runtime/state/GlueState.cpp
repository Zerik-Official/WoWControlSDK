#include "GlueState.h"
#include "core/native/ClientState.h"
#include "hooks/GlueHooks.h"
#include <Windows.h>
#include <cstring>
#include <chrono>

namespace Runtime
{
    namespace Glue
    {
        static bool s_loginLatch = false;

        Screen getScreen()
        {
            if (WoW::IsInWorld())
                return Screen::WORLD;

            const char* name = WoW::GetScreenName();
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
            if (WoW::IsInWorld())
                return "world";
            return WoW::GetScreenName();
        }

        bool isGameplayReady() { return getScreen() == Screen::WORLD; }
        bool isLoginLatched() { return s_loginLatch; }
        void setLoginLatch(bool v) { s_loginLatch = v; }

        static LoginResult mapLoginResultStr(const char* str)
        {
            if (!str) return LoginResult::FAILED;
            if (strcmp(str, "LOGIN_OK") == 0) return LoginResult::OK;
            if (strcmp(str, "LOGIN_SERVER_DOWN") == 0) return LoginResult::SERVER_DOWN;
            if (strcmp(str, "LOGIN_FAILED") == 0) return LoginResult::FAILED;
            if (strcmp(str, "LOGIN_BANNED") == 0) return LoginResult::BANNED;
            if (strcmp(str, "LOGIN_BADVERSION") == 0) return LoginResult::BADVERSION;
            if (strcmp(str, "LOGIN_ALREADYONLINE") == 0) return LoginResult::ALREADYONLINE;
            if (strcmp(str, "LOGIN_NOTIME") == 0) return LoginResult::NO_TIME;
            if (strcmp(str, "LOGIN_DBBUSY") == 0) return LoginResult::DB_BUSY;
            if (strcmp(str, "LOGIN_SUSPENDED") == 0) return LoginResult::SUSPENDED;
            if (strcmp(str, "LOGIN_PARENTALCONTROL") == 0) return LoginResult::PARENTALCONTROL;
            if (strcmp(str, "LOGIN_LOCKED_ENFORCED") == 0) return LoginResult::LOCKED;
            if (strcmp(str, "LOGIN_DISCONNECTED") == 0) return LoginResult::DISCONNECTED;
            if (strcmp(str, "DISCONNECTED") == 0) return LoginResult::DISCONNECTED;
            if (strcmp(str, "LOGIN_ACCOUNT_CONVERTED") == 0) return LoginResult::ACCOUNT_CONVERTED;
            if (strcmp(str, "LOGIN_TRIAL_EXPIRED") == 0) return LoginResult::TRIAL_EXPIRED;
            if (strcmp(str, "LOGIN_CHARGEBACK") == 0) return LoginResult::CHARGEDBACK;
            if (strcmp(str, "LOGIN_GAME_ACCOUNT_LOCKED") == 0) return LoginResult::GAME_ACCOUNT_LOCKED;
            if (strcmp(str, "LOGIN_UNLOCKABLE_LOCK") == 0) return LoginResult::UNLOCKABLE_LOCK;
            if (strcmp(str, "LOGIN_CONVERSION_REQUIRED") == 0) return LoginResult::CONVERSION_REQUIRED;
            if (strcmp(str, "LOGIN_UNKNOWN_ACCOUNT") == 0) return LoginResult::UNKNOWN_ACCOUNT;
            if (strcmp(str, "LOGIN_INCORRECT_PASSWORD") == 0) return LoginResult::INCORRECT_PASSWORD;
            if (strcmp(str, "LOGIN_NO_GAME_ACCOUNT") == 0) return LoginResult::FAILED;
            if (strcmp(str, "LOGIN_TOO_FAST") == 0) return LoginResult::FAILED;
            if (strcmp(str, "LOGIN_EXPIRED") == 0) return LoginResult::FAILED;
            return LoginResult::FAILED;
        }

        LoginResult waitForLoginResult(int timeoutMs)
        {
            auto start = std::chrono::steady_clock::now();

            while (true)
            {
                if (std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::steady_clock::now() - start).count() >= timeoutMs)
                    return LoginResult::TIMEOUT;

                const char* resultStr = Hooks::Glue::getCapturedLoginResult();
                if (resultStr)
                    return mapLoginResultStr(resultStr);

                const char* screen = WoW::GetScreenName();
                if (screen && strcmp(screen, "charselect") == 0)
                    return LoginResult::OK;

                Sleep(15);
            }
        }

        void initialize()
        {
            s_loginLatch = false;
        }
    }
}

const char* Runtime::loginResultString(LoginResult result)
{
    switch (result)
    {
    case LoginResult::OK:                 return "ok";
    case LoginResult::FAILED:             return "login failed";
    case LoginResult::SERVER_DOWN:        return "server down";
    case LoginResult::UNKNOWN_ACCOUNT:    return "unknown account";
    case LoginResult::INCORRECT_PASSWORD: return "incorrect password";
    case LoginResult::DISCONNECTED:       return "disconnected";
    case LoginResult::PARENTALCONTROL:    return "parental control";
    case LoginResult::CHARGEDBACK:        return "chargeback";
    case LoginResult::CONVERSION_REQUIRED:return "conversion required";
    case LoginResult::BANNED:             return "account banned";
    case LoginResult::SUSPENDED:          return "account suspended";
    case LoginResult::LOCKED:             return "account locked";
    case LoginResult::ALREADYONLINE:      return "already online";
    case LoginResult::BADVERSION:         return "bad version";
    case LoginResult::NO_TIME:            return "no time remaining";
    case LoginResult::DB_BUSY:            return "database busy";
    case LoginResult::TRIAL_EXPIRED:      return "trial expired";
    case LoginResult::ACCOUNT_CONVERTED:  return "account converted";
    case LoginResult::GAME_ACCOUNT_LOCKED:return "game account locked";
    case LoginResult::UNLOCKABLE_LOCK:    return "unlockable lock";
    case LoginResult::TIMEOUT:            return "login timeout";
    default:                              return "unknown error";
    }
}
