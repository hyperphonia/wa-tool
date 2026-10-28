cmake_minimum_required(VERSION 3.22)
project(Tool VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(TOOL_BUILD_SHOTS "Build the offline editor screenshot renderer" OFF)
option(TOOL_BUILD_TESTS "Build the offline DSP checks" OFF)
option(TOOL_COPY_PLUGIN_AFTER_BUILD "Copy the built VST3 into the system plugin folder" ON)

include(FetchContent)

# Reuse a local checkout instead of re-downloading JUCE.
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/libs/JUCE/CMakeLists.txt")
    set(FETCHCONTENT_SOURCE_DIR_JUCE "${CMAKE_CURRENT_SOURCE_DIR}/libs/JUCE" CACHE PATH "" FORCE)
endif()

FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.15
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(JUCE)

set(TOOL_PLUGIN_SOURCES
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
    Source/ToolLookAndFeel.cpp
    Source/ToolLookAndFeel.h
)

juce_add_plugin(Tool
    PRODUCT_NAME              "Tool"
    COMPANY_NAME              "Wonderful Audio"
    BUNDLE_ID                 "com.wonderfulaudio.tool"
    PLUGIN_MANUFACTURER_CODE  WnAu
    PLUGIN_CODE               WaLt
    FORMATS                   VST3 Standalone
    VST3_CATEGORIES           Fx
    IS_SYNTH                  FALSE
    NEEDS_MIDI_INPUT          FALSE
    NEEDS_MIDI_OUTPUT         FALSE
    IS_MIDI_EFFECT            FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD   ${TOOL_COPY_PLUGIN_AFTER_BUILD}
    VERSION                   "1.0.0"
    DESCRIPTION               "Pocket utility: output gain, polarity invert, mono"
)

juce_add_binary_data(Tool_Data
    NAMESPACE BinaryData
    SOURCES
        WA_logo_transparent.png
)

target_sources(Tool PRIVATE ${TOOL_PLUGIN_SOURCES})

target_compile_definitions(Tool
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(Tool
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
        Tool_Data
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
)

# What juce_add_plugin normally injects; the dev targets compile the same sources directly.
set(TOOL_DEV_DEFINITIONS
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JucePlugin_Name="Tool"
    JucePlugin_Manufacturer="Wonderful Audio"
    JucePlugin_ManufacturerCode="WnAu"
    JucePlugin_PluginCode="WaLt"
    JucePlugin_Version="1.0.0"
    JucePlugin_VersionString="1.0.0"
    JucePlugin_IsSynth=0
    JucePlugin_WantsMidiInput=0
    JucePlugin_ProducesMidiOutput=0
    JucePlugin_IsMidiEffect=0
    JucePlugin_EditorRequiresKeyboardFocus=0
    JucePlugin_Desc="Pocket utility"
)

if(TOOL_BUILD_SHOTS)
    juce_add_gui_app(ToolShots
        PRODUCT_NAME "ToolShots"
        COMPANY_NAME "Wonderful Audio"
        BUNDLE_ID    "com.wonderfulaudio.toolshots"
    )

    target_sources(ToolShots PRIVATE tools/RenderShots.cpp ${TOOL_PLUGIN_SOURCES})
    target_include_directories(ToolShots PRIVATE Source)
    target_compile_definitions(ToolShots PRIVATE ${TOOL_DEV_DEFINITIONS})
    target_link_libraries(ToolShots
        PRIVATE
            juce::juce_audio_utils
            juce::juce_dsp
            Tool_Data
        PUBLIC
            juce::juce_recommended_config_flags
    )
endif()

if(TOOL_BUILD_TESTS)
    juce_add_console_app(ToolTests PRODUCT_NAME "ToolTests")

    target_sources(ToolTests PRIVATE tools/DspTests.cpp ${TOOL_PLUGIN_SOURCES})
    target_include_directories(ToolTests PRIVATE Source)
    target_compile_definitions(ToolTests PRIVATE ${TOOL_DEV_DEFINITIONS})
    target_link_libraries(ToolTests
        PRIVATE
            juce::juce_audio_utils
            juce::juce_dsp
            Tool_Data
        PUBLIC
            juce::juce_recommended_config_flags
    )
endif()
