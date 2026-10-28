#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    constexpr auto gainId   = "gain";
    constexpr auto invertId = "invert";
    constexpr auto monoId   = "mono";
}

ToolAudioProcessor::ToolAudioProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput ("Input", juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
    gainParam   = apvts.getRawParameterValue (gainId);
    invertParam = apvts.getRawParameterValue (invertId);
    monoParam   = apvts.getRawParameterValue (monoId);
}

juce::AudioProcessorValueTreeState::ParameterLayout ToolAudioProcessor::createParameterLayout()
{
    using namespace juce;

    AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add (std::make_unique<AudioParameterFloat> (
        ParameterID { gainId, 1 }, "Gain",
        NormalisableRange<float> (-maxGainDb, maxGainDb, 0.01f), 0.0f,
        AudioParameterFloatAttributes().withLabel ("dB")));

    layout.add (std::make_unique<AudioParameterBool> (ParameterID { invertId, 1 }, "Invert Polarity", false));
    layout.add (std::make_unique<AudioParameterBool> (ParameterID { monoId, 1 }, "Mono", false));

    return layout;
}

bool ToolAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto in  = layouts.getMainInputChannelSet();
    const auto out = layouts.getMainOutputChannelSet();

    if (in != out)
        return false;

    return out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo();
}

void ToolAudioProcessor::prepareToPlay (double sampleRate, int)
{
    gainLevel.reset (sampleRate, 0.010);
    monoMix.reset (sampleRate, 0.020);

    gainLevel.setCurrentAndTargetValue (juce::Decibels::decibelsToGain (gainParam->load())
                                            * (invertParam->load() > 0.5f ? -1.0f : 1.0f));
    monoMix.setCurrentAndTargetValue (monoParam->load() > 0.5f ? 1.0f : 0.0f);
}

void ToolAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const auto numSamples = buffer.getNumSamples();
    const auto numChannels = buffer.getNumChannels();

    if (numChannels == 0 || numSamples == 0)
        return;

    for (int ch = 2; ch < numChannels; ++ch)
        buffer.clear (ch, 0, numSamples);

    const auto gain = juce::Decibels::decibelsToGain (gainParam->load());

    gainLevel.setTargetValue (gain * (invertParam->load() > 0.5f ? -1.0f : 1.0f));
    monoMix.setTargetValue (monoParam->load() > 0.5f ? 1.0f : 0.0f);

    auto* left = buffer.getWritePointer (0);
    auto* right = numChannels > 1 ? buffer.getWritePointer (1) : nullptr;

    // Ramping the signed gain and the mono blend keeps both switches click-free.
    for (int i = 0; i < numSamples; ++i)
    {
        const auto g = gainLevel.getNextValue();
        const auto mix = monoMix.getNextValue();

        auto l = left[i];
        auto r = right != nullptr ? right[i] : 0.0f;

        if (right != nullptr)
        {
            const auto mid = (l + r) * 0.5f;
            l += mix * (mid - l);
            r += mix * (mid - r);
        }

        left[i] = l * g;

        if (right != nullptr)
            right[i] = r * g;
    }
}

juce::AudioProcessorEditor* ToolAudioProcessor::createEditor()
{
    return new ToolAudioProcessorEditor (*this);
}

void ToolAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void ToolAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ToolAudioProcessor();
}
