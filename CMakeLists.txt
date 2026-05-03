cmake_minimum_required(VERSION 3.20)
project(MiniStateLib)

set(CMAKE_CXX_STANDARD 17)

# Añadir Detours como subproyecto
add_subdirectory(deps/Detours)

# --- Fuentes de la mini DLL ---
add_library(MiniStateLib SHARED
    DllMain.cpp
    GameState.cpp
    Hooks.cpp
)

# --- Includes ---
target_include_directories(MiniStateLib PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/Detours
)

# --- Linkeo ---
target_link_libraries(MiniStateLib PRIVATE
    Detours
)

# --- Opciones de compilador (x86 obligatorio para WoW 3.3.5a) ---
target_compile_options(MiniStateLib PRIVATE
    /arch:IA32
    /GS-
)

# --- Output ---
set_target_properties(MiniStateLib PROPERTIES
    OUTPUT_NAME "MiniStateLib"
    SUFFIX ".dll"
)