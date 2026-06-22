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

        static LoginResult mapAuthCode(int code)
        {
            if (code == 0 || code == 14) return LoginResult::OK;
            switch (code)
            {
            case 3:  return LoginResult::BANNED;
            case 4:  return LoginResult::UNKNOWN_ACCOUNT;
            case 5:  return LoginResult::INCORRECT_PASSWORD;
            case 6:  return LoginResult::ALREADYONLINE;
            case 7:  return LoginResult::NO_TIME;
            case 8:  return LoginResult::DB_BUSY;
            case 9:  return LoginResult::BADVERSION;
            case 10: return LoginResult::SERVER_DOWN;
            case 12: return LoginResult::SUSPENDED;
            case 15: return LoginResult::PARENTALCONTROL;
            case 16: return LoginResult::LOCKED;
            case 17: return LoginResult::TRIAL_EXPIRED;
            case 18: return LoginResult::ACCOUNT_CONVERTED;
            case 22: return LoginResult::CHARGEDBACK;
            case 24: return LoginResult::GAME_ACCOUNT_LOCKED;
            case 25: return LoginResult::UNLOCKABLE_LOCK;
            case 32: return LoginResult::CONVERSION_REQUIRED;
            case 255:return LoginResult::DISCONNECTED;
            default: return LoginResult::FAILED;
            }
        }

        LoginResult waitForLoginResult(int timeoutMs)
        {
            auto start = std::chrono::steady_clock::now();

            while (true)
            {
                if (std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::steady_clock::now() - start).count() >= timeoutMs)
                    return LoginResult::TIMEOUT;

                int code = -1;
                if (Hooks::Glue::tryGetCapturedAuthCode(code))
                {
                    if (code == 0)
                        return LoginResult::OK;
                    return mapAuthCode(code);
                }

                if (Hooks::Glue::tryGetLoginFailedResult(code))
                    return mapAuthCode(code);

                const char* screen = WoW::GetScreenName();
                if (screen && strcmp(screen, "charselect") == 0)
                {
                    return LoginResult::OK;
                }

                Sleep(15);
            }
        }

    void initialize()
    {
        s_loginLatch = false;
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
}
