#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
class VoltaAudioProcessorEditor:public juce::AudioProcessorEditor{
public:explicit VoltaAudioProcessorEditor(VoltaAudioProcessor&);void paint(juce::Graphics&)override;void resized()override;
private:VoltaAudioProcessor&p;juce::Slider mix,det,cut,res,a,d,s,r,tube,iron,voltage;using SA=juce::AudioProcessorValueTreeState::SliderAttachment;std::vector<std::unique_ptr<SA>> at;std::vector<juce::Slider*> knobs;JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VoltaAudioProcessorEditor)
};
