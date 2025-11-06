#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
constexpr float kMinGainDb = -24.0f;
constexpr float kMaxGainDb = 24.0f;
constexpr float kDefaultGainDb = 0.0f;

constexpr float kMinCompression = 0.0f;
constexpr float kMaxCompression = 1.0f;
constexpr float kDefaultCompression = 0.5f;
}

CLA2AStyleAudioProcessor::CLA2AStyleAudioProcessor()
    : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                                       .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "Parameters", createParameterLayout())
{
}

CLA2AStyleAudioProcessor::~CLA2AStyleAudioProcessor() = default;

juce::AudioProcessorValueTreeState::ParameterLayout CLA2AStyleAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "inputGain",
        "Gain",
        juce::NormalisableRange<float>(kMinGainDb, kMaxGainDb),
        kDefaultGainDb,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(value, 1) + " dB"; },
        [](const juce::String& text) { return text.dropLastCharacters(3).getFloatValue(); }));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "compression",
        "Comp",
        juce::NormalisableRange<float>(kMinCompression, kMaxCompression),
        kDefaultCompression));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "deEsser",
        "De-esser",
        false));

    return { params.begin(), params.end() };
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool CLA2AStyleAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    auto mainInput = layouts.getMainInputChannelSet();
    auto mainOutput = layouts.getMainOutputChannelSet();

    if (mainInput != juce::AudioChannelSet::mono() && mainInput != juce::AudioChannelSet::stereo())
        return false;

    if (mainOutput != juce::AudioChannelSet::mono() && mainOutput != juce::AudioChannelSet::stereo())
        return false;

    return mainInput == mainOutput;
}
#endif

void CLA2AStyleAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused(samplesPerBlock);
    currentSampleRate = sampleRate;

    channelStates.assign(static_cast<size_t>(getTotalNumOutputChannels()), {});
    updateCompressorSettings();
}

void CLA2AStyleAudioProcessor::releaseResources()
{
}

void CLA2AStyleAudioProcessor::updateCompressorSettings()
{
    auto gainDb = parameters.getRawParameterValue("inputGain");
    auto comp = parameters.getRawParameterValue("compression");
    auto deEsser = parameters.getRawParameterValue("deEsser");

    float gainValue = kDefaultGainDb;
    if (gainDb != nullptr)
        gainValue = gainDb->load();

    float compValue = kDefaultCompression;
    if (comp != nullptr)
        compValue = comp->load();

    float deEsserValue = 0.0f;
    if (deEsser != nullptr)
        deEsserValue = deEsser->load();

    inputGainLinear = juce::Decibels::decibelsToGain(gainValue);
    compressionControl = compValue;
    deEsserEnabled = deEsserValue >= 0.5f;

    const float minThresholdDb = -35.0f;
    const float maxThresholdDb = -10.0f;
    thresholdDb = juce::jmap(compressionControl, maxThresholdDb, minThresholdDb);

    const float minRatio = 2.0f;
    const float maxRatio = 8.0f;
    ratio = juce::jmap(compressionControl, minRatio, maxRatio);

    const float attackMs = juce::jmap(compressionControl, 10.0f, 3.0f);
    const float releaseMs = juce::jmap(compressionControl, 120.0f, 600.0f);

    attackCoefficient = std::exp(-1.0f / (0.001f * attackMs * static_cast<float>(currentSampleRate)));
    releaseCoefficient = std::exp(-1.0f / (0.001f * releaseMs * static_cast<float>(currentSampleRate)));

    const float deEsserCutoffHz = 3500.0f;
    deEsserLowpassCoeff = std::exp(-2.0f * juce::MathConstants<float>::pi * deEsserCutoffHz / static_cast<float>(currentSampleRate));

    const float deEsserAttackMs = 5.0f;
    const float deEsserReleaseMs = 80.0f;
    deEsserAttackCoeff = std::exp(-1.0f / (0.001f * deEsserAttackMs * static_cast<float>(currentSampleRate)));
    deEsserReleaseCoeff = std::exp(-1.0f / (0.001f * deEsserReleaseMs * static_cast<float>(currentSampleRate)));

    deEsserThresholdDb = juce::jmap(compressionControl, -30.0f, -18.0f);
    deEsserMaxReductionDb = juce::jmap(compressionControl, 6.0f, 14.0f);
}

