#pragma once

namespace Hooks::WindowStart
{
    void Initialize();
    void Shutdown();
    void SetStartHidden(bool hidden);
    bool IsStartHidden();
    void Release();
}
