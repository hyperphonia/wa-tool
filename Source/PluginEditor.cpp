#include "PluginEditor.h"
#include "BinaryData.h"

#include <cstdlib>

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

void ToolAudioProcessorEditor::GainKnob::mouseDown (const juce::MouseEvent& e)
{
    contextMenuClick = e.mods.isPopupMenu();

    if (contextMenuClick)
    {
        if (onContextMenu != nullptr)
            onContextMenu();

        return;
    }

    juce::Slider::mouseDown (e);
}

void ToolAudioProcessorEditor::GainKnob::mouseUp (const juce::MouseEvent& e)
{
    // The drag state was never armed for a context click, so the base class must not see it.
    if (std::exchange (contextMenuClick, false))
        return;

    juce::Slider::mouseUp (e);
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
    gainKnob.onContextMenu = [this] { showGainMenu(); };
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

void ToolAudioProcessorEditor::showGainMenu()
{
    juce::PopupMenu menu;
    menu.addItem (1, "Set value...");
    menu.addSeparator();
    menu.addItem (2, "Reset to 0 dB");

    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (gainKnob).withMousePosition(),
                        [safe = juce::Component::SafePointer<ToolAudioProcessorEditor> (this)] (int result)
                        {
                            if (safe == nullptr)
                                return;

                            if (result == 1)
                                safe->promptForGain();
                            else if (result == 2)
                                safe->gainKnob.setValue (0.0, juce::sendNotificationSync);
                        });
}

void ToolAudioProcessorEditor::promptForGain()
{
    auto* window = new juce::AlertWindow ("Gain", "Enter a value in dB", juce::MessageBoxIconType::NoIcon, this);

    window->addTextEditor ("value", juce::String (gainKnob.getValue(), 1));
    window->addButton ("OK", 1, juce::KeyPress (juce::KeyPress::returnKey));
    window->addButton ("Cancel", 0, juce::KeyPress (juce::KeyPress::escapeKey));

    window->enterModalState (true, juce::ModalCallbackFunction::create (
        [editor = juce::Component::SafePointer<ToolAudioProcessorEditor> (this),
         alert = juce::Component::SafePointer<juce::AlertWindow> (window)] (int result)
        {
            if (editor == nullptr || alert == nullptr || result != 1)
                return;

            editor->setGainFromText (alert->getTextEditorContents ("value"));
        }), true);
}

void ToolAudioProcessorEditor::setGainFromText (const juce::String& text)
{
    auto trimmed = text.trim();

    if (trimmed.endsWithIgnoreCase ("db"))
        trimmed = trimmed.dropLastCharacters (2).trim();

    const auto utf8 = trimmed.toStdString();
    char* end = nullptr;
    const auto parsed = std::strtof (utf8.c_str(), &end);

    if (end == utf8.c_str() || *end != '\0')
        return;

    gainKnob.setValue (parsed, juce::sendNotificationSync);
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
