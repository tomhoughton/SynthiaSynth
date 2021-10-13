/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
BasicOscAudioProcessor::BasicOscAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

BasicOscAudioProcessor::~BasicOscAudioProcessor()
{
}

//==============================================================================
const juce::String BasicOscAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool BasicOscAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool BasicOscAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool BasicOscAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double BasicOscAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int BasicOscAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int BasicOscAudioProcessor::getCurrentProgram()
{
    return 0;
}

void BasicOscAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String BasicOscAudioProcessor::getProgramName (int index)
{
    return {};
}

void BasicOscAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void BasicOscAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    
    // In any DSP algorithm we need to give the sample size buffer size and nubmer of channels trying to process:
    juce::dsp::ProcessSpec spec; // Contains various aspects of the context in which it can expect to be called.
    spec.maximumBlockSize = samplesPerBlock; // How big the buffer size is.
    spec.sampleRate = sampleRate; // The sample rate.
    spec.numChannels = getTotalNumOutputChannels();
    
    // Pass this into the osc:
    osc.prepare(spec); // Pass a ref to the spec object.
    
    // Pass this into the second osc:
    osc2.prepare(spec);
    
    // Prepare the gain since its also a dsp.
    gain.prepare(spec);
    
    
}

void BasicOscAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool BasicOscAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void BasicOscAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    
    // Tutorial took this block out:
//    for (int channel = 0; channel < totalNumInputChannels; ++channel)
//    {
//        auto* channelData = buffer.getWritePointer (channel);
//
//        // ..do something to the data...
//    }
    
    // Create audio block:
    juce::dsp::AudioBlock<float> audioBlock { buffer }; // This just points towards the buffer.
    osc.process (juce::dsp::ProcessContextReplacing<float> (audioBlock));
    osc2.process (juce::dsp::ProcessContextReplacing<float> (audioBlock));
    gain.process (juce::dsp::ProcessContextReplacing<float> (audioBlock));
    
    // Set the frequency of the osc:
    osc.setFrequency(155.563f);
    osc2.setFrequency(233.082f);
    
    // Set the gain:
    gain.setGainLinear(0.1f);
}

//==============================================================================
bool BasicOscAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* BasicOscAudioProcessor::createEditor()
{
    return new BasicOscAudioProcessorEditor (*this);
}

//==============================================================================
void BasicOscAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void BasicOscAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BasicOscAudioProcessor();
}
