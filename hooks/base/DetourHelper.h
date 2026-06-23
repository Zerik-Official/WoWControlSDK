#pragma once
#include <Windows.h>
#include <deps/Detours/detours.h>

namespace Hooks::Detail
{

template<typename Fn>
inline void attach(Fn& original, Fn hook)
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(&(void*&)original, (void*)hook);
    DetourTransactionCommit();
}

template<typename Fn>
inline void detach(Fn& original, Fn hook)
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourDetach(&(void*&)original, (void*)hook);
    DetourTransactionCommit();
}

template<typename Fn>
struct DetourEntry {
    Fn& original;
    Fn hook;
};

template<typename Fn> DetourEntry(Fn&, Fn) -> DetourEntry<Fn>;

template<typename... Entries>
inline void attachBatch(Entries&&... entries)
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    (DetourAttach(&(void*&)entries.original, (void*)entries.hook), ...);
    DetourTransactionCommit();
}

template<typename... Entries>
inline void detachBatch(Entries&&... entries)
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    (DetourDetach(&(void*&)entries.original, (void*)entries.hook), ...);
    DetourTransactionCommit();
}

}
