cmake_minimum_required(VERSION 3.20)
project(WoWControlSDK VERSION 2.3.0)

set(CMAKE_CXX_STANDARD 17)

set(BUILD_INFO_DIR ${CMAKE_BINARY_DIR}/generated)
set(BUILD_INFO_HEADER ${BUILD_INFO_DIR}/BuildVersion.h)
set(BUILD_INFO_COMMAND
    ${CMAKE_COMMAND}
    -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
    -DOUTPUT=${BUILD_INFO_HEADER}
    -DVERSION=${WoWControlSDK_VERSION}
    -P ${CMAKE_SOURCE_DIR}/cmake/BuildInfo.cmake
)
execute_process(COMMAND ${BUILD_INFO_COMMAND})
add_custom_target(WoWControlSDKBuildInfo
    COMMAND ${BUILD_INFO_COMMAND}
    BYPRODUCTS ${BUILD_INFO_HEADER}
    VERBATIM
)

add_compile_definitions(
    _WIN32_WINNT=0x0601
    WIN32_LEAN_AND_MEAN
)

add_subdirectory(src/injector)

add_library(WoWControlSDK SHARED
    src/DllMain.cpp
    src/ExternalApi.cpp
    src/WoWControlSDK.rc

    src/core/engine/LuaEngine.cpp
    src/core/engine/lua/LuaBlocked.cpp
    src/core/engine/lua/LuaBridge.cpp
    src/core/engine/lua/LuaNative.cpp
    src/core/engine/lua/LuaPath.cpp
    src/core/engine/lua/LuaScripts.cpp
    src/core/engine/lua/LuaStack.cpp
    src/core/engine/lua/LuaTaint.cpp

    src/core/native/ObjectManager.cpp
    src/core/native/ClientState.cpp
    src/core/native/NetClient.cpp
    src/core/native/Performance.cpp
    src/core/native/AddonMemory.cpp
    src/core/native/CrashReport.cpp
    src/core/native/RealmListReader.cpp
    src/core/native/WoWObject.cpp
    src/core/native/Unit.cpp
    src/core/native/Player.cpp
    src/core/native/WorldInfo.cpp
    src/core/native/LoadingScreen.cpp
    src/core/native/WorldReader.cpp
    src/core/native/GlueAPI.cpp
    src/core/native/CharacterReader.cpp
    src/core/native/ClientDb.cpp
    src/core/native/NameCache.cpp
    src/core/native/FrameScript.cpp
    src/core/native/ClientPacket.cpp

    src/core/api/UnitAPI.cpp
    src/core/api/PlayerAPI.cpp
    src/core/api/WorldAPI.cpp
    src/core/api/GroupAPI.cpp
    src/core/api/ObjectAPI.cpp
    src/core/api/FriendsAPI.cpp
    src/core/api/SummonAPI.cpp
    src/core/api/LfgAPI.cpp
    src/core/api/TradeAPI.cpp
    src/core/api/DuelAPI.cpp
    src/core/api/BattlefieldAPI.cpp
    src/core/api/ChatAPI.cpp

    src/core/resolvers/NameResolver.cpp
    src/core/resolvers/UnitResolver.cpp

    src/rpc/protocol/Request.cpp
    src/rpc/protocol/Response.cpp
    src/rpc/Dispatcher.cpp
    src/rpc/handlers/core/ClientRpc.cpp
    src/rpc/handlers/core/LuaRpc.cpp
    src/rpc/handlers/player/PlayerRpc.cpp
    src/rpc/handlers/core/WorldRpc.cpp
    src/rpc/handlers/auth/AuthRpc.cpp
    src/rpc/handlers/core/CharRpc.cpp
    src/rpc/handlers/Handlers.cpp
    src/rpc/handlers/core/UnitRpc.cpp
    src/rpc/handlers/core/GroupRpc.cpp
    src/rpc/handlers/core/ObjectRpc.cpp

    src/rpc/MethodRegistry.cpp
    src/runtime/events/EventBus.cpp
    src/runtime/state/GlueState.cpp
    src/runtime/state/StateCache.cpp
    src/runtime/Runtime.cpp
    src/runtime/console/ConsoleManager.cpp
    src/runtime/console/LogCapture.cpp
    src/runtime/realm/RealmList.cpp
    src/runtime/events/EventPipe.cpp
    src/runtime/events/GameEventBridge.cpp
    src/runtime/events/LuaEventBridge.cpp

    src/runtime/diagnostics/CrashMonitor.cpp
    src/runtime/diagnostics/AddonMemoryMonitor.cpp

    src/runtime/window/WindowService.cpp

    src/hooks/Hooks.cpp
    src/hooks/base/GameScheduler.cpp
    src/hooks/base/HardwareBreakpoint.cpp
    src/hooks/base/IatHook.cpp
    src/hooks/base/VTableHook.cpp
    src/hooks/AddonHooks.cpp
    src/hooks/DisconnectHooks.cpp
    src/hooks/ErrorHooks.cpp
    src/hooks/glue/GlueLoadHooks.cpp
    src/hooks/ConsoleHooks.cpp
    src/hooks/FrameHooks.cpp
    src/hooks/GlueHooks.cpp
    src/hooks/EventHooks.cpp
    src/hooks/LoadingHooks.cpp
    src/hooks/WindowHooks.cpp

    src/ipc/CommandPipe.cpp
    src/ipc/Auth.cpp
    src/ipc/PipeSecurity.cpp

    src/rpc/handlers/control/ControlRpc.cpp
    src/rpc/handlers/event/EventRpc.cpp

    src/rpc/handlers/window/WindowRpc.cpp
    src/rpc/handlers/social/SocialRpc.cpp

    src/utils/json/JsonIPC.cpp
    src/utils/BuildInfo.cpp

    src/client/strings/RCString.cpp
    src/client/login/LoginAPI.cpp
    src/client/console/ConsoleAPI.cpp
    src/client/lua/LuaAPI.cpp
    src/client/game/ErrorAPI.cpp
    src/client/utils/CString.cpp
)

target_include_directories(WoWControlSDK PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/api
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/native
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/types
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/resolvers
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rpc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/runtime
    ${CMAKE_CURRENT_SOURCE_DIR}/src/offsets
    ${CMAKE_CURRENT_SOURCE_DIR}/deps
    ${BUILD_INFO_DIR}
)

add_dependencies(WoWControlSDK WoWControlSDKBuildInfo)

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
