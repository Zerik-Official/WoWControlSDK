#include "ManualMapInternal.h"

namespace manualmap {

bool callRemote(HANDLE process, LPVOID start, LPVOID parameter, DWORD timeoutMs, DWORD* exitCodeOut)
{
    HANDLE thread = CreateRemoteThread(
        process, nullptr, 0,
        (LPTHREAD_START_ROUTINE)start,
        parameter, 0, nullptr
    );
    if (!thread)
        return fail("CreateRemoteThread");

    if (WaitForSingleObject(thread, timeoutMs) != WAIT_OBJECT_0)
    {
        CloseHandle(thread);
        return fail("WaitForSingleObject (remote thread)", WAIT_TIMEOUT);
    }

    DWORD exitCode = 0;
    if (!GetExitCodeThread(thread, &exitCode))
    {
        CloseHandle(thread);
        return fail("GetExitCodeThread");
    }
    CloseHandle(thread);

    if (exitCodeOut)
        *exitCodeOut = exitCode;
    return true;
}

}
