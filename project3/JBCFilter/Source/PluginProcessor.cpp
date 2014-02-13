/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic startup code for a Juce application.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


AudioProcessor* JUCE_CALLTYPE createPluginFilter();
const float dGain = 1.0f;
const float dDelay = 0.0f;
const float dCutoffFreq = 0.0f;
const bool distortionEnabledFlag = false;
const std::complex<double> imag = std::sqrt(std::complex<double>(-1));
std::complex<double> z1, z2, z3, z4;
std::complex<double> cB, cC, cD, cE;



# define BUFFER_LEN 12000
//==============================================================================
JbcfilterAudioProcessor::JbcfilterAudioProcessor()
: delayBuffer(2,BUFFER_LEN), lowPassBuffer(2, 4)
{
    freqSliderVal = 800.0;
    // Set up some default values..
    //gain   = dGain;
    delay  = dDelay;
    cutoff = dCutoffFreq;
    distortion = 1.0;
    lastUIWidth = 400;
    lastUIHeight = 200;
    
    lastPosInfo.resetToDefault();
    delayPosition = 0;
    nyquist = (getSampleRate() / 2);
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
        //case gainParam:
          //  return gain;
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
            printf("new cutoff %f\n", cutoff);
            updateCoefficients(cutoff);
            break;
        case delayParam:
            delay = newValue;
            printf("new delay %f\n", delay);
            break;
        //case gainParam:
          //  gain = newValue;
            break;
        case distortionEnabledParam:
            distortionEnabledFlag = newValue > 0.5f;
            break;
        case distortionParam:
            distortion = newValue;
            break;
        default:
            break;
    }
}

float JbcfilterAudioProcessor::getParameterDefaultValue (int index)
{
    switch (index)
    {
        //case gainParam: return dGain;
        case delayParam: return dDelay;
        case cutoffParam: return dCutoffFreq;
        default: break;
    }
    
    return 0.0f;
}

//convert given pole value form s space to z space
std::complex<double> JbcfilterAudioProcessor::computeZpole(double theta, std::complex<double> sPole)
{
    return (1.0 + ((sPole * theta) / 2.0)) / (1.0 - ((sPole * theta) / 2.0));
}

//compute new coefficient values for the new cutoff frequency
void JbcfilterAudioProcessor::updateCoefficients (float cutOff)
{
    angle = (((cutOff * nyquist) / nyquist) * double_Pi);
    
    z1 = computeZpole(angle, sPole_1);
    z2 = computeZpole(angle, sPole_2);
    z3 = computeZpole(angle, sPole_3);
    z4 = computeZpole(angle, sPole_4);
    std::cout << "z1 = " << cB << std::endl;
    std::cout << "z2 = " << cC << std::endl;
    std::cout << "z3 = " << cD << std::endl;
    std::cout << "z4 = " << cE << std::endl;
    //hard coding equation from class...
    cB = z1 + z3 + z3 + z4;
    
    cC = (z1 * z2) +
         (z1 * z3) +
         (z1 * z4) +
         (z2 * z3) +
         (z2 * z4) +
         (z3 * z4);
    
    cD = (z1 * z2 * z3) +
         (z1 * z2 * z4) +
         (z1 * z3 * z4) +
         (z2 * z1 * z4);
    
    cE = (z1 * z2 * z3 * z4);
    std::cout << "coefB = " << cB << std::endl;
    std::cout << "coefC = " << cC << std::endl;
    std::cout << "coefD = " << cD << std::endl;
    std::cout << "coefE = " << cE << std::endl;
}

const String JbcfilterAudioProcessor::getParameterName (int index)
{  
    switch(index){
        case delayParam:
            return "delay";
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
            
            if(distortionEnabledFlag) {
                channelData[i] = channelData[i] * distortion;
                
                if(channelData[i] > 0.03) {
                    channelData[i]  = 0.03;
                }
            }
            
            /*
            channelData[i] -= (delayData[dp] +
                              (std::real(cB) * delayData[dp - 1]) +
                              (std::real(cC) * delayData[dp - 2]) +
                              (std::real(cD) * delayData[dp - 3]) +
                              (std::real(cE) * delayData[dp - 4])) * .0001;
            
            */
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
    //xml.setAttribute ("gain", gain);
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
            
            //gain = (float) xmlState->getDoubleAttribute ("gain", gain);
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
