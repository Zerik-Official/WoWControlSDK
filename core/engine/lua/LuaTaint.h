#pragma once
#include <cstdint>

namespace LuaTaint {

const char* sdkSource();

class Scope {
public:
    explicit Scope(bool secure);
    explicit Scope(const char* source);
    ~Scope();

    Scope(const Scope&) = delete;
    Scope& operator=(const Scope&) = delete;

private:
    uintptr_t m_previous;
};

}
