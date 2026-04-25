# Source/CMakeLists.txt
juce_add_plugin(SA2A
    PLUGIN_MANUFACTURER_CODE MyCompany
    PLUGIN_CODE SA2A
    FORMATS VST3 AU Standalone
    PRODUCT_NAME "SA-2A"
)

target_sources(SA2A PRIVATE
    PluginProcessor.cpp
    PluginProcessor.h
    PluginEditor.cpp
    PluginEditor.h
)

target_compile_definitions(SA2A PRIVATE JUCE_WEB_BROWSER=0)

# Link libs
target_link_libraries(SA2A PRIVATE
    juce::juce_audio_utils
    ${TORCH_LIBRARIES}
)

# Torch flags (example)
set_property(TARGET SA2A PROPERTY CXX_STANDARD 17)
# If Torch sets TORCH_CXX_FLAGS, append them:
if(TORCH_CXX_FLAGS)
  target_compile_options(SA2A PRIVATE ${TORCH_CXX_FLAGS})
endif()
