cmake_minimum_required(VERSION 3.20)

project(PedalMultiDelayAudition LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(pedal_multidelay_audition
    main.cpp
    ../../src/PedalDelayEngine.cpp
)

target_include_directories(pedal_multidelay_audition
    PRIVATE
        ../../include
)

if(MSVC)
    target_compile_options(pedal_multidelay_audition PRIVATE /W4 /permissive-)
else()
    target_compile_options(pedal_multidelay_audition PRIVATE -Wall -Wextra -Wpedantic)
endif()

enable_testing()

set(PEDAL_MULTIDELAY_AUDITION_MATRIX
    "${CMAKE_CURRENT_LIST_DIR}/../../training/examples/pedal_multidelay_audition_matrix.csv"
)
set(PEDAL_MULTIDELAY_AUDITION_OUTPUT
    "${CMAKE_BINARY_DIR}/audition_artifacts"
)

add_test(
    NAME PedalMultiDelayAuditionMatrix
    COMMAND
        pedal_multidelay_audition
        --matrix "${PEDAL_MULTIDELAY_AUDITION_MATRIX}"
        --output-dir "${PEDAL_MULTIDELAY_AUDITION_OUTPUT}"
)
set_tests_properties(
    PedalMultiDelayAuditionMatrix
    PROPERTIES
        TIMEOUT 300
        LABELS "pedal_multidelay;audition;host-only"
)
