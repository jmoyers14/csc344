/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic startup code for a Juce application.

  ==============================================================================
*/

#ifndef PLUGINPROCESSOR_H_INCLUDED
#define PLUGINPROCESSOR_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include <complex>
#include <math.h>

//==============================================================================
/**
*/
class JbcfilterAudioProcessor  : public AudioProcessor
{
public:
    //==============================================================================
    JbcfilterAudioProcessor();
    ~JbcfilterAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock);
    void releaseResources();

    void processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages);
    void reset() override;
    //==============================================================================
    AudioProcessorEditor* createEditor();
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const;

    int getNumParameters();

    float getParameter (int index);
    float getParameterDefaultValue (int index) override;
    void setParameter (int index, float newValue);

    const String getParameterName (int index);
    const String getParameterText (int index);

    const String getInputChannelName (int channelIndex) const;
    const String getOutputChannelName (int channelIndex) const;
    bool isInputChannelStereoPair (int index) const;
    bool isOutputChannelStereoPair (int index) const;

    bool acceptsMidi() const;
    bool producesMidi() const;
    bool silenceInProducesSilenceOut() const;
    double getTailLengthSeconds() const;

    //==============================================================================
    int getNumPrograms();
    int getCurrentProgram();
    void setCurrentProgram (int index);
    const String getProgramName (int index);
    void changeProgramName (int index, const String& newName);

    //==============================================================================
    void getStateInformation (MemoryBlock& destData);
    void setStateInformation (const void* data, int sizeInBytes);
    
    void updateCoefficients (float cutOff);
    
    
    
    AudioPlayHead::CurrentPositionInfo lastPosInfo;
    int lastUIWidth, lastUIHeight;
    
    std::complex<double> sPole_1 = std::complex<double>(-0.14113177417460146,0.9847055588695726);
    std::complex<double> sPole_2 = std::complex<double>(-0.3407222432940997,0.40787839742795523);
    std::complex<double> sPole_3 = std::complex<double>(-0.3407222432940997,-0.40787839742795506);
    std::complex<double> sPole_4 = std::complex<double>(-0.1411317741746015,-0.9847055588695726);
    
    
    enum Parameters
    {
        delayParam = 0,
        cutoffParam,
        
        totalNumParams
    };
    
    float delay;
    double angle, cutoff;
    int nyquist;

private:
    //==============================================================================
    AudioSampleBuffer delayBuffer, lowPassBuffer;
    int delayPosition;
    double freqSliderVal;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JbcfilterAudioProcessor)

};

#endif  // PLUGINPROCESSOR_H_INCLUDED
