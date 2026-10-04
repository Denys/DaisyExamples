cmake_minimum_required(VERSION 3.20)
project(HydraPulseField VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(HPF_ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)

add_library(hydrapulse_core INTERFACE)
target_include_directories(hydrapulse_core INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_options(hydrapulse_core INTERFACE /W4 /permissive-)
else()
    target_compile_options(hydrapulse_core INTERFACE
        -Wall
        -Wextra
        -Wpedantic
        -Wconversion
        -Wshadow
    )
endif()

if(HPF_ENABLE_SANITIZERS)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(hydrapulse_core INTERFACE
            -fsanitize=address,undefined
            -fno-omit-frame-pointer
        )
        target_link_options(hydrapulse_core INTERFACE
            -fsanitize=address,undefined
            -fno-omit-frame-pointer
        )
    else()
        message(FATAL_ERROR "HPF_ENABLE_SANITIZERS currently requires GCC or Clang")
    endif()
endif()

include(CTest)
if(BUILD_TESTING)
    add_subdirectory(tests/native)
endif()
