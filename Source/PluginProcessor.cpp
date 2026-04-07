#include "PluginProcessor.h"
#include "PluginEditor.h"

// ===================================================
// PARAMETER LAYOUT
// ===================================================
juce::AudioProcessorValueTreeState::ParameterLayout
MESABOOGIEINGSOFTAMPAudioProcessor::createParameters()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Input / Output
    params.push_back(std::make_unique<juce::AudioParameterFloat>("INPUT", "Input", -20.0f, 20.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("OUTPUT", "Output", -20.0f, 20.0f, 0.0f));

    // Amp
    params.push_back(std::make_unique<juce::AudioParameterFloat>("GAIN", "Gain", 0.0f, 10.0f, 5.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("BASS", "Bass", 0.0f, 10.0f, 5.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("MID", "Mid", 0.0f, 10.0f, 5.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("TREBLE", "Treble", 0.0f, 10.0f, 5.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("PRESENCE", "Presence", 0.0f, 10.0f, 5.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("MASTER", "Master", 0.0f, 10.0f, 5.0f));

    // Cabinet
    params.push_back(std::make_unique<juce::AudioParameterFloat>("LOWCUT", "Low Cut", 10.0f, 200.0f, 10.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("HIGHCUT", "High Cut", 5000.0f, 20000.0f, 20000.0f));
    params.push_back(std::make_unique<juce::AudioParameterInt>("MIC", "Mic", 1, 4, 1));

    // Overdrive
    params.push_back(std::make_unique<juce::AudioParameterFloat>("OD_DRIVE", "OD Drive", 0.0f, 1.0f, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("OD_TONE", "OD Tone", 0.0f, 1.0f, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("OD_LEVEL", "OD Level", 0.0f, 1.0f, 0.5f));

    // Chorus
    params.push_back(std::make_unique<juce::AudioParameterFloat>("CHO_RATE", "Cho Rate", 0.0f, 1.0f, 0.3f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("CHO_DEPTH", "Cho Depth", 0.0f, 1.0f, 0.3f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("CHO_LEVEL", "Cho Level", 0.0f, 1.0f, 0.5f));

    // Delay
    params.push_back(std::make_unique<juce::AudioParameterFloat>("DEL_TIME", "Del Time", 0.0f, 1.0f, 0.3f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("DEL_FB", "Del FB", 0.0f, 0.95f, 0.4f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("DEL_MIX", "Del Mix", 0.0f, 1.0f, 0.3f));

    // EQ 6 Bandas
    for (int i = 1; i <= 6; ++i)
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            "EQ" + juce::String(i), "EQ Band " + juce::String(i), -12.0f, 12.0f, 0.0f));

    return { params.begin(), params.end() };
}

// ===================================================
// CONSTRUCTOR / DESTRUCTOR
// ===================================================
MESABOOGIEINGSOFTAMPAudioProcessor::MESABOOGIEINGSOFTAMPAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
    apvts(*this, nullptr, "Parameters", createParameters())
{
    tunerBuffer.resize(TUNER_BUFFER_SIZE, 0.0f);
}

MESABOOGIEINGSOFTAMPAudioProcessor::~MESABOOGIEINGSOFTAMPAudioProcessor() {}

// ===================================================
// PREPARE TO PLAY
// ===================================================
void MESABOOGIEINGSOFTAMPAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    currentBlockSize = samplesPerBlock;

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = (juce::uint32)samplesPerBlock;
    spec.numChannels = 2;

    // Input/Output Gain
    inputGain.prepare(spec);
    inputGain.setRampDurationSeconds(0.05);
    outputGain.prepare(spec);
    outputGain.setRampDurationSeconds(0.05);

    // Noise Gate
    noiseGate.prepare(spec);
    noiseGate.setThreshold(-96.0f);
    noiseGate.setAttack(1.0f);
    noiseGate.setRelease(200.0f);
    noiseGate.setRatio(2.0f);

    // --- Waveshapers (válvulas) ---
    preampStage1.prepare(spec);
    preampStage1.functionToUse = [](float x)
        {
            x = x * 1.5f;
            if (x >= 0.0f)
                return 1.0f - std::exp(-x);
            else
                return -1.0f + std::exp(x * 0.7f);
        };

    preampStage2.prepare(spec);
    preampStage2.functionToUse = [](float x)
        {
            float driven = x * 2.5f;
            float clip = std::tanh(driven + 0.15f) * 0.6f
                + std::tanh(driven * 0.5f) * 0.4f;
            return clip * 0.8f;
        };

    preampStage3.prepare(spec);
    preampStage3.functionToUse = [](float x)
        {
            float sign = x >= 0.0f ? 1.0f : -1.0f;
            float abs_x = std::abs(x);
            return sign * (1.0f - std::exp(-abs_x * 2.0f)) * 0.85f;
        };

    // --- Tone Stack (Filtros IIR) ---
    // Se actualizan en updateFilters()
    bassFilter.prepare(spec);
    midFilter.prepare(spec);
    trebleFilter.prepare(spec);
    presenceFilter.prepare(spec);

    // --- EQ 6 bandas ---
    for (auto& f : eqFilters)
        f.prepare(spec);

    // --- Cabinet IR ---
    // cabinetConvolution.prepare(spec);
    // cabinetConvolution.reset();
    lowCutFilter.prepare(spec);
    highCutFilter.prepare(spec);

    // --- Overdrive ---
    overdriveWS.prepare(spec);
    overdriveWS.functionToUse = [](float x) -> float
        {
            return x - (x * x * x) / 3.0f; // soft clip simple en prepare
        };
    overdriveToneFilter.prepare(spec);

    // --- Chorus ---
    chorus.prepare(spec);

    // --- Delay (max 2 segundos) ---
    delayLine.prepare(spec);
    delayLine.setMaximumDelayInSamples((int)(sampleRate * 2.0));

    updateFilters();
}

// ===================================================
// UPDATE FILTERS (llamado en processBlock cada bloque)
// ===================================================
void MESABOOGIEINGSOFTAMPAudioProcessor::updateFilters()
{
    double sr = currentSampleRate;

    // ---- TONE STACK MESA BOOGIE ----
    float bass = apvts.getRawParameterValue("BASS")->load() / 10.0f;
    float mid = apvts.getRawParameterValue("MID")->load() / 10.0f;
    float treble = apvts.getRawParameterValue("TREBLE")->load() / 10.0f;
    float presence = apvts.getRawParameterValue("PRESENCE")->load() / 10.0f;

    // Bass: Low shelf a 200Hz
    *bassFilter.coefficients = *juce::dsp::IIR::Coefficients<float>::makeLowShelf(
        sr, 200.0f, 0.7f, juce::Decibels::decibelsToGain((bass - 0.5f) * 24.0f));

    // Mid: Peak a 750Hz (el mid scoop típico del Mesa)
    *midFilter.coefficients = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(
        sr, 750.0f, 1.2f, juce::Decibels::decibelsToGain((mid - 0.5f) * 20.0f));

    // Treble: High shelf a 3.5kHz
    *trebleFilter.coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(
        sr, 3500.0f, 0.7f, juce::Decibels::decibelsToGain((treble - 0.5f) * 20.0f));

    // Presence: High shelf a 6kHz (el "bite" del Mesa)
    *presenceFilter.coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(
        sr, 6000.0f, 1.5f, juce::Decibels::decibelsToGain((presence - 0.5f) * 16.0f));

    // ---- EQ GRÁFICO 6 BANDAS ----
    float eqGains[6];
    for (int i = 0; i < 6; ++i)
        eqGains[i] = apvts.getRawParameterValue("EQ" + juce::String(i + 1))->load();

    for (int i = 0; i < 6; ++i)
    {
        *eqFilters[i].coefficients = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(
            sr,
            eqFrequencies[i],
            1.41f,
            juce::Decibels::decibelsToGain(eqGains[i]));
    }

    // ---- LOW CUT / HIGH CUT del Cabinet ----
    float lowCut = apvts.getRawParameterValue("LOWCUT")->load();
    float highCut = apvts.getRawParameterValue("HIGHCUT")->load();

    *lowCutFilter.coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighPass(sr, lowCut, 0.707f);
    *highCutFilter.coefficients = *juce::dsp::IIR::Coefficients<float>::makeLowPass(sr, highCut, 0.707f);

    // ---- OVERDRIVE TONE ----
    float odTone = apvts.getRawParameterValue("OD_TONE")->load();
    float toneFreq = 500.0f + odTone * 4500.0f; // 500Hz a 5kHz
    *overdriveToneFilter.coefficients =
        *juce::dsp::IIR::Coefficients<float>::makeLowPass(sr, toneFreq, 0.707f);

    // ---- CHORUS ----
    float choRate = apvts.getRawParameterValue("CHO_RATE")->load();
    float choDepth = apvts.getRawParameterValue("CHO_DEPTH")->load();
    chorus.setRate(0.1f + choRate * 3.9f);   // 0.1 - 4 Hz
    chorus.setDepth(choDepth * 0.02f);           // 0 - 20ms
    chorus.setMix(apvts.getRawParameterValue("CHO_LEVEL")->load() * 0.5f);
    chorus.setCentreDelay(7.0f);
    chorus.setFeedback(0.1f);
}

// ===================================================
// PROCESS BLOCK
// ===================================================
void MESABOOGIEINGSOFTAMPAudioProcessor::processBlock(
    juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    for (int ch = 1; ch < buffer.getNumChannels(); ++ch)
        buffer.copyFrom(ch, 0, buffer.getReadPointer(0), buffer.getNumSamples());

    // ---- Leer parámetros ----
    float inputdB = apvts.getRawParameterValue("INPUT")->load();
    float outputdB = apvts.getRawParameterValue("OUTPUT")->load();
    float gain = apvts.getRawParameterValue("GAIN")->load() / 10.0f;   // 0-1
    float master = apvts.getRawParameterValue("MASTER")->load() / 10.0f;
    float odDrive = apvts.getRawParameterValue("OD_DRIVE")->load();
    float odLevel = apvts.getRawParameterValue("OD_LEVEL")->load();
    float delTime = apvts.getRawParameterValue("DEL_TIME")->load();
    float delFb = apvts.getRawParameterValue("DEL_FB")->load();
    float delMix = apvts.getRawParameterValue("DEL_MIX")->load();

    updateFilters();

    auto block = juce::dsp::AudioBlock<float>(buffer);
    auto ctx = juce::dsp::ProcessContextReplacing<float>(block);

    // =========================================
    // 1. INPUT GAIN
    // =========================================
    inputGain.setGainDecibels(inputdB);
    inputGain.process(ctx);

    // =========================================
    // 2. NOISE GATE
    // =========================================
    noiseGate.process(ctx);

    // =========================================
    // 3. OVERDRIVE (pre-amp, en paralelo dry/wet)
    // =========================================
    if (odDrive > 0.01f || odLevel > 0.01f)
    {
        juce::AudioBuffer<float> odBuffer(buffer.getNumChannels(), buffer.getNumSamples());
        odBuffer.copyFrom(0, 0, buffer, 0, 0, buffer.getNumSamples());
        if (buffer.getNumChannels() > 1)
            odBuffer.copyFrom(1, 0, buffer, 1, 0, buffer.getNumSamples());

        auto odBlock = juce::dsp::AudioBlock<float>(odBuffer);
        auto odCtx = juce::dsp::ProcessContextReplacing<float>(odBlock);

        // Gain pre-shape
        for (int ch = 0; ch < odBuffer.getNumChannels(); ++ch)
        {
            auto* data = odBuffer.getWritePointer(ch);
            for (int s = 0; s < odBuffer.getNumSamples(); ++s)
                data[s] *= (1.0f + odDrive * 10.0f);
        }

        // Actualizar drive en el atómico (thread-safe)
        currentOdDrive.store(odDrive);
        overdriveWS.process(odCtx);
        overdriveToneFilter.process(odCtx);

        // Mix wet con dry
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            auto* dry = buffer.getWritePointer(ch);
            auto* wet = odBuffer.getReadPointer(ch);
            for (int s = 0; s < buffer.getNumSamples(); ++s)
                dry[s] = dry[s] * (1.0f - odLevel) + wet[s] * odLevel * 0.3f;
        }
    }

    // =========================================
    // 4. PRE-AMP TUBE SIMULATION (3 etapas)
    // =========================================
    // Aplicar gain del amp antes de los stages
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* data = buffer.getWritePointer(ch);
        for (int s = 0; s < buffer.getNumSamples(); ++s)
            data[s] *= (1.0f + gain * 15.0f); // 0.1 a 2.6 (no lineal)
    }

    preampStage1.process(ctx);
    preampStage2.process(ctx);

    // =========================================
    // 5. TONE STACK (Bass/Mid/Treble/Presence)
    // =========================================
    bassFilter.process(ctx);
    midFilter.process(ctx);
    trebleFilter.process(ctx);

    // =========================================
    // 6. POWER AMP
    // =========================================
    preampStage3.process(ctx);
    presenceFilter.process(ctx); // Presence actúa en el power amp loop

    // =========================================
    // 7. TUNER (tap del signal limpio post-preamp)
    // =========================================
    processTuner(buffer.getReadPointer(0), buffer.getNumSamples());

    // =========================================
    // 8. CABINET IR + FILTERS
    // =========================================
    // cabinetConvolution.process(ctx);
    lowCutFilter.process(ctx);
    highCutFilter.process(ctx);

    // =========================================
    // 9. EQ GRÁFICO 6 BANDAS
    // =========================================
    for (auto& f : eqFilters)
        f.process(ctx);

    // =========================================
    // 10. CHORUS
    // =========================================
    chorus.process(ctx);

    // =========================================
    // 11. DELAY
    // =========================================
    int delaySamples = (int)(delTime * currentSampleRate * 1.0); // 0 a 1 segundo
    delaySamples = juce::jmax(1, delaySamples);
    delayLine.setDelay((float)delaySamples);

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* data = buffer.getWritePointer(ch);
        for (int s = 0; s < buffer.getNumSamples(); ++s)
        {
            float delayed = delayLine.popSample(ch, (float)delaySamples);
            float input = data[s] + delayed * delFb;
            delayLine.pushSample(ch, input);
            data[s] = data[s] * (1.0f - delMix) + delayed * delMix;
        }
    }

    // =========================================
    // 12. MASTER + OUTPUT GAIN
    // =========================================
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* data = buffer.getWritePointer(ch);
        for (int s = 0; s < buffer.getNumSamples(); ++s)
            data[s] *= master;
    }

    outputGain.setGainDecibels(outputdB);
    outputGain.process(ctx);
}

// ===================================================
// TUNER - YIN PITCH DETECTION
// ===================================================
void MESABOOGIEINGSOFTAMPAudioProcessor::processTuner(
    const float* data, int numSamples)
{
    for (int i = 0; i < numSamples; ++i)
    {
        tunerBuffer[tunerBufferPos] = data[i];
        tunerBufferPos = (tunerBufferPos + 1) % TUNER_BUFFER_SIZE;
    }

    if (tunerBufferPos == 0) // Buffer lleno, detectar pitch
        detectedFrequency = yinPitchDetect(tunerBuffer, currentSampleRate);
}

float MESABOOGIEINGSOFTAMPAudioProcessor::yinPitchDetect(
    const std::vector<float>& buf, double sr)
{
    // Implementación del algoritmo YIN
    int N = (int)buf.size();
    int halfN = N / 2;
    std::vector<float> d(halfN, 0.0f);

    // Paso 1 y 2: Difference function
    for (int tau = 1; tau < halfN; ++tau)
    {
        float sum = 0.0f;
        for (int j = 0; j < halfN; ++j)
        {
            float diff = buf[j] - buf[j + tau];
            sum += diff * diff;
        }
        d[tau] = sum;
    }

    // Paso 3: Cumulative mean normalized
    std::vector<float> cmn(halfN, 0.0f);
    cmn[0] = 1.0f;
    float runningSum = 0.0f;
    for (int tau = 1; tau < halfN; ++tau)
    {
        runningSum += d[tau];
        cmn[tau] = d[tau] / ((1.0f / tau) * runningSum);
    }

    // Paso 4: Threshold (0.1 recomendado por de Cheveigné & Kawahara)
    float threshold = 0.15f;
    int tau = -1;
    for (int t = 2; t < halfN; ++t)
    {
        if (cmn[t] < threshold)
        {
            // Buscar mínimo local
            while (t + 1 < halfN && cmn[t + 1] < cmn[t])
                ++t;
            tau = t;
            break;
        }
    }

    if (tau == -1) return 0.0f; // No detectado

    // Paso 5: Interpolación parabólica para mayor precisión
    float betterTau = (float)tau;
    if (tau > 0 && tau < halfN - 1)
    {
        float s0 = cmn[tau - 1];
        float s1 = cmn[tau];
        float s2 = cmn[tau + 1];
        betterTau += (s2 - s0) / (2.0f * (2.0f * s1 - s2 - s0));
    }

    return (float)(sr / betterTau);
}

// ===================================================
// IR LOADER
// ===================================================
//void MESABOOGIEINGSOFTAMPAudioProcessor::loadIR(const juce::File& file)
//{
    //cabinetConvolution.loadImpulseResponse(
     //   file,
       // juce::dsp::Convolution::Stereo::yes,
       // juce::dsp::Convolution::Trim::yes,
       // 0);
//}

// ===================================================
// STATE INFORMATION (Save/Load preset)
// ===================================================
void MESABOOGIEINGSOFTAMPAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void MESABOOGIEINGSOFTAMPAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState && xmlState->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

void MESABOOGIEINGSOFTAMPAudioProcessor::releaseResources() {}

juce::AudioProcessorEditor* MESABOOGIEINGSOFTAMPAudioProcessor::createEditor()
{
    return new MESABOOGIEINGSOFTAMPAudioProcessorEditor(*this);
}

// ===================================================
// PLUGIN ENTRY POINT
// ===================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MESABOOGIEINGSOFTAMPAudioProcessor();
}