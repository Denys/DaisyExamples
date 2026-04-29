cmake_minimum_required(VERSION 3.24)

project(DaisyHost VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(DAISYHOST_UNIT_TEST_RUN_TAG "default" CACHE STRING
    "Unique tag for the current unit-test payload output path."
)

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
    src/AppRegistry.cpp
    src/BoardControlMapping.cpp
    src/BoardProfile.cpp
    src/ComputerKeyboardMidi.cpp
    src/DaisyBraidsCore.cpp
    src/DaisyCloudSeedCore.cpp
    src/DaisyFieldRSPLayout.cpp
    src/DaisyHarmoniqsCore.cpp
    src/DaisyPolyOscCore.cpp
    src/DaisySubharmoniqCore.cpp
    src/DaisyVASynthCore.cpp
    src/EffectiveHostStateSnapshot.cpp
    src/FieldOledTransient.cpp
    src/HostModulation.cpp
    src/HostModulationUiText.cpp
    src/HostSessionState.cpp
    src/HostStartupPolicy.cpp
    src/HostAutomationBridge.cpp
    src/LiveRackTopology.cpp
    src/MidiEventTracker.cpp
    src/MidiNotePreview.cpp
    src/MidiLearnMap.cpp
    src/SignalGenerator.cpp
    src/StandaloneUiPolicy.cpp
    src/TestInputSignal.cpp
    src/VersionInfo.cpp
    src/apps/BraidsCore.cpp
    src/apps/CloudSeedCore.cpp
    src/apps/HarmoniqsCore.cpp
    src/apps/MultiDelayCore.cpp
    src/apps/PolyOscCore.cpp
    src/apps/SubharmoniqCore.cpp
    src/apps/TorusCore.cpp
    src/apps/VASynthCore.cpp
    ../third_party/BraidsCore/braids/analog_oscillator.cc
    ../third_party/BraidsCore/braids/digital_oscillator.cc
    ../third_party/BraidsCore/braids/macro_oscillator.cc
    ../third_party/BraidsCore/braids/resources.cc
    ../third_party/CloudSeedCore/DSP/Biquad.cpp
    ../stmlib/utils/random.cc
    ../third_party/CloudSeedCore/DSP/RandomBuffer.cpp
    ../third_party/CloudSeedCore/Parameters.cpp
)

target_include_directories(daisyhost_core
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/../DaisySP/Source
    PRIVATE
        ${PROJECT_SOURCE_DIR}/..
        ${PROJECT_SOURCE_DIR}/../third_party/BraidsCore
        ${PROJECT_SOURCE_DIR}/../third_party/CloudSeedCore
)

target_compile_features(daisyhost_core PUBLIC cxx_std_17)

target_compile_definitions(daisyhost_core
    PUBLIC
        DAISYHOST_VERSION="${PROJECT_VERSION}"
        DAISYHOST_BUILD_IDENTITY="DaisyHost ${PROJECT_VERSION}"
    PRIVATE
        BUFFER_SIZE=1024
        MAX_STR_SIZE=32
)

add_library(daisyhost_hub_core STATIC
    src/HubSupport.cpp
)

