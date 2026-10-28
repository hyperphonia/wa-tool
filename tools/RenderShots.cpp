#include "PluginEditor.h"
#include "PluginProcessor.h"

namespace
{
    void setParam (juce::AudioProcessorValueTreeState& apvts, const char* id, float normalised)
    {
        if (auto* p = apvts.getParameter (id))
            p->setValueNotifyingHost (normalised);
    }

    void setGain (juce::AudioProcessorValueTreeState& apvts, float db)
    {
        if (auto* p = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter ("gain")))
            p->setValueNotifyingHost (p->convertTo0to1 (db));
    }

    void renderShot (const juce::File& dir, const juce::String& name,
                     float gainDb, bool invert, bool mono)
    {
        ToolAudioProcessor processor;

        setGain (processor.apvts, gainDb);
        setParam (processor.apvts, "invert", invert ? 1.0f : 0.0f);
        setParam (processor.apvts, "mono", mono ? 1.0f : 0.0f);

        std::unique_ptr<juce::AudioProcessorEditor> editor (processor.createEditor());
        editor->setVisible (true);

        const auto image = editor->createComponentSnapshot (editor->getLocalBounds(), true, 2.0f);
        const auto file = dir.getChildFile (name + ".png");
        file.deleteFile();

        if (auto stream = file.createOutputStream())
        {
            juce::PNGImageFormat png;
            png.writeImageToStream (image, *stream);
        }

        std::cout << file.getFullPathName() << std::endl;
    }
}

class ShotApp : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override    { return "ToolShots"; }
    const juce::String getApplicationVersion() override { return "1.0.0"; }
    bool moreThanOneInstanceAllowed() override          { return true; }

    void initialise (const juce::String& commandLine) override
    {
        auto dir = juce::File::getCurrentWorkingDirectory().getChildFile ("screenshots");
        dir.createDirectory();

        if (commandLine.isNotEmpty())
            dir = juce::File (commandLine.trim());

        renderShot (dir, "01-default",  0.0f,  false, false);
        renderShot (dir, "02-engaged",  6.0f,  true,  true);
        renderShot (dir, "03-trim",   -12.5f,  false, false);

        setApplicationReturnValue (0);
        quit();
    }

    void shutdown() override {}
    void systemRequestedQuit() override { quit(); }
};

START_JUCE_APPLICATION (ShotApp)
