#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"

class SA2AAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                  private juce::Button::Listener
{
public:
    SA2AAudioProcessorEditor (SA2AAudioProcessor&);
    ~SA2AAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    SA2AAudioProcessor& processor;

    juce::Slider inputGainSlider;
    juce::Label  inputGainLabel;

    juce::Slider peakReductionSlider;
    juce::Label  peakReductionLabel;

    juce::Slider outputGainSlider;
    juce::Label  outputGainLabel;

    // Buttons: only Load single file + Clear
    juce::TextButton loadFileButton { "Load Model (.pt)" };
    juce::TextButton clearModelsButton { "Clear Model" };

    void buttonClicked(juce::Button* b) override;
    void refreshSliderValuesFromProcessor();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SA2AAudioProcessorEditor)
};