target_include_directories(daisyhost_hub_core
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_compile_features(daisyhost_hub_core PUBLIC cxx_std_17)

target_compile_definitions(daisyhost_hub_core
    PUBLIC
        DAISYHOST_SOURCE_DIR="${PROJECT_SOURCE_DIR}"
)

target_link_libraries(daisyhost_hub_core
    PUBLIC
        daisyhost_core
        juce::juce_core
)

add_library(daisyhost_render STATIC
    src/RenderRuntime.cpp
)

target_include_directories(daisyhost_render
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_compile_features(daisyhost_render PUBLIC cxx_std_17)

target_link_libraries(daisyhost_render
    PUBLIC
        daisyhost_core
        juce::juce_audio_formats
        juce::juce_core
)

add_executable(DaisyHostRender
    tools/render_app.cpp
)

target_link_libraries(DaisyHostRender
    PRIVATE
        daisyhost_render
)

add_library(daisyhost_cli STATIC
    src/CliPayloads.cpp
    src/DoctorDiagnostics.cpp
    src/GateDiagnostics.cpp
    src/RenderAssertions.cpp
)

target_include_directories(daisyhost_cli
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_compile_features(daisyhost_cli PUBLIC cxx_std_17)

target_link_libraries(daisyhost_cli
    PUBLIC
        daisyhost_core
        juce::juce_core
)

add_executable(DaisyHostCLI
    tools/cli_app.cpp
)

target_link_libraries(DaisyHostCLI
    PRIVATE
        daisyhost_cli
        daisyhost_render
        juce::juce_core
)

juce_add_gui_app(DaisyHostHub
    COMPANY_NAME Antigravity
    ICON_SMALL "${PROJECT_SOURCE_DIR}/assets/daisyhost-icon-compact.png"
    PRODUCT_NAME "DaisyHost Hub"
)

target_sources(DaisyHostHub
    PRIVATE
        src/hub/DaisyHostHubMain.cpp
        src/hub/DaisyHostHubWindow.cpp
)

target_include_directories(DaisyHostHub PRIVATE ${PROJECT_SOURCE_DIR}/include)

target_compile_definitions(DaisyHostHub
    PRIVATE
        DAISYHOST_VERSION="${PROJECT_VERSION}"
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
)

target_link_libraries(DaisyHostHub
    PRIVATE
        daisyhost_hub_core
        juce::juce_gui_extra
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

juce_add_plugin(DaisyHostPatch
    COMPANY_NAME Antigravity
    BUNDLE_ID com.antigravity.daisyhost.patch
    ICON_SMALL "${PROJECT_SOURCE_DIR}/assets/daisyhost-icon-compact.png"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT TRUE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE AGRv
    PLUGIN_CODE DhP1
    FORMATS VST3 Standalone
    PRODUCT_NAME "DaisyHost Patch"
    VST3_AUTO_MANIFEST FALSE
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
        daisyhost_hub_core
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# The JUCE-generated MSBuild post-build step invokes the VST3 manifest helper
# from the build tree and currently fails in this checkout. Run the same helper
# from the source root instead so the VST3 target stays reproducible.
_juce_add_vst3_manifest_helper_target()
add_dependencies(DaisyHostPatch_VST3 juce_vst3_helper)

get_target_property(DAISYHOST_VST3_ARTEFACT DaisyHostPatch_VST3 JUCE_PLUGIN_ARTEFACT_FILE)
set(DAISYHOST_VST3_MANIFEST_SCRIPT "${PROJECT_SOURCE_DIR}/tools/write_vst3_manifest.ps1")
find_program(DAISYHOST_POWERSHELL powershell REQUIRED)

if(DAISYHOST_VST3_ARTEFACT)
    add_custom_command(TARGET DaisyHostPatch_VST3 POST_BUILD
        COMMAND ${DAISYHOST_POWERSHELL}
            -ExecutionPolicy Bypass
            -File "${DAISYHOST_VST3_MANIFEST_SCRIPT}"
            -HelperPath "$<TARGET_FILE:juce_vst3_helper>"
            -PluginPath "${DAISYHOST_VST3_ARTEFACT}"
            -OutputPath "${DAISYHOST_VST3_ARTEFACT}/Contents/Resources/moduleinfo.json"
            -Version "${PROJECT_VERSION}"
        VERBATIM
    )
endif()

set(DAISYHOST_UNIT_TEST_SOURCES
    tests/test_app_registry.cpp
    tests/test_braids_core.cpp
    tests/test_board_control_mapping.cpp
    tests/test_board_profile.cpp
    tests/test_cloudseed_core.cpp
    tests/test_computer_keyboard_midi.cpp
    tests/test_cli_payloads.cpp
    tests/test_doctor_diagnostics.cpp
    tests/test_gate_diagnostics.cpp
    tests/test_daisy_braids_core.cpp
    tests/test_daisy_cloudseed_core.cpp
    tests/test_daisy_field_rsp_layout.cpp
    tests/test_daisy_harmoniqs_core.cpp
    tests/test_daisy_polyosc_core.cpp
    tests/test_daisy_vasynth_core.cpp
    tests/test_effective_host_state_snapshot.cpp
    tests/test_field_oled_transient.cpp
    tests/test_midi_event_tracker.cpp
    tests/test_hub_support.cpp
    tests/test_host_automation_bridge.cpp
    tests/test_host_modulation.cpp
    tests/test_host_session_state.cpp
    tests/test_host_startup_policy.cpp
    tests/test_harmoniqs_core.cpp
    tests/test_live_rack_topology.cpp
    tests/test_midi_note_preview.cpp
    tests/test_multidelay_core.cpp
    tests/test_parameter_parity.cpp
    tests/test_polyosc_core.cpp
    tests/test_signal_generator.cpp
    tests/test_standalone_ui_policy.cpp
    tests/test_subharmoniq_core.cpp
    tests/test_test_input_signal.cpp
    tests/test_render_runtime.cpp
    tests/test_render_assertions.cpp
    tests/test_torus_core.cpp
    tests/test_vasynth_core.cpp
    tests/test_version_info.cpp
)

find_program(DAISYHOST_PYTHON_LAUNCHER NAMES py REQUIRED)

add_executable(unit_tests
    ${DAISYHOST_UNIT_TEST_SOURCES}
)

set_target_properties(unit_tests
    PROPERTIES
        OUTPUT_NAME DaisyHostTestPayload
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/unit_test_bin/${DAISYHOST_UNIT_TEST_RUN_TAG}"
        SUFFIX ".bin"
)

set_target_properties(unit_tests
    PROPERTIES
        TEST_LAUNCHER
            "${DAISYHOST_PYTHON_LAUNCHER};-3;${PROJECT_SOURCE_DIR}/tests/run_unit_test_payload.py"
)

add_custom_command(
    TARGET unit_tests
    PRE_LINK
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:unit_tests>"
    VERBATIM
)

target_link_libraries(unit_tests
    PRIVATE
        daisyhost_cli
        daisyhost_hub_core
        daisyhost_render
        GTest::gtest
        GTest::gtest_main
)

add_test(
    NAME DaisyHostNextWpSuggester
    COMMAND
        ${DAISYHOST_PYTHON_LAUNCHER} -3
        ${PROJECT_SOURCE_DIR}/tests/test_next_wp_suggester.py
)

include(GoogleTest)

gtest_add_tests(TARGET unit_tests SOURCES ${DAISYHOST_UNIT_TEST_SOURCES})

set(DAISYHOST_SMOKE_SCRIPT "${PROJECT_SOURCE_DIR}/tests/run_smoke.py")

add_test(
    NAME DaisyHostStandaloneSmoke
    COMMAND
        ${DAISYHOST_PYTHON_LAUNCHER} -3
        ${DAISYHOST_SMOKE_SCRIPT}
        --mode standalone
        --build-dir ${PROJECT_BINARY_DIR}
        --source-dir ${PROJECT_SOURCE_DIR}
        --config $<CONFIG>
)

add_test(
    NAME DaisyHostRenderSmoke
    COMMAND
        ${DAISYHOST_PYTHON_LAUNCHER} -3
        ${DAISYHOST_SMOKE_SCRIPT}
        --mode render
        --build-dir ${PROJECT_BINARY_DIR}
        --source-dir ${PROJECT_SOURCE_DIR}
        --config $<CONFIG>
)

add_test(
    NAME DaisyHostCliListApps
    COMMAND
        $<TARGET_FILE:DaisyHostCLI>
        list-apps
        --json
)

add_test(
    NAME DaisyHostCliDescribeApp
    COMMAND
        $<TARGET_FILE:DaisyHostCLI>
        describe-app
        multidelay
        --json
)

add_test(
    NAME DaisyHostCliDescribeBoard
    COMMAND
        $<TARGET_FILE:DaisyHostCLI>
        describe-board
        daisy_field
        --json
)

add_test(
    NAME DaisyHostCliValidateScenario
    COMMAND
        $<TARGET_FILE:DaisyHostCLI>
        validate-scenario
        ${PROJECT_SOURCE_DIR}/training/examples/multidelay_smoke.json
        --json
)

add_test(
    NAME DaisyHostCliDoctor
    COMMAND
        $<TARGET_FILE:DaisyHostCLI>
        doctor
        --build-dir
        ${PROJECT_BINARY_DIR}
        --source-dir
        ${PROJECT_SOURCE_DIR}
        --config
        $<CONFIG>
        --json
)

add_test(
    NAME DaisyHostCliRender
    COMMAND
        $<TARGET_FILE:DaisyHostCLI>
        render
        ${PROJECT_SOURCE_DIR}/training/examples/multidelay_smoke.json
        --output-dir
        ${PROJECT_BINARY_DIR}/cli_smoke/multidelay
        --json
)

add_test(
    NAME DaisyHostCliRenderAssertions
    COMMAND
        $<TARGET_FILE:DaisyHostCLI>
        render
        ${PROJECT_SOURCE_DIR}/training/examples/multidelay_smoke.json
        --output-dir
        ${PROJECT_BINARY_DIR}/cli_smoke/render_assertions
        --expect-checksum
        definitely-not-the-checksum
        --json
)
set_tests_properties(DaisyHostCliRenderAssertions PROPERTIES WILL_FAIL TRUE)

add_test(
    NAME DaisyHostCliRenderAssertionsPass
    COMMAND
        $<TARGET_FILE:DaisyHostCLI>
        render
        ${PROJECT_SOURCE_DIR}/training/examples/multidelay_smoke.json
        --output-dir
        ${PROJECT_BINARY_DIR}/cli_smoke/render_assertions_pass
        --expect-non-silent
        --expect-timeline-target-node
        node0
        --json
)

set_tests_properties(
    DaisyHostStandaloneSmoke
    DaisyHostRenderSmoke
    DaisyHostCliListApps
    DaisyHostCliDescribeApp
    DaisyHostCliDescribeBoard
    DaisyHostCliValidateScenario
    DaisyHostCliDoctor
    DaisyHostCliRender
    DaisyHostCliRenderAssertions
    DaisyHostCliRenderAssertionsPass
    PROPERTIES
        LABELS "smoke"
        RUN_SERIAL TRUE
        TIMEOUT 120
)
