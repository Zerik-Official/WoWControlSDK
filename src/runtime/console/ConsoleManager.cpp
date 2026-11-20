#include "ConsoleManager.h"
#include "utils/BuildInfo.h"
#include <Windows.h>
#include <cstdio>
#include <string>

#pragma execution_character_set("utf-8")

namespace Runtime
{
    namespace ConsoleManager
    {
        static bool s_initialized = false;
        static bool s_visible = false;
        static uint64_t s_startMs = 0;

        static const char* LOGO =
            "\n"
            "██╗    ██╗ ██████╗ ██╗    ██╗\n"
            "██║    ██║██╔═══██╗██║    ██║\n"
            "██║ █╗ ██║██║   ██║██║ █╗ ██║\n"
            "██║███╗██║██║   ██║██║███╗██║\n"
            "╚███╔███╔╝╚██████╔╝╚███╔███╔╝\n"
            " ╚══╝╚══╝  ╚═════╝  ╚══╝╚══╝ \n"
            "                              \n"
            "      ██████╗ ██████╗ ███╗   ██╗████████╗██████╗  ██████╗ ██╗\n"
            "     ██╔════╝██╔═══██╗████╗  ██║╚══██╔══╝██╔══██╗██╔═══██╗██║\n"
            "     ██║     ██║   ██║██╔██╗ ██║   ██║   ██████╔╝██║   ██║██║\n"
            "     ██║     ██║   ██║██║╚██╗██║   ██║   ██╔══██╗██║   ██║██║\n"
            "     ╚██████╗╚██████╔╝██║ ╚████║   ██║   ██║  ██║╚██████╔╝███████╗\n"
            "      ╚═════╝ ╚═════╝ ╚═╝  ╚═══╝   ╚═╝   ╚═╝  ╚═╝ ╚═════╝ ╚══════╝\n"
            "                                                                  \n"
            "               ███████╗██████╗ ██╗  ██╗\n"
            "               ██╔════╝██╔══██╗██║ ██╔╝\n"
            "               ███████╗██║  ██║█████╔╝\n"
            "               ╚════██║██║  ██║██╔═██╗\n"
            "               ███████║██████╔╝██║  ██╗\n"
            "               ╚══════╝╚═════╝ ╚═╝  ╚═╝\n";

        static const char* RST = "\033[0m";
        static const char* DIM  = "\033[2m";
        static const char* YEL  = "\033[33m";
        static const char* CYN  = "\033[36m";
        static const char* GRN  = "\033[32m";
        static const char* WHT  = "\033[97m";

        static void ColorPrint(const char* color, const char* text)
        {
            printf("%s%s%s\n", color, text, RST);
        }

        static uint64_t nowMs()
        {
            return GetTickCount64();
        }

        static void EnableAnsi()
        {
            HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
            DWORD mode = 0;
            if (GetConsoleMode(hOut, &mode))
            {
                SetConsoleMode(hOut, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
            }
        }

        void Initialize()
        {
            if (s_initialized) return;

            AllocConsole();

            SetConsoleOutputCP(CP_UTF8);
            SetConsoleCP(CP_UTF8);
            EnableAnsi();

            FILE* f;
            freopen_s(&f, "CONOUT$", "w", stdout);
            freopen_s(&f, "CONOUT$", "w", stderr);
            freopen_s(&f, "CONIN$", "r", stdin);

            std::string title = std::string("WoWControlSDK v") + SDK::BuildInfo::version();
            SetConsoleTitleA(title.c_str());

            s_initialized = true;
            s_visible = true;
            s_startMs = nowMs();

            ColorPrint(CYN, LOGO);
            ColorPrint(DIM, "────────────────────────────────────────────────────────");
            std::string version = std::string("  Version : ") + SDK::BuildInfo::version();
            ColorPrint(WHT, version.c_str());
            if (SDK::BuildInfo::commitShort()[0] != '\0')
            {
                std::string commit = std::string("  Commit  : ") + SDK::BuildInfo::commitShort()
                    + (SDK::BuildInfo::dirty() ? "-dirty" : "") + " (" + SDK::BuildInfo::commitDate() + ")";
                ColorPrint(WHT, commit.c_str());
            }
            ColorPrint(WHT, "  Author  : Zerik-Official");
            ColorPrint(WHT, "  License : Apache License, Version 2.0");
            ColorPrint(WHT, "  Repo    : https://github.com/Zerik-Official/WoWControlSDK");
            ColorPrint(DIM, "────────────────────────────────────────────────────────");
            Print("");
        }

        void Shutdown()
        {
            if (!s_initialized) return;

            Print("shutting down console...");

            FreeConsole();
            s_initialized = false;
            s_visible = false;
        }

        void Show()
        {
            if (!s_initialized) return;
            ShowWindow(GetConsoleWindow(), SW_SHOW);
            s_visible = true;
        }

        void Hide()
        {
            if (!s_initialized) return;
            ShowWindow(GetConsoleWindow(), SW_HIDE);
            s_visible = false;
        }

        void Toggle()
        {
            if (!s_initialized)
            {
                Initialize();
                return;
            }
            if (s_visible)
                Hide();
            else
                Show();
        }

        bool IsInitialized()
        {
            return s_initialized;
        }

        bool IsVisible()
        {
            return s_visible;
        }

        ConsoleInfo GetInfo()
        {
            ConsoleInfo info = {};
            info.initialized = s_initialized;
            info.visible = s_visible;
            info.uptimeMs = s_initialized ? (nowMs() - s_startMs) : 0;
            info.version = SDK::BuildInfo::version();
            return info;
        }

        void Print(const char* text)
        {
            if (!s_initialized || !s_visible || !text) return;
            printf("%s\n", text);
        }
    }
}
