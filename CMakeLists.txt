cmake_minimum_required(VERSION 3.20)
project(WoWControlSDK)

set(CMAKE_CXX_STANDARD 17)

add_compile_definitions(
    _WIN32_WINNT=0x0601
    WIN32_LEAN_AND_MEAN
)

add_subdirectory(deps/Detours)

add_library(WoWControlSDK SHARED
    DllMain.cpp

    core/GameState.cpp
    core/CharCache.cpp
    core/engine/LuaEngine.cpp
    core/PlayerState.cpp
    core/state/player/PlayerBase.cpp
    core/state/player/PlayerPower.cpp
    core/state/player/PlayerStatus.cpp

    core/state/group/GroupMemberState.cpp
    core/state/group/GroupState.cpp
    core/state/group/GroupCombatState.cpp
    core/state/group/GroupStateJson.cpp

    core/api/object/ObjectManager.cpp
    core/api/unit/Unit.cpp
    core/api/internal/UnitResolver.cpp
    core/api/internal/NameResolver.cpp

    core/map/MapInfo.cpp
    core/map/MapReader.cpp

    hooks/Hooks.cpp
    hooks/EndSceneHook.cpp

    ipc/CommandPipe.cpp
    ipc/PipeServer.cpp

    commands/CommandHandler.cpp

    utils/JsonUtils.cpp
)

target_include_directories(WoWControlSDK PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/core
    ${CMAKE_CURRENT_SOURCE_DIR}/core/state
    ${CMAKE_CURRENT_SOURCE_DIR}/offsets
    ${CMAKE_CURRENT_SOURCE_DIR}/core/api
)

target_link_libraries(WoWControlSDK PRIVATE
    Detours
)

target_compile_options(WoWControlSDK PRIVATE
    /arch:IA32
    /GS-
)

set_target_properties(WoWControlSDK PROPERTIES
    OUTPUT_NAME "WoWControlSDK"

    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin/release
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin/debug
)