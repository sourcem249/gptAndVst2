#include "PluginEditor.h"
#include "PluginProcessor.h"

namespace
{
constexpr int kDefaultWidth = 400;
constexpr int kDefaultHeight = 220;
constexpr int kDialSize = 120;
constexpr int kDialSpacing = 20;
constexpr int kFooterHeight = 40;
}

CLA2AStyleAudioProcessorEditor::CLA2AStyleAudioProcessorEditor(CLA2AStyleAudioProcessor& processor)
    : AudioProcessorEditor(&processor), processorRef(processor)
{
    setSize(kDefaultWidth, kDefaultHeight);
    setResizable(false, false);

    configureSlider(gainSlider, "Gain");
    configureSlider(compressionSlider, "Comp");

    deEsserButton.setClickingTogglesState(true);
    deEsserButton.setTooltip("Toggle the high-frequency de-esser stage");
    addAndMakeVisible(deEsserButton);

    auto& valueTree = processorRef.getValueTreeState();
    gainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(valueTree, "inputGain", gainSlider);
    compressionAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(valueTree, "compression", compressionSlider);
    deEsserAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(valueTree, "deEsser", deEsserButton);
}

CLA2AStyleAudioProcessorEditor::~CLA2AStyleAudioProcessorEditor() = default;

void CLA2AStyleAudioProcessorEditor::configureSlider(juce::Slider& slider, const juce::String& labelText)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 20);
    slider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::orange);
    slider.setColour(juce::Slider::thumbColourId, juce::Colours::white);
    addAndMakeVisible(slider);

    juce::Label* label = nullptr;
    if (labelText == "Gain")
        label = &gainLabel;
    else if (labelText == "Comp")
        label = &compressionLabel;

    if (label != nullptr)
    {
        label->setText(labelText, juce::dontSendNotification);
        label->setJustificationType(juce::Justification::centred);
        label->setColour(juce::Label::textColourId, juce::Colours::white);
        label->attachToComponent(&slider, false);
        addAndMakeVisible(*label);
    }
}

void CLA2AStyleAudioProcessorEditor::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    juce::ColourGradient gradient(juce::Colours::darkslategrey, bounds.getTopLeft().toFloat(),
                                  juce::Colours::black, bounds.getBottomRight().toFloat(), true);

    g.setGradientFill(gradient);
    g.fillAll();

    g.setColour(juce::Colours::white.withAlpha(0.8f));
    g.setFont(juce::Font(22.0f, juce::Font::bold));
    g.drawFittedText("CLA2A Inspired Leveler", bounds.removeFromTop(40), juce::Justification::centred, 1);

    g.setFont(juce::Font(14.0f));
    g.drawFittedText("Smooth vocal leveling with stereo support", bounds.removeFromBottom(kFooterHeight), juce::Justification::centred, 1);
}

void CLA2AStyleAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(20);
    auto topArea = area.removeFromTop(kDialSize);

    auto gainArea = topArea.removeFromLeft(kDialSize);
    gainSlider.setBounds(gainArea.withSizeKeepingCentre(kDialSize, kDialSize));

    topArea.removeFromLeft(kDialSpacing);

    auto compArea = topArea.removeFromLeft(kDialSize);
    compressionSlider.setBounds(compArea.withSizeKeepingCentre(kDialSize, kDialSize));

    area.removeFromTop(20);
    auto buttonArea = area.removeFromTop(30);
    deEsserButton.setBounds(buttonArea.withSizeKeepingCentre(140, 24));
}
