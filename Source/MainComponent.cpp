/*
 The application generates an audio according to the control box for frequency and cut-off frequency, central frequency, or amplitude adjustments, and draw the waveform of the generated audio on the screen in real time.
 
 Used JUCE official website as resources, including https://docs.juce.com/master/tutorial_sine_synth.html
 */

#include "MainComponent.h"
#include <JuceHeader.h>

MainComponent::MainComponent() : waveformButton("Waveform"), filterButton("Filter"), sineButton("Sine Wave"), pulseButton("Train of Pulses"), lowPassButton("Low Pass Filter") , highPassButton("High Pass Filter"), bandPassButton("Band Pass Filter"), noneButton("None"), waveformVisible(false), filterVisible(false), currentWaveformType(SineWave), activeFilter(None)
{
    setSize (800, 600);
    setAudioChannels(0, 2);
    waveBuffer.setSize(1, 512);
    waveBuffer.clear();
    
    addAndMakeVisible(xAxisLabel);
    addAndMakeVisible(yAxisLabel);

    xAxisLabel.setText("Current amplitude (x-axis): ", juce::dontSendNotification);
    yAxisLabel.setText("Current frequency (y-axis): ", juce::dontSendNotification);

    xAxisLabel.setJustificationType(juce::Justification::centred);
    yAxisLabel.setJustificationType(juce::Justification::centred);
    
    addAndMakeVisible(waveformButton);
    addAndMakeVisible(filterButton);
    
    waveformButton.onClick = [this]() {
        waveformVisible = !waveformVisible;
        sineButton.setVisible(waveformVisible);
        pulseButton.setVisible(waveformVisible);
        updateAxisLabels();
        resized();
    };

    filterButton.onClick = [this]() {
        filterVisible = !filterVisible; 
        noneButton.setVisible(filterVisible);
        lowPassButton.setVisible(filterVisible);
        highPassButton.setVisible(filterVisible);
        bandPassButton.setVisible(filterVisible);
        updateAxisLabels();
        resized();
    };

    addAndMakeVisible(sineButton);
    addAndMakeVisible(pulseButton);
    addAndMakeVisible(noneButton);
    addAndMakeVisible(lowPassButton);
    addAndMakeVisible(highPassButton);
    addAndMakeVisible(bandPassButton);

    sineButton.setVisible(false);
    pulseButton.setVisible(false);
    noneButton.setVisible(false);
    lowPassButton.setVisible(false);
    highPassButton.setVisible(false);
    bandPassButton.setVisible(false);

    sineButton.onClick = [this]() { currentWaveformType = SineWave; updateAxisLabels();};
    pulseButton.onClick = [this]() { currentWaveformType = TrainOfPulses; updateAxisLabels();};
    
    noneButton.onClick = [this]() { activeFilter = None; updateAxisLabels(); lowPassFilter.reset(); highPassFilter.reset(); bandPassFilter.reset();};
    lowPassButton.onClick = [this]() { activeFilter = LowPass; updateAxisLabels(); currentAmplitude = 1.0f;};
    highPassButton.onClick = [this]() { activeFilter = HighPass; updateAxisLabels(); currentAmplitude = 1.0f;};
    bandPassButton.onClick = [this]() { activeFilter = BandPass; updateAxisLabels(); currentAmplitude = 1.0f;};
    
    resized();
}

MainComponent::~MainComponent()
{
    shutdownAudio();
}

void MainComponent::paint (juce::Graphics& g)
{
    // Drawing control box and the background
    g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::darkgrey);
    g.fillRect(controlBox);
    
    // Drawing the waveform of the generated sound from left to right
    g.setColour(juce::Colours::yellow);
    juce::Path wavePath;
     
    auto width = getWidth();
    auto height = getHeight();
    auto startX = width/3.0f;
    auto midY = height/2.0f;
    
    float scaleX = (float)(width - startX) / (float)waveBuffer.getNumSamples();
    
    if(AudioOn){
        wavePath.startNewSubPath(startX, midY - waveBuffer.getSample(0,0) * (midY/2.0f));
        
        for (int i = 1; i < waveBuffer.getNumSamples(); i++)
        {
            float x = startX + i * scaleX;
            float y = midY - waveBuffer.getSample(0, i) * (midY/2.0f);
            wavePath.lineTo(x, y);
        }
        
        g.strokePath(wavePath, juce::PathStrokeType(2.0f));
    }
}


