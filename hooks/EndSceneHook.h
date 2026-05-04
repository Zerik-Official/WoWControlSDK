#pragma once
#include <Windows.h>
#include <string>
#include <functional>

struct IDirect3DDevice9;

namespace EndSceneHook {

using Task = std::function<std::string()>;

void initialize();
void shutdown();

std::string dispatch(Task task, DWORD timeoutMs = 500);

HRESULT WINAPI hkEndScene(IDirect3DDevice9* pDevice);

}