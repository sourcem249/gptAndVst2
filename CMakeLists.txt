cmake_minimum_required(VERSION 3.15)

project(CLA2AStyleProcessor VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT DEFINED JUCE_DIR)
    message(FATAL_ERROR "JUCE_DIR must point to a valid JUCE checkout (see README.md)")
endif()

add_subdirectory(${JUCE_DIR} JUCE)

juce_add_plugin(CLA2AStyle
    COMPANY_NAME "GPT Audio"
    BUNDLE_ID "com.gptaudio.cla2astyle"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE GPTA
    PLUGIN_CODE ClA2
    FORMATS VST3 Standalone
    PRODUCT_NAME "CLA2A Style"
)

juce_generate_juce_header(CLA2AStyle)

set(SOURCE_FILES
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
)

target_sources(CLA2AStyle PRIVATE ${SOURCE_FILES})

target_compile_definitions(CLA2AStyle
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_MODAL_LOOPS_PERMITTED=1
)

target_link_libraries(CLA2AStyle
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
)
