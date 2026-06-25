#pragma once

#include <cstdint>

namespace Offsets
{
    namespace Login
    {
        constexpr uintptr_t CHAR_VECTOR             = 0x00B6B238;
        constexpr uintptr_t LOGOUT_TO_CHAR_SELECT   = 0x00510430;
        constexpr uintptr_t QUIT_GAME               = 0x00510A00;
        constexpr uintptr_t SEND_CHAR_ENUM          = 0x006B14C0;
        constexpr uintptr_t NET_LOGIN               = 0x004D8A30;
        constexpr uintptr_t LOAD_GLUE_XML           = 0x004DA9AC;
        constexpr uintptr_t LOAD_CHARACTERS         = 0x004E47E5;

        constexpr uintptr_t GLUE_MGR_UPDATE         = 0x004DAB40;
        constexpr uintptr_t GRUNT_PRINTER           = 0x004DA4B0;

        constexpr uintptr_t EULA_PENDING            = 0x00B6AF50;
        constexpr uintptr_t EULA_ACCEPTED           = 0x00B6AF54;
        constexpr uintptr_t TOS_PENDING             = 0x00B6AF58;
        constexpr uintptr_t TOS_ACCEPTED            = 0x00B6AF5C;
        constexpr uintptr_t TERM_PENDING            = 0x00B6AF60;
        constexpr uintptr_t TERM_ACCEPTED           = 0x00B6AF64;
        constexpr uintptr_t SCAN_PENDING            = 0x00B6AF68;
        constexpr uintptr_t SCAN_ACCEPTED           = 0x00B6AF6C;
        constexpr uintptr_t CONTEST_PENDING         = 0x00B6AF70;
        constexpr uintptr_t CONTEST_ACCEPTED        = 0x00B6AF74;
    }
}
