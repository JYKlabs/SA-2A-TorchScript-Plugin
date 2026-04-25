#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

#include <torch/script.h>

#include <map>
#include <mutex>
#include <atomic>
#include <vector>
#include <memory>

class SA2AAudioProcessor  : public juce::AudioProcessor
{
public:
    SA2AAudioProcessor();
    ~SA2AAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    double getTailLengthSeconds() const override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    // model management: only single file load
    bool loadSingleModelFile(const juce::File& file);
    void clearModels();

    // params exposed to editor
    std::atomic<float> inputGain  { 1.0f };   // linear gain
    std::atomic<float> peakReduction { 50.0f }; // 0..100
    std::atomic<float> outputGain { 1.0f };   // linear gain

private:
    std::shared_ptr<torch::jit::script::Module> model;
    std::mutex modelMutex;

    std::vector<std::vector<float>> tempChannelBuffer;
    int modelWindow = 512;
    int maxChannels = 2;
    double currentSampleRate = 44100.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SA2AAudioProcessor)
};
