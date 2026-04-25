#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace juce;

// -----------------------------
// Constructor / destructor
// -----------------------------
SA2AAudioProcessor::SA2AAudioProcessor()
    : AudioProcessor (BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
                      .withInput  ("Input",  AudioChannelSet::stereo(), true)
#endif
                      .withOutput ("Output", AudioChannelSet::stereo(), true)
#endif
                      )
{
    tempChannelBuffer.resize(maxChannels);
    for (int ch = 0; ch < maxChannels; ++ch)
        tempChannelBuffer[ch].assign(modelWindow, 0.0f);
    maxChannels = 2;
}

SA2AAudioProcessor::~SA2AAudioProcessor()
{
}

// -----------------------------
// Basic metadata / program API
// -----------------------------
const String SA2AAudioProcessor::getName() const { return JucePlugin_Name; }
bool SA2AAudioProcessor::acceptsMidi() const { return false; }
bool SA2AAudioProcessor::producesMidi() const { return false; }
double SA2AAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int SA2AAudioProcessor::getNumPrograms() { return 1; }
int SA2AAudioProcessor::getCurrentProgram() { return 0; }
void SA2AAudioProcessor::setCurrentProgram (int) {}
const juce::String SA2AAudioProcessor::getProgramName (int) { return {}; }
void SA2AAudioProcessor::changeProgramName (int, const juce::String&) {}

// -----------------------------
// prepare / release
// -----------------------------
void SA2AAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    tempChannelBuffer.resize(maxChannels);
        for (int ch = 0; ch < maxChannels; ++ch)
            tempChannelBuffer[ch].assign(modelWindow, 0.0f);

}

void SA2AAudioProcessor::releaseResources()
{
}

// -----------------------------
// isBusesLayoutSupported
// -----------------------------
bool SA2AAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;
#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif
    return true;
}

// -----------------------------
// load / clear model
// -----------------------------
bool SA2AAudioProcessor::loadSingleModelFile(const juce::File& file)
{
    if (!file.existsAsFile()) return false;
    if (!file.hasFileExtension("pt") && !file.hasFileExtension("PT")) return false;
    try {
        auto loaded = std::make_shared<torch::jit::script::Module>(torch::jit::load(file.getFullPathName().toStdString()));
        loaded->eval();
        std::lock_guard<std::mutex> g(modelMutex);
        model = loaded;
        DBG("SA2A: loaded model: " << file.getFullPathName());
        return true;
    } catch (const std::exception& e) {
        DBG("SA2A: load model exception: " << e.what());
        return false;
    } catch (...) {
        DBG("SA2A: unknown exception loading model");
        return false;
    }
}

void SA2AAudioProcessor::clearModels()
{
    std::lock_guard<std::mutex> g(modelMutex);
    model.reset();
    DBG("SA2A: cleared model");
}

// -----------------------------
// processBlock (synchronous)
// - If host block size != modelWindow, passthrough (safe).
// -----------------------------
void SA2AAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer&)
{
    const int numChannels = buffer.getNumChannels();
    const int numSamples  = buffer.getNumSamples();
    
    for (int ch = 0; ch < numChannels; ++ch)
    {
        auto& temp = tempChannelBuffer[ch];
        const float* r = buffer.getReadPointer(ch);

        // copy existing samples
        int copyN = std::min(numSamples, modelWindow);
        for (int i = 0; i < copyN; ++i)
            temp[i] = r[i] * inputGain.load();

        // zero-pad remainder
        for (int i = copyN; i < modelWindow; ++i)
            temp[i] = 0.0f;
    }

    // snapshot model
    std::shared_ptr<torch::jit::script::Module> mod;
    { std::lock_guard<std::mutex> g(modelMutex); mod = model; }

    float inG  = inputGain.load();
    float outG = outputGain.load();

    torch::NoGradGuard no_grad;
    auto options = torch::TensorOptions().dtype(torch::kFloat32).device(torch::kCPU);
    torch::Tensor peakTensor = torch::tensor({{ peakReduction.load() }}, options);

    for (int ch = 0; ch < numChannels; ++ch)
    {
        auto& temp = tempChannelBuffer[ch];
        const float* r = buffer.getReadPointer(ch);

        // copy + input gain
        for (int i = 0; i < modelWindow; ++i)
            temp[i] = r[i] * inG;

        if (mod)
        {
            try
            {
                torch::Tensor inputTensor =
                    torch::from_blob(temp.data(), {1,1,modelWindow}, options).clone();

                auto rawOut = mod->forward({ inputTensor, peakTensor });
                torch::Tensor outT = rawOut.toTensor().contiguous();

                int outLen = modelWindow;
                if (outT.dim() == 3)      outLen = (int)outT.size(2);
                else if (outT.dim() == 2) outLen = (int)outT.size(1);
                else if (outT.dim() == 1) outLen = (int)outT.size(0);

                const float* outPtr = outT.data_ptr<float>();
                float* w = buffer.getWritePointer(ch);

                int copyN = std::min(outLen, modelWindow);
                for (int i = 0; i < copyN; ++i)
                    w[i] = outPtr[i] * outG;

                for (int i = copyN; i < modelWindow; ++i)
                    w[i] = 0.0f;
            }
            catch (...)
            {
                float* w = buffer.getWritePointer(ch);
                for (int i = 0; i < modelWindow; ++i)
                    w[i] = temp[i] * outG;
            }
        }
        else
        {
            float* w = buffer.getWritePointer(ch);
            for (int i = 0; i < modelWindow; ++i)
                w[i] = temp[i] * outG;
        }
    }
}


// -----------------------------
// Editor creation
// -----------------------------
juce::AudioProcessorEditor* SA2AAudioProcessor::createEditor()
{
    return new SA2AAudioProcessorEditor(*this);
}

bool SA2AAudioProcessor::hasEditor() const { return true; }

// -----------------------------
// state (no-op)
void SA2AAudioProcessor::getStateInformation (MemoryBlock& destData) {}
void SA2AAudioProcessor::setStateInformation (const void* data, int sizeInBytes) {}

// -----------------------------
// Factory symbol
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SA2AAudioProcessor();
}