void MainComponent::resized()
{
    int controlBoxWidth = getWidth() / 3;
    controlBox = juce::Rectangle<int>(0, 0, controlBoxWidth, getHeight());

    waveformButton.setBounds(10, 10, 120, 30);
    filterButton.setBounds(140, 10, 120, 30);

    int yOffset = 50;
    
    if (waveformVisible) {
        sineButton.setBounds(10, yOffset, 120, 30);
        pulseButton.setBounds(10, yOffset + 40, 120, 30);
    }

    if (filterVisible) {
        noneButton.setBounds(140, yOffset, 120, 30);
        lowPassButton.setBounds(140, yOffset + 40, 120, 30);
        highPassButton.setBounds(140, yOffset + 80, 120, 30);
        bandPassButton.setBounds(140, yOffset + 120, 120, 30);
    }
    
    xAxisLabel.setBounds(getWidth() - 250, getHeight() - 60, 240, 30);
    yAxisLabel.setBounds(getWidth() - 250, getHeight() - 30, 240, 30);
}

void MainComponent::updateAxisLabels()
{
    juce::String cutOffString = juce::String(currentCutOff, 2); // 2 decimal places
    juce::String frequencyString = juce::String(currentFrequency, 2); // 2 decimal places
    
    if (activeFilter == LowPass) {
        xAxisLabel.setText("Current cut-off frequency (x-axis): " + juce::String(currentCutOff), juce::dontSendNotification);
        yAxisLabel.setText("Current frequency (y-axis): " + juce::String(currentFrequency), juce::dontSendNotification);
    } else if (activeFilter == HighPass) {
        xAxisLabel.setText("Current cut-off frequency (x-axis): " + juce::String(currentCutOff), juce::dontSendNotification);
        yAxisLabel.setText("Current frequency (y-axis): " + juce::String(currentFrequency), juce::dontSendNotification);
    } else if (activeFilter == BandPass) {
        xAxisLabel.setText("Central frequency (x-axis): " + juce::String(centralFrequency), juce::dontSendNotification);
        yAxisLabel.setText("Current frequency (y-axis): " + juce::String(currentFrequency), juce::dontSendNotification);
    } else {
        // For general sine wave and pulse train
        xAxisLabel.setText("Current amplitude (x-axis): " + juce::String(currentAmplitude), juce::dontSendNotification);
        yAxisLabel.setText("Current frequency (y-axis): " + juce::String(currentFrequency), juce::dontSendNotification);
    }
}


void MainComponent::mouseDown(const juce::MouseEvent& event)
{
    if (controlBox.contains(event.getPosition()))
    {
        AudioOn = true;
        updateSignal(event.getPosition());
    }
}

void MainComponent::mouseDrag(const juce::MouseEvent& event)
{
    if (controlBox.contains(event.getPosition()))
    {
        updateSignal(event.getPosition());
    }
}

void MainComponent::mouseUp(const juce::MouseEvent& event){
    AudioOn = false;
}

void MainComponent::updateSignal(const juce::Point<int>& position)
{
    
    currentFrequency = juce::jmap(static_cast<float>(position.y), 0.0f, static_cast<float>(getHeight()), 5000.0f, 20.0f);
    
    if(activeFilter == LowPass){
        currentAmplitude = 1.0f; 
        currentCutOff = juce::jmap(static_cast<float>(position.x), 0.0f, static_cast<float>(controlBox.getWidth()), 20.0f, 3000.0f);
        lowPassFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(currentSampleRate, currentCutOff);
    }else if(activeFilter == HighPass){
        currentAmplitude = 1.0f;
        currentCutOff = juce::jmap(static_cast<float>(position.x), 0.0f, static_cast<float>(controlBox.getWidth()), 20.0f, 3000.0f);
        highPassFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(currentSampleRate, currentCutOff);
    }else if(activeFilter == BandPass){
        currentAmplitude = 1.0f;
        centralFrequency = juce::jmap(static_cast<float>(position.x), 0.0f, static_cast<float>(controlBox.getWidth()), 200.0f, 3000.0f);
        float bandwidth = centralFrequency/5.0f;
        bandPassFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeBandPass(currentSampleRate, centralFrequency, bandwidth);
    }else{
        currentAmplitude = juce::jmap(static_cast<float>(position.x), 0.0f, static_cast<float>(controlBox.getWidth()), 0.0f, 1.0f);
    }
    
    updateAngleDelta();
    updateAxisLabels();
}

