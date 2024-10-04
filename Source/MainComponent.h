#pragma once

#include <JuceHeader.h>

class MainComponent  : public juce::AudioAppComponent, public juce::AsyncUpdater
{
public:
    MainComponent(); // Constructor
    ~MainComponent() override; // Destructor

    void paint (juce::Graphics&) override; // UI
    void resized() override; // UI
    
    void mouseDown(const juce::MouseEvent& event) override; // Mouse event handler; called when the user clicks the mouse
    void mouseDrag(const juce::MouseEvent& event) override; // Mouse event handler; called when the user drags the mouse
    void mouseUp(const juce::MouseEvent& event) override; // Mouse event handler; called when the user releases the mouse
    void updateSignal(const juce::Point<int>& position); // Update the frequency and amplitude of the generated sound based on the current position of the mouse
    void updateAngleDelta(); // Update the angle delta, which determines the speed of the sine wave oscillation based on the current frequency
    void prepareToPlay(int, double sampleRate) override; // called when the audio device starts
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override; //Release the audio(sound) when the program shuts down
    
    void generateSineWave(const juce::AudioSourceChannelInfo& bufferToFill);
    void generateTrainOfPulses(const juce::AudioSourceChannelInfo& bufferToFill);
    
    juce::AudioBuffer<float> waveBuffer; // Audio buffer storing a sequence of audio samples
    
    //Low Pass Filter
    juce::dsp::IIR::Filter<float> lowPassFilter;
    
    //High Pass Filter
    juce::dsp::IIR::Filter<float> highPassFilter;
    
    //Band Pass Filter
    juce::dsp::IIR::Filter<float> bandPassFilter;
    
    
private:
    juce::Rectangle<int> controlBox;
    
    bool AudioOn = false;
    
    float currentSampleRate;
    float currentAngle; // current phase of the sine wave at any given point in time
    float angleDelta; // determines how much the phase increases between samples
    
    float currentFrequency = 0.0f;
    float currentAmplitude = 0.0f; //current amplitude is fixed at 1.0f when applying filters
    float currentCutOff = 0.0f;
    float centralFrequency = 0.0f;
    
    float totalTime = 0.0f;
    
    // Buttons
    juce::TextButton waveformButton { "Waveform" };
    juce::TextButton filterButton { "Filter" };
    juce::TextButton sineButton { "Sine Wave" };
    juce::TextButton pulseButton { "Train of Pulses" };
    juce::TextButton lowPassButton { "Low Pass Filter" };
    juce::TextButton highPassButton { "High Pass Filter" };
    juce::TextButton bandPassButton { "Band Pass Filter" };
    juce::TextButton noneButton { "None" };
    
    bool waveformVisible;
    bool filterVisible;

    // Waveform type
    enum WaveformType { SineWave, TrainOfPulses };
    WaveformType currentWaveformType = SineWave;
    
    // Filter type
    enum FilterType { None, LowPass, HighPass, BandPass };
    FilterType activeFilter = None;
    
    juce::Label xAxisLabel;
    juce::Label yAxisLabel;
    void updateAxisLabels();
    
    void handleAsyncUpdate() override;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};

