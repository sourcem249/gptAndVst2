#pragma once

#include <JuceHeader.h>

class CLA2AStyleAudioProcessor;

class CLA2AStyleAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit CLA2AStyleAudioProcessorEditor(CLA2AStyleAudioProcessor&);
    ~CLA2AStyleAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void configureSlider(juce::Slider& slider, const juce::String& labelText);

    CLA2AStyleAudioProcessor& processorRef;

    juce::Slider gainSlider;
    juce::Slider compressionSlider;
    juce::ToggleButton deEsserButton { "De-esser" };

    juce::Label gainLabel;
    juce::Label compressionLabel;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> compressionAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> deEsserAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CLA2AStyleAudioProcessorEditor)
};
