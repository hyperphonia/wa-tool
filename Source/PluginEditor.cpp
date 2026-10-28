#include "PluginEditor.h"
#include "BinaryData.h"

namespace
{
    constexpr int editorWidth  = 280;
    constexpr int editorHeight = 228;
    constexpr int margin       = 14;

    constexpr int logoHeight   = 26;
    constexpr int logoTop      = 14;
    constexpr int knobSize     = 96;
    constexpr int buttonHeight = 30;
    constexpr int buttonGap    = 8;

    constexpr float logoOpacity = 0.72f;

    juce::Image trimTransparentEdges (const juce::Image& source)
    {
        const juce::Image::BitmapData pixels (source, juce::Image::BitmapData::readOnly);

        int minX = source.getWidth(), minY = source.getHeight(), maxX = -1, maxY = -1;

        for (int y = 0; y < source.getHeight(); ++y)
        {
            for (int x = 0; x < source.getWidth(); ++x)
            {
                if (pixels.getPixelColour (x, y).getAlpha() > 8)
                {
                    minX = juce::jmin (minX, x);
                    minY = juce::jmin (minY, y);
                    maxX = juce::jmax (maxX, x);
                    maxY = juce::jmax (maxY, y);
                }
            }
        }

        if (maxX < minX || maxY < minY)
            return source;

        return source.getClippedImage ({ minX, minY, maxX - minX + 1, maxY - minY + 1 });
    }

    juce::Image downscale (juce::Image image, int targetWidth, int targetHeight)
    {
        targetWidth = juce::jmax (1, targetWidth);
        targetHeight = juce::jmax (1, targetHeight);

        while (image.getWidth() >= targetWidth * 2 && image.getHeight() >= targetHeight * 2)
            image = image.rescaled (image.getWidth() / 2, image.getHeight() / 2,
                                    juce::Graphics::highResamplingQuality);

        return image.rescaled (targetWidth, targetHeight, juce::Graphics::highResamplingQuality);
    }
}

void ToolAudioProcessorEditor::ValueReadout::setValue (float newValue)
{
    if (! juce::approximatelyEqual (value, newValue))
    {
        value = newValue;
        repaint();
    }
}

void ToolAudioProcessorEditor::ValueReadout::paint (juce::Graphics& g)
{
    const auto number = std::abs (value) < 0.05f
                            ? juce::String ("0.0")
                            : juce::String::formatted ("%+.1f", value);

    juce::AttributedString readout;
    readout.append (number, ToolLookAndFeel::valueFont (14.0f), ToolColours::text);
    readout.append (" dB", ToolLookAndFeel::uiFont (9.5f, true, 0.10f), ToolColours::textDim);
    readout.setJustification (juce::Justification::centred);
    readout.draw (g, getLocalBounds().toFloat());
}

ToolAudioProcessorEditor::ToolAudioProcessorEditor (ToolAudioProcessor& processor)
    : AudioProcessorEditor (&processor),
      processorRef (processor),
      logo (trimTransparentEdges (juce::ImageCache::getFromMemory (BinaryData::WA_logo_transparent_png,
                                                                  BinaryData::WA_logo_transparent_pngSize))),
      invertButton ("INVERT"),
      monoButton ("MONO"),
      gainAttachment (processorRef.apvts, "gain", gainKnob),
      invertAttachment (processorRef.apvts, "invert", invertButton),
      monoAttachment (processorRef.apvts, "mono", monoButton)
{
    setLookAndFeel (&lnf);

    gainKnob.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    gainKnob.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    gainKnob.setMouseDragSensitivity (240);
    gainKnob.setDoubleClickReturnValue (true, 0.0);
    gainKnob.setTitle ("Gain");
    addAndMakeVisible (gainKnob);

    gainKnob.onValueChange = [this]
    {
        readout.setValue (static_cast<float> (gainKnob.getValue()));
    };

    readout.setValue (static_cast<float> (gainKnob.getValue()));
    readout.setInterceptsMouseClicks (false, false);
    addAndMakeVisible (readout);

    for (auto* button : { &invertButton, &monoButton })
    {
        button->setClickingTogglesState (true);
        addAndMakeVisible (*button);
    }

    invertButton.setTitle ("Invert polarity");
    monoButton.setTitle ("Fold to mono");

    setSize (editorWidth, editorHeight);
}

ToolAudioProcessorEditor::~ToolAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

juce::Rectangle<float> ToolAudioProcessorEditor::logoBounds() const
{
    if (logo.isNull())
        return {};

    const auto width = logoHeight * (float) logo.getWidth() / (float) logo.getHeight();

    return juce::Rectangle<float> (width, (float) logoHeight)
        .withCentre ({ editorWidth * 0.5f, logoTop + logoHeight * 0.5f });
}

void ToolAudioProcessorEditor::refreshLogo (float scale)
{
    const auto bounds = logoBounds() * scale;

    scaledLogo = downscale (logo, juce::roundToInt (bounds.getWidth()),
                            juce::roundToInt (bounds.getHeight()));
    logoScale = scale;
}

void ToolAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (ToolColours::bg);

    if (logo.isNull())
        return;

    const auto scale = g.getInternalContext().getPhysicalPixelScaleFactor();

    if (! juce::approximatelyEqual (scale, logoScale) || scaledLogo.isNull())
        refreshLogo (scale);

    g.setImageResamplingQuality (juce::Graphics::highResamplingQuality);
    g.setOpacity (logoOpacity);
    g.drawImage (scaledLogo, logoBounds());
    g.setOpacity (1.0f);
}

void ToolAudioProcessorEditor::resized()
{
    const auto contentWidth = editorWidth - margin * 2;
    const auto buttonWidth = (contentWidth - buttonGap) / 2;
    const auto buttonY = editorHeight - margin - buttonHeight;

    gainKnob.setBounds ((editorWidth - knobSize) / 2, 56, knobSize, knobSize);
    readout.setBounds (margin, 152, contentWidth, 18);

    invertButton.setBounds (margin, buttonY, buttonWidth, buttonHeight);
    monoButton.setBounds (margin + buttonWidth + buttonGap, buttonY, buttonWidth, buttonHeight);
}
