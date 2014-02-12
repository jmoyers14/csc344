/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic startup code for a Juce application.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <complex>
#include <math.h>

AudioProcessor* JUCE_CALLTYPE createPluginFilter();
const float dGain = 1.0f;
const float dDelay = 0.0f;
const float dCutoffFreq = 1.0f;
const std::complex<double> imag = std::sqrt(std::complex<double>(-1));
std::complex<double> sPoles[4];
std::complex<double> zPoles[4];
std::complex<double> coefficients[4];



# define BUFFER_LEN 12000
//==============================================================================
JbcfilterAudioProcessor::JbcfilterAudioProcessor()
: delayBuffer(2,BUFFER_LEN), lowPassBuffer(2, 4)
{
    freqSliderVal = 800.0;
    // Set up some default values..
    gain   = dGain;
    delay  = dDelay;
    cutoff = dCutoffFreq;
    
    lastUIWidth = 400;
    lastUIHeight = 200;
    
    lastPosInfo.resetToDefault();
    delayPosition = 0;
    
    //compute chebyshev poles
    for (int i=0; i<4; i++) {
        sPoles[i] = imag * cos((1.0 / 4.0) * acos((imag/0.5)) + (((i) * double_Pi) / 4));
    }
}

JbcfilterAudioProcessor::~JbcfilterAudioProcessor()
{
}

//==============================================================================
const String JbcfilterAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

int JbcfilterAudioProcessor::getNumParameters()
{
    return totalNumParams;
}

float JbcfilterAudioProcessor::getParameter (int index)
{
    
    switch(index) {
        case freqParam:
            return freqSliderVal;
        case gainParam:
            return gain;
        case delayParam:
            return delay;
        case cutoffParam:
            return cutoff;
        default:
            return 0.0f;
    }
}

void JbcfilterAudioProcessor::setParameter (int index, float newValue)
{
    switch (index) {
        case cutoffParam:
            cutoff = newValue;
            break;
        case delayParam:
            delay = newValue;
            break;
        case gainParam:
            gain = newValue;
            break;
        default:
            break;
    }
}

float JbcfilterAudioProcessor::getParameterDefaultValue (int index)
{
    switch (index)
    {
        case gainParam: return dGain;
        case delayParam: return dDelay;
        case cutoffParam: return dCutoffFreq;
        default: break;
    }
    
    return 0.0f;
}

const String JbcfilterAudioProcessor::getParameterName (int index)
{  
    switch(index){
        case freqParam:
            return "freqParam";
        case delayParam:
            return "delay";
        case gainParam:
            return "gain";
        case cutoffParam:
            return "cutoff";
        default:
            return String::empty;
    }
}

const String JbcfilterAudioProcessor::getParameterText (int index)
{
 return String (getParameter (index), 2);
}

const String JbcfilterAudioProcessor::getInputChannelName (int channelIndex) const
{
    return String (channelIndex + 1);
}

const String JbcfilterAudioProcessor::getOutputChannelName (int channelIndex) const
{
    return String (channelIndex + 1);
}

bool JbcfilterAudioProcessor::isInputChannelStereoPair (int index) const
{
    return true;
}

bool JbcfilterAudioProcessor::isOutputChannelStereoPair (int index) const
{
    return true;
}

bool JbcfilterAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool JbcfilterAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool JbcfilterAudioProcessor::silenceInProducesSilenceOut() const
{
    return false;
}

double JbcfilterAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int JbcfilterAudioProcessor::getNumPrograms()
{
    return 0;
}

int JbcfilterAudioProcessor::getCurrentProgram()
{
    return 0;
}

void JbcfilterAudioProcessor::setCurrentProgram (int index)
{
}

const String JbcfilterAudioProcessor::getProgramName (int index)
{
    return String::empty;
}

void JbcfilterAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void JbcfilterAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    delayBuffer.clear();
    lowPassBuffer.clear();
}

void JbcfilterAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

void JbcfilterAudioProcessor::reset()
{
    // Use this method as the place to clear any delay lines, buffers, etc, as it
    // means there's been a break in the audio's continuity.
    delayBuffer.clear();
    lowPassBuffer.clear();
}


void JbcfilterAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    const int samples = buffer.getNumSamples();
    const int delayBufferSamples = delayBuffer.getNumSamples();

    int dp = delayPosition;
    
    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    for (int channel = 0; channel < getNumInputChannels(); ++channel)
    {
        float* channelData = buffer.getSampleData (channel);
        float* delayData = delayBuffer.getSampleData (channel);
        dp = delayPosition;
        
        for (int i = 0; i < samples; ++i)
        {
            const float in = channelData[i];
            channelData[i] += delayData[dp];
            delayData[dp] = (delayData[dp] + in) * delay;
            dp += 1;
            if (dp >= delayBufferSamples)
                dp = 0;
        }
    }

    delayPosition = dp;
    
    // In case we have more outputs than inputs, we'll clear any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    for (int i = getNumInputChannels(); i < getNumOutputChannels(); ++i)
    {
        buffer.clear (i, 0, buffer.getNumSamples());
    }
}

//==============================================================================
bool JbcfilterAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* JbcfilterAudioProcessor::createEditor()
{
    return new JbcfilterAudioProcessorEditor (this);
}

//==============================================================================
void JbcfilterAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // Here's an example of how you can use XML to make it easy and more robust:
    
    // Create an outer XML element..
    XmlElement xml ("MYPLUGINSETTINGS");
    
    // add some attributes to it..
    xml.setAttribute ("uiWidth", lastUIWidth);
    xml.setAttribute ("uiHeight", lastUIHeight);
    xml.setAttribute ("gain", gain);
    xml.setAttribute ("delay", delay);
    
    // then use this helper function to stuff it into the binary blob and return it..
    copyXmlToBinary (xml, destData);
}

void JbcfilterAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    
    // This getXmlFromBinary() helper function retrieves our XML from the binary blob..
    ScopedPointer<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    
    if (xmlState != nullptr)
    {
        // make sure that it's actually our type of XML object..
        if (xmlState->hasTagName ("MYPLUGINSETTINGS"))
        {
            // ok, now pull out our parameters..
            lastUIWidth = xmlState->getIntAttribute ("uiWidth", lastUIWidth);
            lastUIHeight = xmlState->getIntAttribute ("uiHeight", lastUIHeight);
            
            gain = (float) xmlState->getDoubleAttribute ("gain", gain);
            delay = (float) xmlState->getDoubleAttribute ("delay", delay);
        }
    }
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new JbcfilterAudioProcessor();
}
