#pragma once
#include <JuceHeader.h>

class MESABOOGIEINGSOFTAMPAudioProcessor : public juce::AudioProcessor
{
public:
    MESABOOGIEINGSOFTAMPAudioProcessor();
    ~MESABOOGIEINGSOFTAMPAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override { return true; }
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "MESABOOGIEINGOSOFT AMP"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 2.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;

    // ===== APVTS =====
    juce::AudioProcessorValueTreeState apvts;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameters();

    // ===== IR LOADER =====
    //void loadIR(const juce::File& file);

private:
    // ===== DSP CHAIN =====
    double currentSampleRate = 44100.0;
    int currentBlockSize = 512;

    // --- Input/Output Gain ---
    juce::dsp::Gain<float> inputGain, outputGain;

    // --- Noise Gate ---
    juce::dsp::NoiseGate<float> noiseGate;

    // --- Pre-amp (Tube simulation) ---
    // Waveshaper con curva de válvula 12AX7
    juce::dsp::WaveShaper<float> preampStage1; // input stage
    juce::dsp::WaveShaper<float> preampStage2; // gain stage
    juce::dsp::WaveShaper<float> preampStage3; // power stage

    // --- Tone Stack EQ (Bass/Mid/Treble/Presence - estilo Mesa Boogie) ---
    juce::dsp::IIR::Filter<float> bassFilter;
    juce::dsp::IIR::Filter<float> midFilter;
    juce::dsp::IIR::Filter<float> trebleFilter;
    juce::dsp::IIR::Filter<float> presenceFilter;

    // --- Graphic EQ (6 bandas) ---
    juce::dsp::IIR::Filter<float> eqFilters[6];
    float eqFrequencies[6] = { 100.f, 200.f, 400.f, 800.f, 1600.f, 3200.f };

    // --- Cabinet IR (Convolution) ---
    // juce::dsp::Convolution cabinetConvolution{ juce::dsp::Convolution::NonUniform { 512 } };
    juce::dsp::IIR::Filter<float> lowCutFilter;
    juce::dsp::IIR::Filter<float> highCutFilter;

    // --- Overdrive ---
    juce::dsp::WaveShaper<float> overdriveWS;
    juce::dsp::IIR::Filter<float> overdriveToneFilter;

    // --- Chorus ---
    juce::dsp::Chorus<float> chorus;

    // --- Delay ---
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayLine;
    float delayFeedbackSample = 0.0f;
    std::atomic<float> currentOdDrive{ 0.5f };

    // --- Tuner (YIN pitch detection) ---
    std::vector<float> tunerBuffer;
    int tunerBufferPos = 0;
    static constexpr int TUNER_BUFFER_SIZE = 2048;
    float detectedFrequency = 0.0f;

    // ===== HELPERS =====
    void updateFilters();
    void processTuner(const float* data, int numSamples);
    float yinPitchDetect(const std::vector<float>& buffer, double sampleRate);

    // Curva waveshaper para válvulas (12AX7)
    static float tubeWaveshaper(float x, float drive)
    {
        // Asimetric soft-clip con bias (simula conducción de válvula)
        float bias = 0.1f;
        x = x + bias;
        float driven = x * (1.0f + drive * 9.0f);
        // Soft-clip asimétrico
        if (driven > 1.0f)       return 2.0f / 3.0f;
        else if (driven < -1.0f) return -2.0f / 3.0f;
        return driven - (driven * driven * driven) / 3.0f;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MESABOOGIEINGSOFTAMPAudioProcessor)
};