float CLA2AStyleAudioProcessor::applyCompressor(ChannelState& state, float sample) const
{
    float magnitude = std::abs(sample) + 1.0e-6f;

    if (magnitude > state.envelope)
        state.envelope = attackCoefficient * state.envelope + (1.0f - attackCoefficient) * magnitude;
    else
        state.envelope = releaseCoefficient * state.envelope + (1.0f - releaseCoefficient) * magnitude;

    auto levelDb = juce::Decibels::gainToDecibels(state.envelope);
    auto overDb = levelDb - thresholdDb;

    float gainReductionDb = 0.0f;
    if (overDb > 0.0f)
    {
        const float slope = 1.0f - (1.0f / juce::jmax(1.0f, ratio));
        gainReductionDb = juce::jlimit(0.0f, 24.0f, overDb * slope);
    }

    float gainReduction = juce::Decibels::decibelsToGain(-gainReductionDb);
    return sample * gainReduction;
}

float CLA2AStyleAudioProcessor::applyDeEsser(ChannelState& state, float sample) const
{
    float low = state.deEsserLow;
    low = deEsserLowpassCoeff * low + (1.0f - deEsserLowpassCoeff) * sample;
    state.deEsserLow = low;

    float highBand = sample - low;
    float magnitude = std::abs(highBand) + 1.0e-6f;

    if (magnitude > state.deEsserEnvelope)
        state.deEsserEnvelope = deEsserAttackCoeff * state.deEsserEnvelope + (1.0f - deEsserAttackCoeff) * magnitude;
    else
        state.deEsserEnvelope = deEsserReleaseCoeff * state.deEsserEnvelope + (1.0f - deEsserReleaseCoeff) * magnitude;

    float highDb = juce::Decibels::gainToDecibels(state.deEsserEnvelope);
    float over = highDb - deEsserThresholdDb;

    if (over <= 0.0f)
        return sample;

    float reductionDb = juce::jlimit(0.0f, deEsserMaxReductionDb, over);
    float reduction = juce::Decibels::decibelsToGain(-reductionDb);
    return sample * reduction;
}

void CLA2AStyleAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    juce::ignoreUnused(midiMessages);

    auto totalInputChannels = getTotalNumInputChannels();
    auto totalOutputChannels = getTotalNumOutputChannels();

    updateCompressorSettings();

    for (int channel = totalInputChannels; channel < totalOutputChannels; ++channel)
        buffer.clear(channel, 0, buffer.getNumSamples());

    if (static_cast<int>(channelStates.size()) < totalOutputChannels)
        channelStates.resize(static_cast<size_t>(totalOutputChannels));

    for (int channel = 0; channel < totalInputChannels; ++channel)
    {
        auto* samples = buffer.getWritePointer(channel);
        auto& state = channelStates[static_cast<size_t>(channel)];

        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            float processed = samples[i] * inputGainLinear;
            processed = applyCompressor(state, processed);

            if (deEsserEnabled)
                processed = applyDeEsser(state, processed);

            samples[i] = processed;
        }
    }
}

void CLA2AStyleAudioProcessor::processBlock(juce::AudioBuffer<double>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::AudioBuffer<float> tempBuffer(buffer.getNumChannels(), buffer.getNumSamples());

    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto* src = buffer.getReadPointer(channel);
        auto* dst = tempBuffer.getWritePointer(channel);

        for (int i = 0; i < buffer.getNumSamples(); ++i)
            dst[i] = static_cast<float>(src[i]);
    }

    processBlock(tempBuffer, midiMessages);

    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto* src = tempBuffer.getReadPointer(channel);
        auto* dst = buffer.getWritePointer(channel);

        for (int i = 0; i < buffer.getNumSamples(); ++i)
            dst[i] = static_cast<double>(src[i]);
    }
}

void CLA2AStyleAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    if (auto xml = parameters.copyState().createXml())
        copyXmlToBinary(*xml, destData);
}

void CLA2AStyleAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xmlState = getXmlFromBinary(data, sizeInBytes))
        parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorEditor* CLA2AStyleAudioProcessor::createEditor()
{
    return new CLA2AStyleAudioProcessorEditor(*this);
}
