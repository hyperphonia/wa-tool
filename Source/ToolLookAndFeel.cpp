#include "ToolLookAndFeel.h"

ToolLookAndFeel::ToolLookAndFeel()
{
    setColour (juce::ResizableWindow::backgroundColourId, ToolColours::bg);
    setColour (juce::TextButton::buttonColourId, ToolColours::surface);
    setColour (juce::TextButton::buttonOnColourId, ToolColours::accent);
    setColour (juce::TextButton::textColourOffId, ToolColours::textDim);
    setColour (juce::TextButton::textColourOnId, ToolColours::bg);
}

juce::Font ToolLookAndFeel::uiFont (float height, bool bold, float kerning)
{
    return juce::Font (juce::FontOptions (juce::Font::getDefaultSansSerifFontName(), height,
                                          bold ? juce::Font::bold : juce::Font::plain)
                           .withKerningFactor (kerning));
}

juce::Font ToolLookAndFeel::valueFont (float height)
{
    return juce::Font (juce::FontOptions (juce::Font::getDefaultMonospacedFontName(), height,
                                          juce::Font::plain));
}

void ToolLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                        float sliderPos, float startAngle, float endAngle,
                                        juce::Slider&)
{
    const auto centre = juce::Point<float> (x + width * 0.5f, y + height * 0.5f);
    const auto half = juce::jmin (width, height) * 0.5f;

    const auto arcRadius = half * 0.82f;
    const auto discRadius = half * 0.72f;
    const auto angle = startAngle + sliderPos * (endAngle - startAngle);
    const auto midAngle = startAngle + 0.5f * (endAngle - startAngle);

    constexpr float arcThickness = 3.5f;

    {
        const auto dir = juce::Point<float> (std::sin (midAngle), -std::cos (midAngle));
        const auto inner = half * 0.905f;
        const auto outer = half * 0.965f;

        g.setColour (ToolColours::textDim);
        g.drawLine ({ centre + dir * inner, centre + dir * outer }, 1.6f);
    }

    {
        juce::Path track;
        track.addCentredArc (centre.x, centre.y, arcRadius, arcRadius, 0.0f,
                             startAngle, endAngle, true);
        g.setColour (ToolColours::line);
        g.strokePath (track, juce::PathStrokeType (arcThickness, juce::PathStrokeType::curved,
                                                   juce::PathStrokeType::rounded));
    }

    if (std::abs (angle - midAngle) > 0.002f)
    {
        juce::Path fill;
        fill.addCentredArc (centre.x, centre.y, arcRadius, arcRadius, 0.0f,
                            juce::jmin (midAngle, angle), juce::jmax (midAngle, angle), true);
        g.setColour (ToolColours::accent);
        g.strokePath (fill, juce::PathStrokeType (arcThickness, juce::PathStrokeType::curved,
                                                  juce::PathStrokeType::rounded));
    }

    {
        const auto disc = juce::Rectangle<float> (discRadius * 2.0f, discRadius * 2.0f)
                              .withCentre (centre);

        g.setColour (ToolColours::surface);
        g.fillEllipse (disc);

        g.setColour (ToolColours::line);
        g.drawEllipse (disc.reduced (0.5f), 1.0f);
    }

    {
        const auto dir = juce::Point<float> (std::sin (angle), -std::cos (angle));
        g.setColour (ToolColours::text);
        g.drawLine ({ centre + dir * (discRadius * 0.26f),
                      centre + dir * (discRadius - 1.0f) },
                    2.6f);
    }
}

void ToolLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& button,
                                            const juce::Colour&, bool isMouseOver, bool isMouseDown)
{
    const auto bounds = button.getLocalBounds().toFloat().reduced (0.5f);
    const auto active = button.getToggleState();

    juce::Colour fill;

    if (active)
        fill = isMouseOver ? ToolColours::accent.brighter (0.10f) : ToolColours::accent;
    else if (isMouseDown)
        fill = ToolColours::surfaceHover.brighter (0.06f);
    else if (isMouseOver)
        fill = ToolColours::surfaceHover;
    else
        fill = ToolColours::surface;

    g.setColour (fill);
    g.fillRoundedRectangle (bounds, 4.0f);

    if (! active)
    {
        g.setColour (isMouseOver ? ToolColours::line.brighter (0.30f) : ToolColours::line);
        g.drawRoundedRectangle (bounds, 4.0f, 1.0f);
    }
}

void ToolLookAndFeel::drawButtonText (juce::Graphics& g, juce::TextButton& button,
                                      bool, bool)
{
    const auto active = button.getToggleState();

    juce::AttributedString label (button.getButtonText());
    label.setFont (uiFont (11.5f, true, 0.10f));
    label.setColour (active ? ToolColours::bg : ToolColours::textDim);
    label.setJustification (juce::Justification::centred);
    label.draw (g, button.getLocalBounds().toFloat());
}
