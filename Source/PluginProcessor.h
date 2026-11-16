#pragma once
#include <JuceHeader.h>
#include "PresetDatabase.h"
#include <thread>
#include <mutex>
#include <atomic>

class MESABOOGIEINGSOFTAMPAudioProcessor : public juce::AudioProcessor,
    private juce::ValueTree::Listener
{
public:
    MESABOOGIEINGSOFTAMPAudioProcessor();
    ~MESABOOGIEINGSOFTAMPAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    // FIX #22: isBusesLayoutSupported ahora valida el layout en vez de aceptar cualquier cosa
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override
    {
        if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo() &&
            layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono())
            return false;
        if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
            return false;
        return true;
    }

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "NoEraCleanDSP"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 2.0; }
    void loadCabinetIR(const juce::File& file);
    void clearCabinetIR();

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;

    // --- Sección de Presets ---
    void loadPresetFromDB(int presetID);

    // ===== APVTS =====
    juce::AudioProcessorValueTreeState apvts;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameters();
    float getDetectedFrequency() const { return detectedFrequency.load(); }

    // ===== BASE DE DATOS =====
    PresetDatabase database;

private:
    // ===== DSP CHAIN =====
    double currentSampleRate = 44100.0;
    int currentBlockSize = 512;

    // --- Input/Output Gain ---
    juce::dsp::Gain<float> inputGain, outputGain;

    // --- Noise Gate ---
    juce::dsp::NoiseGate<float> noiseGate;

    // --- Pre-amp (Tube simulation) ---
    juce::dsp::WaveShaper<float> preampStage1; // cathode follower — soft asymmetric
    juce::dsp::WaveShaper<float> preampStage2; // hard clip asimétrico — corazón del Rectifier
    juce::dsp::WaveShaper<float> preampStage3; // power amp — hard clip simétrico final

    // --- Filtros fijos de carácter Mesa ---
    // FIX #13: coeficientes se recalculan en updateFilters() para soportar cambios de sample rate
    juce::dsp::IIR::Filter<float> charBassFilter;
    juce::dsp::IIR::Filter<float> charMidFilter;
    juce::dsp::IIR::Filter<float> charTrebleFilter;
    juce::dsp::IIR::Filter<float> charAirFilter;

    // --- Tone Stack EQ (Bass/Mid/Treble/Presence) ---
    juce::dsp::IIR::Filter<float> bassFilter;
    juce::dsp::IIR::Filter<float> midFilter;
    juce::dsp::IIR::Filter<float> trebleFilter;
    juce::dsp::IIR::Filter<float> presenceFilter;

    // --- Graphic EQ (6 bandas) ---
    juce::dsp::IIR::Filter<float> eqFilters[6];
    // FIX #9: frecuencias redistribuidas para ser útiles post-distorsión
    float eqFrequencies[6] = { 100.f, 250.f, 500.f, 1000.f, 2500.f, 5000.f };

    // --- Cabinet filters ---
    juce::dsp::IIR::Filter<float> lowCutFilter;
    juce::dsp::IIR::Filter<float> highCutFilter;

    juce::dsp::Convolution irLoader;
    juce::dsp::Gain<float> irOutputGain;

    // --- Overdrive ---
    juce::dsp::WaveShaper<float> overdriveWS;
    juce::dsp::IIR::Filter<float> overdriveToneFilter;

    // --- Chorus ---
    juce::dsp::Chorus<float> chorus;

    // --- Delay ---
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayLine;
    // FIX #28: delayFeedbackSample eliminado — era variable declarada sin usar
    std::atomic<float> currentOdDrive{ 0.5f };

    // --- Oversampling 4x anti-aliasing ---
    juce::dsp::Oversampling<float> oversampler{ 1, 2, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR };

    // --- Pre-gain tight filters (ANTES de distorsionar) ---
    juce::dsp::IIR::Filter<float> preHPF;         // HPF 80Hz — tight sin matar cuerpo
    juce::dsp::IIR::Filter<float> preLowMidCut;   // Corte 200Hz — quita barro pre-clip
    juce::dsp::IIR::Filter<float> preAttackBoost; // Boost 2.2kHz — pick attack

    // --- Post-clipping cleanup ---
    juce::dsp::IIR::Filter<float> postFizzCut;    // LPF 9kHz — anti-fizz digital
    juce::dsp::IIR::Filter<float> postBodyCut;    // Corte 400Hz — barro post-clip

    // --- Tuner (YIN pitch detection) ---
    std::vector<float> tunerBuffer;
    std::vector<float> tunerInputCopy;   // FIX #23: copia limpia pre-distorsión para el tuner
    int tunerBufferPos = 0;
    static constexpr int TUNER_BUFFER_SIZE = 2048;
    std::atomic<float> detectedFrequency{ 0.0f };

    // FIX #24: YIN en thread separado para no bloquear el audio thread
    std::thread tunerThread;
    std::atomic<bool> tunerReady{ false };
    std::atomic<bool> shouldRunTuner{ true };
    std::mutex tunerMutex;

    // FIX #7: flag para recalcular filtros solo cuando cambian parámetros
    std::atomic<bool> filtersNeedUpdate{ true };

    std::atomic<bool> irBypassed{ false };

    // ===== HELPERS =====
    void updateFilters();
    void resetAllFilters();
    void processTuner(const float* data, int numSamples);
    float yinPitchDetect(const std::vector<float>& buffer, double sampleRate);
    void valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier&) override
    {
        filtersNeedUpdate.store(true);
    }

    // FIX #11: tubeWaveshaper eliminada — estaba declarada pero nunca usada

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MESABOOGIEINGSOFTAMPAudioProcessor)
};