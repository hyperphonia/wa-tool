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
    class GainKnob : public juce::Slider
    {
    public:
        std::function<void()> onContextMenu;

    private:
        void mouseDown (const juce::MouseEvent&) override;
        void mouseUp (const juce::MouseEvent&) override;

        bool contextMenuClick = false;
    };

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

    void showGainMenu();
    void promptForGain();
    void setGainFromText (const juce::String& text);

    ToolAudioProcessor& processorRef;
    ToolLookAndFeel lnf;

    juce::Image logo, scaledLogo;
    float logoScale = 0.0f;

    GainKnob gainKnob;
    ValueReadout readout;
    juce::TextButton invertButton, monoButton;

    juce::AudioProcessorValueTreeState::SliderAttachment gainAttachment;
    juce::AudioProcessorValueTreeState::ButtonAttachment invertAttachment, monoAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ToolAudioProcessorEditor)
};
