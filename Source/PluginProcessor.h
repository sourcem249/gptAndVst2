#pragma once

#include <JuceHeader.h>

class CLA2AStyleAudioProcessor : public juce::AudioProcessor
{
public:
    CLA2AStyleAudioProcessor();
    ~CLA2AStyleAudioProcessor() override;

    // AudioProcessor overrides
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void processBlock(juce::AudioBuffer<double>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }

    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int index) override { juce::ignoreUnused(index); }
    const juce::String getProgramName(int index) override { juce::ignoreUnused(index); return {}; }
    void changeProgramName(int index, const juce::String& newName) override { juce::ignoreUnused(index, newName); }

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getValueTreeState() { return parameters; }

private:
    struct ChannelState
    {
        float envelope = 0.0f;
        float deEsserEnvelope = 0.0f;
        float deEsserLow = 0.0f;
    };

    void updateCompressorSettings();
    float applyCompressor(ChannelState& state, float sample) const;
    float applyDeEsser(ChannelState& state, float sample) const;

    juce::AudioProcessorValueTreeState parameters;

    std::vector<ChannelState> channelStates;

    double currentSampleRate = 44100.0;
    float inputGainLinear = 1.0f;
    float compressionControl = 0.5f;
    bool deEsserEnabled = false;

    float thresholdDb = -18.0f;
    float ratio = 4.0f;
    float attackCoefficient = 0.9f;
    float releaseCoefficient = 0.99f;

    float deEsserLowpassCoeff = 0.1f;
    float deEsserAttackCoeff = 0.8f;
    float deEsserReleaseCoeff = 0.98f;
    float deEsserThresholdDb = -18.0f;
    float deEsserMaxReductionDb = 12.0f;

    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CLA2AStyleAudioProcessor)
};
