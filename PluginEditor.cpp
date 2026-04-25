#include "PluginEditor.h"

using namespace juce;

SA2AAudioProcessorEditor::SA2AAudioProcessorEditor (SA2AAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    setSize (420, 220);

    // Input Gain
    inputGainSlider.setRange(0.0, 4.0, 0.001);
    inputGainSlider.setSliderStyle(Slider::Rotary);
    inputGainSlider.setTextBoxStyle(Slider::TextBoxBelow, true, 80, 20);
    addAndMakeVisible(inputGainSlider);

    inputGainLabel.setText("Input Gain", dontSendNotification);
    inputGainLabel.setJustificationType(Justification::centred);
    addAndMakeVisible(inputGainLabel);

    inputGainSlider.onValueChange = [this]() {
        processor.inputGain = (float) inputGainSlider.getValue();
    };

    // Peak Reduction
    peakReductionSlider.setRange(0.0, 100.0, 1.0);
    peakReductionSlider.setSliderStyle(Slider::Rotary);
    peakReductionSlider.setTextBoxStyle(Slider::TextBoxBelow, true, 80, 20);
    addAndMakeVisible(peakReductionSlider);

    peakReductionLabel.setText("Peak Reduction", dontSendNotification);
    peakReductionLabel.setJustificationType(Justification::centred);
    addAndMakeVisible(peakReductionLabel);

    peakReductionSlider.onValueChange = [this]() {
        processor.peakReduction = (float) peakReductionSlider.getValue();
    };

    // Output Gain
    outputGainSlider.setRange(0.0, 4.0, 0.001);
    outputGainSlider.setSliderStyle(Slider::Rotary);
    outputGainSlider.setTextBoxStyle(Slider::TextBoxBelow, true, 80, 20);
    addAndMakeVisible(outputGainSlider);

    outputGainLabel.setText("Output Gain", dontSendNotification);
    outputGainLabel.setJustificationType(Justification::centred);
    addAndMakeVisible(outputGainLabel);

    outputGainSlider.onValueChange = [this]() {
        processor.outputGain = (float) outputGainSlider.getValue();
    };

    // Buttons
    addAndMakeVisible(loadFileButton);
    loadFileButton.addListener(this);

    addAndMakeVisible(clearModelsButton);
    clearModelsButton.addListener(this);

    refreshSliderValuesFromProcessor();
}

SA2AAudioProcessorEditor::~SA2AAudioProcessorEditor()
{
    loadFileButton.removeListener(this);
    clearModelsButton.removeListener(this);
}

void SA2AAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (Colours::darkgrey);
    g.setColour (Colours::white);
    g.setFont (15.0f);
    g.drawText ("SA-2A model", getLocalBounds().withTrimmedTop(8).withHeight(22),
                Justification::centred, false);
}

void SA2AAudioProcessorEditor::resized()
{
    auto r = getLocalBounds().reduced(12);
    auto top = r.removeFromTop(40);
    auto row = r.removeFromTop(120);
    auto w = row.getWidth() / 3;

    Rectangle<int> r1 = row.removeFromLeft(w).reduced(8);
    inputGainSlider.setBounds(r1.removeFromTop(r1.getHeight() - 26));
    inputGainLabel.setBounds(r1.removeFromBottom(26));

    Rectangle<int> r2 = row.removeFromLeft(w).reduced(8);
    peakReductionSlider.setBounds(r2.removeFromTop(r2.getHeight() - 26));
    peakReductionLabel.setBounds(r2.removeFromBottom(26));

    Rectangle<int> r3 = row.removeFromLeft(w).reduced(8);
    outputGainSlider.setBounds(r3.removeFromTop(r3.getHeight() - 26));
    outputGainLabel.setBounds(r3.removeFromBottom(26));

    loadFileButton.setBounds(10, getHeight() - 56, 180, 36);
    clearModelsButton.setBounds(200, getHeight() - 56, 180, 36);
}

void SA2AAudioProcessorEditor::buttonClicked(juce::Button* b)
{
    if (b == &loadFileButton)
    {
        auto chooser = new FileChooser ("Select TorchScript model (.pt)", {}, "*.pt");
        chooser->launchAsync(FileBrowserComponent::canSelectFiles,
            [this, chooser] (const FileChooser& fc)
            {
                File f = fc.getResult();
                bool ok = false;
                if (f.existsAsFile())
                {
                    ok = processor.loadSingleModelFile(f);
                }
                const String msg = ok ? "Loaded model file." : "Failed to load model file.";
                MessageManager::callAsync([msg]() {
                    AlertWindow::showMessageBoxAsync(AlertWindow::InfoIcon, String("Load Model"), msg);
                });
                delete chooser;
            });
    }
    else if (b == &clearModelsButton)
    {
        processor.clearModels();
        MessageManager::callAsync([]() {
            AlertWindow::showMessageBoxAsync(AlertWindow::InfoIcon, "Model", "Cleared loaded model.");
        });
    }
}

void SA2AAudioProcessorEditor::refreshSliderValuesFromProcessor()
{
    inputGainSlider.setValue((double) processor.inputGain.load(), dontSendNotification);
    peakReductionSlider.setValue((double) processor.peakReduction.load(), dontSendNotification);
    outputGainSlider.setValue((double) processor.outputGain.load(), dontSendNotification);
}
