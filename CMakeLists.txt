cmake_minimum_required(VERSION 3.22)

project(VirtualAmpSystem VERSION 1.0.0 LANGUAGES CXX)

# --- JUCE Setup ---
include(FetchContent)
FetchContent_Declare(
    juce
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        origin/master
)
FetchContent_MakeAvailable(juce)

# --- Plugin Configuration ---
juce_add_plugin(VirtualAmpSystem
    COMPANY_NAME "Xraight"
    IS_PLUG_IN TRUE
    PLUGIN_MANUFACTURER_CODE "Xrta"
    PLUGIN_CODE "Vamp"
    FORMATS VST3 Standalone
    PRODUCT_NAME "Virtual Amp System"
)

# --- Source Files ---
target_sources(VirtualAmpSystem PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
)

# --- Assets / Binary Data ---
# Note: This creates a target named BinaryData and generates BinaryData.h
juce_add_binary_data(BinaryData
    SOURCES
        Resources/PIEL.png
)

# --- Dependencies ---
target_link_libraries(VirtualAmpSystem
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_dsp
        juce::juce_gui_extra
        juce::juce_audio_plugin_client
        BinaryData
)

# --- Compiler Settings ---
target_compile_features(VirtualAmpSystem PRIVATE cxx_std_17)
target_compile_definitions(VirtualAmpSystem PRIVATE
    JUCE_VST3_CAN_REPLACE_VST2=0
)

# Generating JuceHeader.h for compatibility with existing code
juce_generate_juce_header(VirtualAmpSystem)

# Ensure the generated BinaryData header can be found as "BinaryData.h"
# juce_add_binary_data handles this by adding the include path to the target.
