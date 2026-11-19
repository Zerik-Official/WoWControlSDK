#pragma once
#include <Windows.h>

namespace PipeSecurity {

SECURITY_ATTRIBUTES* attributes();

constexpr DWORD PIPE_FLAGS = PIPE_REJECT_REMOTE_CLIENTS;

}
