#include "PluginProcessor.h"

namespace
{
    int failures = 0;

    void check (const char* name, float actual, float expected, float tolerance = 1.0e-4f)
    {
        const auto ok = std::abs (actual - expected) <= tolerance;

        if (! ok)
            ++failures;

        std::cout << (ok ? "PASS  " : "FAIL  ") << name
                  << "  got " << actual << "  expected " << expected << std::endl;
    }

    void setParam (juce::AudioProcessorValueTreeState& apvts, const char* id, float normalised)
    {
        if (auto* p = apvts.getParameter (id))
            p->setValueNotifyingHost (normalised);
    }

    float gainToNormalised (juce::AudioProcessorValueTreeState& apvts, float db)
    {
        if (auto* p = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter ("gain")))
            return p->convertTo0to1 (db);

        return 0.0f;
    }

    juce::AudioBuffer<float> run (float db, bool invert, bool mono,
                                  float leftIn, float rightIn, int numSamples = 4096)
    {
        ToolAudioProcessor processor;

        setParam (processor.apvts, "gain", gainToNormalised (processor.apvts, db));
        setParam (processor.apvts, "invert", invert ? 1.0f : 0.0f);
        setParam (processor.apvts, "mono", mono ? 1.0f : 0.0f);

        processor.setPlayConfigDetails (2, 2, 48000.0, 512);
        processor.prepareToPlay (48000.0, 512);

        juce::AudioBuffer<float> buffer (2, numSamples);
        buffer.clear();

        for (int i = 0; i < numSamples; ++i)
        {
            buffer.setSample (0, i, leftIn);
            buffer.setSample (1, i, rightIn);
        }

        juce::MidiBuffer midi;
        processor.processBlock (buffer, midi);

        return buffer;
    }
}

int main()
{
    juce::ScopedJuceInitialiser_GUI juceInit;

    {
        const auto out = run (0.0f, false, false, 1.0f, -0.5f);
        check ("unity pass-through L", out.getSample (0, 4095), 1.0f);
        check ("unity pass-through R", out.getSample (1, 4095), -0.5f);
    }

    {
        const auto out = run (6.0206f, false, false, 1.0f, 1.0f);
        check ("+6 dB gain", out.getSample (0, 4095), 2.0f, 1.0e-3f);
    }

    {
        const auto out = run (-6.0206f, false, false, 1.0f, 1.0f);
        check ("-6 dB gain", out.getSample (0, 4095), 0.5f, 1.0e-3f);
    }

    {
        const auto out = run (0.0f, true, false, 1.0f, -0.5f);
        check ("invert L", out.getSample (0, 4095), -1.0f);
        check ("invert R", out.getSample (1, 4095), 0.5f);
    }

    {
        const auto out = run (0.0f, false, true, 1.0f, 0.5f);
        check ("mono averages L", out.getSample (0, 4095), 0.75f);
        check ("mono averages R", out.getSample (1, 4095), 0.75f);
    }

    {
        const auto out = run (0.0f, false, true, 1.0f, -1.0f);
        check ("mono cancels out-of-phase", out.getSample (0, 4095), 0.0f);
    }

    {
        const auto out = run (6.0206f, true, true, 0.5f, 0.5f);
        check ("combined: mono, gain, invert L", out.getSample (0, 4095), -1.0f, 1.0e-3f);
        check ("combined: mono, gain, invert R", out.getSample (1, 4095), -1.0f, 1.0e-3f);
    }

    {
        ToolAudioProcessor processor;
        setParam (processor.apvts, "gain", gainToNormalised (processor.apvts, -12.0f));
        setParam (processor.apvts, "mono", 1.0f);

        processor.setPlayConfigDetails (2, 2, 48000.0, 512);
        processor.prepareToPlay (48000.0, 512);

        juce::MemoryBlock state;
        processor.getStateInformation (state);

        ToolAudioProcessor restored;
        restored.setStateInformation (state.getData(), (int) state.getSize());
        restored.setPlayConfigDetails (2, 2, 48000.0, 512);
        restored.prepareToPlay (48000.0, 512);

        juce::AudioBuffer<float> buffer (2, 256);
        for (int ch = 0; ch < 2; ++ch)
            for (int i = 0; i < 256; ++i)
                buffer.setSample (ch, i, 0.25f);

        juce::MidiBuffer midi;
        restored.processBlock (buffer, midi);

        check ("state round-trip keeps gain", restored.apvts.getRawParameterValue ("gain")->load(), -12.0f);
        check ("state round-trip keeps mono", restored.apvts.getRawParameterValue ("mono")->load(), 1.0f);
        check ("state round-trip output", buffer.getSample (0, 255), 0.0625f, 1.0e-3f);
    }

    std::cout << (failures == 0 ? "\nAll checks passed." : "\nFailures: " + std::to_string (failures))
              << std::endl;

    return failures == 0 ? 0 : 1;
}
