cmake_minimum_required(VERSION 3.20)
project(WowInjector)

set(CMAKE_CXX_STANDARD 17)

add_executable(WowInjector
    main.cpp
    args.cpp
    process.cpp
    inject.cpp
    pipe.cpp
    manualmap/Diagnostics.cpp
    manualmap/NativeLoader.cpp
    manualmap/Pe.cpp
    manualmap/RemoteCall.cpp
    manualmap/Image.cpp
    manualmap/Shellcode.cpp
    manualmap/ManualMap.cpp
)

target_compile_definitions(WowInjector PRIVATE
    _WIN32_WINNT=0x0601
)

target_compile_options(WowInjector PRIVATE /arch:IA32)

target_link_libraries(WowInjector PRIVATE kernel32)

set_target_properties(WowInjector PROPERTIES
    OUTPUT_NAME "WowInjector"
    RUNTIME_OUTPUT_DIRECTORY             ${CMAKE_SOURCE_DIR}/bin
    RUNTIME_OUTPUT_DIRECTORY_RELEASE     ${CMAKE_SOURCE_DIR}/bin/release
    RUNTIME_OUTPUT_DIRECTORY_DEBUG       ${CMAKE_SOURCE_DIR}/bin/debug
)
