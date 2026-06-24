cmake_minimum_required(VERSION 3.20)
project(WoWControlSDK)

set(CMAKE_CXX_STANDARD 17)

add_compile_definitions(
    _WIN32_WINNT=0x0601
    WIN32_LEAN_AND_MEAN
)

add_subdirectory(deps/Detours)
add_subdirectory(injector)

add_library(WoWControlSDK SHARED
    DllMain.cpp

    core/engine/LuaEngine.cpp

    core/native/ObjectManager.cpp
    core/native/ClientState.cpp
    core/native/NetClient.cpp
    core/native/RealmListReader.cpp
    core/native/WoWObject.cpp
    core/native/Unit.cpp
    core/native/Player.cpp
    core/native/WorldInfo.cpp
    core/native/WorldReader.cpp
    core/native/GlueAPI.cpp
    core/native/CharacterReader.cpp

    core/api/UnitAPI.cpp
    core/api/PlayerAPI.cpp
    core/api/WorldAPI.cpp
    core/api/GroupAPI.cpp
    core/api/ObjectAPI.cpp

    core/resolvers/NameResolver.cpp
    core/resolvers/UnitResolver.cpp

    rpc/protocol/Request.cpp
    rpc/protocol/Response.cpp
    rpc/Dispatcher.cpp
    rpc/handlers/core/ClientRpc.cpp
    rpc/handlers/core/LuaRpc.cpp
    rpc/handlers/player/PlayerRpc.cpp
    rpc/handlers/core/WorldRpc.cpp
    rpc/handlers/auth/AuthRpc.cpp
    rpc/handlers/core/CharRpc.cpp
    rpc/handlers/Handlers.cpp
    rpc/handlers/core/UnitRpc.cpp
    rpc/handlers/core/GroupRpc.cpp
    rpc/handlers/core/ObjectRpc.cpp

    rpc/MethodRegistry.cpp
    runtime/events/EventBus.cpp
    runtime/state/GlueState.cpp
    runtime/state/StateCache.cpp
    runtime/Runtime.cpp
    runtime/console/ConsoleManager.cpp
    runtime/console/LogCapture.cpp
    runtime/realm/RealmList.cpp
    runtime/events/EventPipe.cpp

    hooks/Hooks.cpp
    hooks/lua/LuaHooks.cpp
    hooks/glue/GlueLoadHooks.cpp
    hooks/EndSceneHook.cpp
    hooks/ConsoleHooks.cpp
    hooks/FrameHooks.cpp
    hooks/GlueHooks.cpp
    hooks/EventHooks.cpp
    hooks/LoadingHooks.cpp

    ipc/CommandPipe.cpp

    rpc/handlers/control/ControlRpc.cpp
    rpc/handlers/event/EventRpc.cpp

    utils/json/JsonIPC.cpp

    client/globals/ClientGlobals.cpp
    client/strings/RCString.cpp
    client/login/LoginAPI.cpp
    client/console/ConsoleAPI.cpp
    client/lua/LuaAPI.cpp
    client/game/ErrorAPI.cpp
    client/utils/CString.cpp
)

target_include_directories(WoWControlSDK PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/core
    ${CMAKE_CURRENT_SOURCE_DIR}/core/api
    ${CMAKE_CURRENT_SOURCE_DIR}/core/native
    ${CMAKE_CURRENT_SOURCE_DIR}/core/types
    ${CMAKE_CURRENT_SOURCE_DIR}/core/resolvers
    ${CMAKE_CURRENT_SOURCE_DIR}/rpc
    ${CMAKE_CURRENT_SOURCE_DIR}/runtime
    ${CMAKE_CURRENT_SOURCE_DIR}/offsets
    ${CMAKE_CURRENT_SOURCE_DIR}/deps
)

target_link_libraries(WoWControlSDK PRIVATE
    Detours
)

target_compile_options(WoWControlSDK PRIVATE
    /arch:IA32
    /GS-
    /utf-8
)

set_target_properties(WoWControlSDK PROPERTIES
    OUTPUT_NAME "WoWControlSDK"

    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin/release
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin/debug
)
