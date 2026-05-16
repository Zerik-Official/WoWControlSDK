#pragma once
#include <string>

namespace LuaEngine {

bool        initialize();
void        shutdown();

std::string execute(const std::string& code);

void        executeSimple(const std::string& code);

}