void MainComponent::updateAngleDelta()
{
    if (currentSampleRate > 0.0)
    {
        auto cyclesPerSample = currentFrequency / currentSampleRate;
        angleDelta = cyclesPerSample * 2.0f * juce::MathConstants<double>::pi;
    }
}

void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    currentSampleRate = sampleRate;
    updateAngleDelta();
    currentAngle = 0.0;
    
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlockExpected;
    spec.numChannels = 2;
    
    lowPassFilter.prepare(spec);
    highPassFilter.prepare(spec);
    bandPassFilter.prepare(spec);
    
    lowPassFilter.reset();
    highPassFilter.reset();
    bandPassFilter.reset();
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    if (AudioOn) {
        if (currentWaveformType == SineWave){
            generateSineWave(bufferToFill);
        } else if (currentWaveformType == TrainOfPulses) {
            generateTrainOfPulses(bufferToFill);
        }
    } else {
        bufferToFill.clearActiveBufferRegion();
    }
}


void MainComponent::generateSineWave(const juce::AudioSourceChannelInfo& bufferToFill)
{
    if (AudioOn)
    {
        auto* leftBuffer = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);
        auto* rightBuffer = bufferToFill.buffer->getWritePointer(1, bufferToFill.startSample);

        for (int sample = 0; sample < bufferToFill.numSamples; sample++)
        {
            auto currentSample = (float)std::sin(currentAngle) * currentAmplitude;
            currentAngle += angleDelta;
            
            if (activeFilter == LowPass && lowPassFilter.coefficients != nullptr) {
                currentSample = lowPassFilter.processSample(currentSample);
            } else if (activeFilter == HighPass && highPassFilter.coefficients != nullptr) {
                currentSample = highPassFilter.processSample(currentSample);
            } else if (activeFilter == BandPass && bandPassFilter.coefficients != nullptr) {
                currentSample = bandPassFilter.processSample(currentSample);
            }

            leftBuffer[sample] = currentSample;
            rightBuffer[sample] = currentSample;

            waveBuffer.setSample(0, sample % waveBuffer.getNumSamples(), currentSample);
        }
        triggerAsyncUpdate();
    }
    else
    {
        bufferToFill.clearActiveBufferRegion();
    }
}

void MainComponent::generateTrainOfPulses(const juce::AudioSourceChannelInfo& bufferToFill)
{
      if (AudioOn)
      {
          auto* leftBuffer = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);
          auto* rightBuffer = bufferToFill.buffer->getWritePointer(1, bufferToFill.startSample);
          
          float alpha = 0.1f;

          for (int sample = 0; sample < bufferToFill.numSamples; sample++)
          {
              totalTime += 1.0f/(float)currentSampleRate;
              float pulsePeriod = 1.0f/currentFrequency;

              auto currentSample = std::fmod(totalTime, pulsePeriod) / pulsePeriod;
              currentSample = (currentSample < alpha) ? 1.0f: 0.0f;
              
              if (activeFilter == LowPass && lowPassFilter.coefficients != nullptr) {
                  currentSample = lowPassFilter.processSample(currentSample);
              } else if (activeFilter == HighPass && highPassFilter.coefficients != nullptr) {
                  currentSample = highPassFilter.processSample(currentSample);
              } else if (activeFilter == BandPass && bandPassFilter.coefficients != nullptr) {
                  currentSample = bandPassFilter.processSample(currentSample);
              }

              leftBuffer[sample] = currentSample;
              rightBuffer[sample] = currentSample;
              
              waveBuffer.setSample(0, sample % waveBuffer.getNumSamples(), currentSample);
          }
          triggerAsyncUpdate();
      }
      else
      {
          bufferToFill.clearActiveBufferRegion();
      }
 }


void MainComponent::releaseResources()
{
}

void MainComponent::handleAsyncUpdate(){
    repaint();
}
