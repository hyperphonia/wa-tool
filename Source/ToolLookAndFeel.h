#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace ToolColours
{
    inline const juce::Colour bg           { 0xff1e2220 };
    inline const juce::Colour surface      { 0xff272c29 };
    inline const juce::Colour surfaceHover { 0xff313833 };
    inline const juce::Colour line         { 0xff3b423e };
    inline const juce::Colour text         { 0xffedf0ee };
    inline const juce::Colour textDim      { 0xff8e9b93 };
    inline const juce::Colour accent       { 0xff45ce7b };
}

class ToolLookAndFeel : public juce::LookAndFeel_V4
{
public:
    ToolLookAndFeel();

    static juce::Font uiFont (float height, bool bold = false, float kerning = 0.0f);
    static juce::Font valueFont (float height);

    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPos, float startAngle, float endAngle,
                           juce::Slider&) override;

    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&,
                               bool isMouseOver, bool isMouseDown) override;

    void drawButtonText (juce::Graphics&, juce::TextButton&,
                         bool isMouseOver, bool isMouseDown) override;
};
