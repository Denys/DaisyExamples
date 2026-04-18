cmake_minimum_required(VERSION 3.24)

project(DaisyHost VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(FetchContent)

FetchContent_Declare(
    juce
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.6
)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
)

set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(juce googletest)

enable_testing()

add_library(daisyhost_core STATIC
    src/BoardProfile.cpp
    src/ComputerKeyboardMidi.cpp
    src/HostSessionState.cpp
    src/HostStartupPolicy.cpp
    src/MidiEventTracker.cpp
    src/MidiNotePreview.cpp
    src/MidiLearnMap.cpp
    src/apps/MultiDelayCore.cpp
)

target_include_directories(daisyhost_core
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/../DaisySP/Source
)

target_compile_features(daisyhost_core PUBLIC cxx_std_17)

juce_add_plugin(DaisyHostPatch
    COMPANY_NAME Antigravity
    BUNDLE_ID com.antigravity.daisyhost.patch
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT TRUE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE AGRv
    PLUGIN_CODE DhP1
    FORMATS VST3 Standalone
    PRODUCT_NAME "DaisyHost Patch"
    VST3_CATEGORIES Fx
)

target_sources(DaisyHostPatch
    PRIVATE
        src/juce/DaisyHostPluginEditor.cpp
        src/juce/DaisyHostPluginProcessor.cpp
)

target_include_directories(DaisyHostPatch PRIVATE ${PROJECT_SOURCE_DIR}/include)

target_compile_definitions(DaisyHostPatch
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
)

target_link_libraries(DaisyHostPatch
    PRIVATE
        daisyhost_core
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

add_executable(unit_tests
    tests/test_board_profile.cpp
    tests/test_computer_keyboard_midi.cpp
    tests/test_midi_event_tracker.cpp
    tests/test_host_session_state.cpp
    tests/test_host_startup_policy.cpp
    tests/test_midi_note_preview.cpp
    tests/test_multidelay_core.cpp
)

target_link_libraries(unit_tests
    PRIVATE
        daisyhost_core
        GTest::gtest
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(unit_tests)
