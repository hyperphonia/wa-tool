#pragma once

#include "PluginProcessor.h"
#include "ToolLookAndFeel.h"

class ToolAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit ToolAudioProcessorEditor (ToolAudioProcessor&);
    ~ToolAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    class ValueReadout : public juce::Component
    {
    public:
        void setValue (float newValue);
        void paint (juce::Graphics&) override;

    private:
        float value = 0.0f;
    };

    void refreshLogo (float scale);
    juce::Rectangle<float> logoBounds() const;

    ToolAudioProcessor& processorRef;
    ToolLookAndFeel lnf;

    juce::Image logo, scaledLogo;
    float logoScale = 0.0f;

    juce::Slider gainKnob;
    ValueReadout readout;
    juce::TextButton invertButton, monoButton;

    juce::AudioProcessorValueTreeState::SliderAttachment gainAttachment;
    juce::AudioProcessorValueTreeState::ButtonAttachment invertAttachment, monoAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ToolAudioProcessorEditor)
};
