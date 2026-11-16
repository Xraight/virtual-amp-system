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
    // FIX #2: LOWCUT default cambiado de 10Hz a 80Hz — empieza cortando graves desde el inicio
    params.push_back(std::make_unique<juce::AudioParameterFloat>("LOWCUT", "Low Cut", 10.0f, 200.0f, 80.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("HIGHCUT", "High Cut", 5000.0f, 20000.0f, 20000.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("IR_GAIN", "IR Gain", -20.0f, 20.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterInt>("MIC", "Mic", 1, 4, 1));

    // Overdrive
    params.push_back(std::make_unique<juce::AudioParameterBool>("OD_ENABLE", "OD Enable", true));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("OD_DRIVE", "OD Drive", 0.0f, 1.0f, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("OD_TONE", "OD Tone", 0.0f, 1.0f, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("OD_LEVEL", "OD Level", 0.0f, 1.0f, 0.5f));

    // Chorus
    params.push_back(std::make_unique<juce::AudioParameterBool>("CHO_ENABLE", "Cho Enable", true));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("CHO_RATE", "Cho Rate", 0.0f, 1.0f, 0.3f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("CHO_DEPTH", "Cho Depth", 0.0f, 1.0f, 0.3f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("CHO_LEVEL", "Cho Level", 0.0f, 1.0f, 0.5f));

    // Delay
    params.push_back(std::make_unique<juce::AudioParameterBool>("DEL_ENABLE", "Del Enable", true));
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
// CONSTRUCTOR
// ===================================================
MESABOOGIEINGSOFTAMPAudioProcessor::MESABOOGIEINGSOFTAMPAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
    apvts(*this, nullptr, "Parameters", createParameters())
{
    tunerBuffer.resize(TUNER_BUFFER_SIZE, 0.0f);
    tunerInputCopy.resize(TUNER_BUFFER_SIZE, 0.0f);
    apvts.state.addListener(this);

    // FIX #24: lanzar el thread del tuner aquí para que no bloquee el audio thread
    shouldRunTuner.store(true);
    tunerThread = std::thread([this]()
        {
            while (shouldRunTuner.load())
            {
                if (tunerReady.exchange(false))
                {
                    std::vector<float> localBuffer;
                    {
                        std::lock_guard<std::mutex> lock(tunerMutex);
                        localBuffer = tunerInputCopy;
                    }
                    detectedFrequency.store(yinPitchDetect(localBuffer, currentSampleRate));
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        });
}

// ===================================================
// DESTRUCTOR
// ===================================================
MESABOOGIEINGSOFTAMPAudioProcessor::~MESABOOGIEINGSOFTAMPAudioProcessor()
{
    // FIX #24: detener el thread del tuner limpiamente
    shouldRunTuner.store(false);
    if (tunerThread.joinable())
        tunerThread.join();
}

// ===================================================
// RESET ALL FILTERS — helper para prepareToPlay y releaseResources
// ===================================================
void MESABOOGIEINGSOFTAMPAudioProcessor::resetAllFilters()
{
    // FIX #20: reset de todos los filtros para limpiar estado interno
    charBassFilter.reset();
    charMidFilter.reset();
    charTrebleFilter.reset();
    charAirFilter.reset();
    bassFilter.reset();
    midFilter.reset();
    trebleFilter.reset();
    presenceFilter.reset();
    preHPF.reset();
    preLowMidCut.reset();
    preAttackBoost.reset();
    postFizzCut.reset();
    postBodyCut.reset();
    lowCutFilter.reset();
    highCutFilter.reset();
    overdriveToneFilter.reset();
    for (auto& f : eqFilters) f.reset();
}

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

    // FIX #12: spec oversampled para preparar los waveshapers correctamente
    juce::dsp::ProcessSpec oversampledSpec;
    oversampledSpec.sampleRate = sampleRate * 4.0;
    oversampledSpec.maximumBlockSize = (juce::uint32)(samplesPerBlock * 4);
    oversampledSpec.numChannels = 1;

    // --- Input/Output Gain ---
    inputGain.prepare(spec);
    inputGain.setRampDurationSeconds(0.05);
    outputGain.prepare(spec);
    outputGain.setRampDurationSeconds(0.05);

    // --- Noise Gate ---
    // FIX #5: threshold, ratio y release corregidos para high gain real
    noiseGate.prepare(spec);
    noiseGate.setThreshold(-55.0f);  // antes: -96dB — nunca se activaba
    noiseGate.setAttack(0.5f);       // ataque rápido — no corta el pick attack
    noiseGate.setRelease(100.0f);    // release más corto — cierra más limpio
    noiseGate.setRatio(10.0f);       // antes: 2.0 — ratio de gate real, no compresor

    // --- Waveshapers (preparados con spec oversampled) ---
    // FIX #12: se preparan con oversampledSpec porque se procesan dentro del oversampler

    // Stage 1: Soft clip asimétrico (emulación más realista de 12AX7)
    preampStage1.functionToUse = [](float x) -> float
        {
            const float bias = 0.15f;
            // Tangente hiperbólica con offset, restando el bias en la salida
            float output = std::tanh(x * 3.0f + bias) - std::tanh(bias);
            return output * 0.8f;
        };

    // Stage 2: Distorsión principal (Asimetría agresiva exponencial)
    preampStage2.functionToUse = [](float x) -> float
        {
            float drive = 1.8f; // Factor de intensidad
            x *= drive;

            // Función de transferencia suave (Arcotangente optimizada)
            // Esto hace que la distorsión no sea un "muro", sino una curva.
            return (2.0f / juce::MathConstants<float>::pi) * std::atan(x * 1.5f);
        };

    // Stage 3: Power amp clipper
    preampStage3.functionToUse = [](float x) -> float
        {
            // Función algebraica matemática rápida y gruesa para el power amp
            return (x * 1.5f) / (1.0f + std::abs(x * 1.5f));
        };

    // --- Filtros fijos de carácter ---
    charBassFilter.prepare(spec);
    charMidFilter.prepare(spec);
    charTrebleFilter.prepare(spec);
    charAirFilter.prepare(spec);

    // --- Tone Stack ---
    bassFilter.prepare(spec);
    midFilter.prepare(spec);
    trebleFilter.prepare(spec);
    presenceFilter.prepare(spec);

    // --- EQ 6 bandas ---
    for (auto& f : eqFilters)
        f.prepare(spec);

    // --- Cabinet filters ---
    lowCutFilter.prepare(spec);
    highCutFilter.prepare(spec);
    irLoader.prepare(spec);
    irOutputGain.prepare(spec);
    irOutputGain.setRampDurationSeconds(0.05);

    // --- Overdrive ---
    overdriveWS.prepare(spec);
    // FIX #29: OD waveshaper ahora limita la entrada para evitar NaN con input alto
    overdriveWS.functionToUse = [](float x) -> float
        {
            x = juce::jlimit(-1.0f, 1.0f, x); // acota antes de aplicar la curva
            return x - (x * x * x) / 3.0f;
        };
    overdriveToneFilter.prepare(spec);

    // --- Chorus ---
    chorus.prepare(spec);

    // --- Delay ---
    delayLine.prepare(spec);
    delayLine.setMaximumDelayInSamples((int)(sampleRate * 2.0));

    // --- Oversampling ---
    oversampler.reset();
    oversampler.initProcessing((size_t)samplesPerBlock);

    // --- Pre-gain tight filters ---
    preHPF.prepare(spec);
    preLowMidCut.prepare(spec);
    preAttackBoost.prepare(spec);

    // --- Post-clipping cleanup ---
    postFizzCut.prepare(spec);
    postBodyCut.prepare(spec);

    // FIX #20: reset de todos los filtros al inicializar
    resetAllFilters();

    // FIX #7: marca filtros para actualizar en el primer bloque
    filtersNeedUpdate.store(true);

    // updateFilters calcula todos los coeficientes incluyendo char filters
    // FIX #13: char filters ahora se calculan en updateFilters para soportar cambios de SR
    updateFilters();
}

// ===================================================
// UPDATE FILTERS
// ===================================================
void MESABOOGIEINGSOFTAMPAudioProcessor::updateFilters()
{
    double sr = currentSampleRate;

    // ---- CHAR FILTERS (Mesa Rectifier character) ----
    // FIX #13: movidos aquí desde prepareToPlay para recalcular si cambia el sample rate
    // FIX #14: charBassFilter cambiado a peak quirúrgico — menos agresivo que shelf
    // antes: makeLowShelf 118Hz -10dB — demasiado agresivo acumulado con preHPF
    *charBassFilter.coefficients = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(
        sr, 90.0f, 1.0f, juce::Decibels::decibelsToGain(+4.0f));

    // Mid scoop Mesa — se mantiene
    *charMidFilter.coefficients = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(
        sr, 500.0f, 1.5f, juce::Decibels::decibelsToGain(-12.0f));

    // FIX #19: boost de charTreble reducido de +5dB a +2dB para no acumularse con preAttackBoost
    *charTrebleFilter.coefficients = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(
        sr, 4000.0f, 0.7f, juce::Decibels::decibelsToGain(+4.0f));

    // FIX #4: charAirFilter subido a 8kHz y reducido a -4dB
    // antes: 6kHz -8dB se acumulaba con presenceFilter en 6kHz matando la presence
    *charAirFilter.coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(
        sr, 6500.0f, 0.7f, juce::Decibels::decibelsToGain(-6.0f));

    // ---- PRE-GAIN TIGHT FILTERS ----
    // FIX #14: preHPF bajado a 80Hz — menos agresivo, preserva cuerpo en palm mutes
    // antes: 100Hz acumulado con charBassFilter en 118Hz era demasiado
    *preHPF.coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighPass(
        sr, 400.0f, 0.707f);

    // FIX #18: preLowMidCut bajado a 200Hz para no solaparse con postBodyCut en 400Hz
    // antes: 280Hz solapaba con postBodyCut en 320Hz — -13dB acumulados en esa zona
    *preLowMidCut.coefficients = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(
        sr, 7000.0f, 1.2f, juce::Decibels::decibelsToGain(-6.0f));

    // FIX #19: preAttackBoost subido a 2.2kHz y reducido a +2.5dB
    // antes: +4dB a 2kHz + charTreble +5dB a 1630Hz = acumulación excesiva en mid-highs
    *preAttackBoost.coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(
        sr, 1500.0f, 1.0f, juce::Decibels::decibelsToGain(+6.0f));

    // ---- POST-CLIPPING CLEANUP ----
    *postFizzCut.coefficients = *juce::dsp::IIR::Coefficients<float>::makeLowPass(
        sr, 9000.0f, 0.65f);

    // FIX #18: postBodyCut subido a 400Hz — separado de preLowMidCut en 200Hz
    // antes: 320Hz solapaba con preLowMidCut en 280Hz
    *postBodyCut.coefficients = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(
        sr, 400.0f, 1.0f, juce::Decibels::decibelsToGain(-4.0f));

    // ---- TONE STACK MESA BOOGIE ----
    float bass = apvts.getRawParameterValue("BASS")->load() / 10.0f;
    float mid = apvts.getRawParameterValue("MID")->load() / 10.0f;
    float treble = apvts.getRawParameterValue("TREBLE")->load() / 10.0f;
    float presence = apvts.getRawParameterValue("PRESENCE")->load() / 10.0f;

    // FIX #15: rangos de ganancia reducidos para comportamiento más realista
    // antes: bass/treble ±12dB, mid ±10dB — demasiado extremo para un amp real
    *bassFilter.coefficients = *juce::dsp::IIR::Coefficients<float>::makeLowShelf(
        sr, 150.0f, 0.7f, juce::Decibels::decibelsToGain((bass - 0.5f) * 16.0f));  // ±8dB

    *midFilter.coefficients = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(
        sr, 500.0f, 1.5f, juce::Decibels::decibelsToGain((mid - 0.5f) * 14.0f));   // ±7dB

    *trebleFilter.coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(
        sr, 2500.0f, 0.7f, juce::Decibels::decibelsToGain((treble - 0.5f) * 16.0f)); // ±8dB

    // FIX #4: presenceFilter separado de charAirFilter — ahora en 5kHz en vez de 6kHz
    // para no acumularse con charAirFilter que opera desde 8kHz
    *presenceFilter.coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(
        sr, 5000.0f, 1.2f, juce::Decibels::decibelsToGain((presence - 0.5f) * 12.0f)); // ±6dB

    // ---- EQ GRÁFICO 6 BANDAS ----
    for (int i = 0; i < 6; ++i)
    {
        float gain = apvts.getRawParameterValue("EQ" + juce::String(i + 1))->load();
        *eqFilters[i].coefficients = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(
            sr, eqFrequencies[i], 1.41f, juce::Decibels::decibelsToGain(gain));
    }

    // ---- CABINET LOW CUT / HIGH CUT ----
    // FIX #2: lowCutFilter ya no se sobreescribe — solo usa el valor del parámetro
    // el código muerto que asignaba 120Hz fijo fue eliminado
    float lowCut = apvts.getRawParameterValue("LOWCUT")->load();
    float highCut = apvts.getRawParameterValue("HIGHCUT")->load();

    *lowCutFilter.coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighPass(sr, lowCut, 0.707f);
    *highCutFilter.coefficients = *juce::dsp::IIR::Coefficients<float>::makeLowPass(sr, highCut, 0.707f);

    // ---- OVERDRIVE TONE ----
    float odTone = apvts.getRawParameterValue("OD_TONE")->load();
    float toneFreq = 500.0f + odTone * 4500.0f;
    *overdriveToneFilter.coefficients =
        *juce::dsp::IIR::Coefficients<float>::makeLowPass(sr, toneFreq, 0.707f);

    // ---- CHORUS ----
    float choRate = apvts.getRawParameterValue("CHO_RATE")->load();
    float choDepth = apvts.getRawParameterValue("CHO_DEPTH")->load();
    chorus.setRate(0.1f + choRate * 3.9f);
    chorus.setDepth(choDepth * 0.02f);
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

    // FIX #27: mono summing documentado explícitamente
    // Este plugin es mono-in: el canal derecho siempre es copia del izquierdo
    //for (int ch = 1; ch < buffer.getNumChannels(); ++ch)
        //buffer.copyFrom(ch, 0, buffer.getReadPointer(0), buffer.getNumSamples());

    // ---- Leer parámetros ----
    float inputdB = apvts.getRawParameterValue("INPUT")->load();
    float outputdB = apvts.getRawParameterValue("OUTPUT")->load();
    float gain = apvts.getRawParameterValue("GAIN")->load() / 10.0f;
    float master = apvts.getRawParameterValue("MASTER")->load() / 10.0f;
    float odDrive = apvts.getRawParameterValue("OD_DRIVE")->load();
    float odLevel = apvts.getRawParameterValue("OD_LEVEL")->load();
    float delTime = apvts.getRawParameterValue("DEL_TIME")->load();
    float delFb = apvts.getRawParameterValue("DEL_FB")->load();
    float delMix = apvts.getRawParameterValue("DEL_MIX")->load();
    bool odEnable = apvts.getRawParameterValue("OD_ENABLE")->load() > 0.5f;
    bool choEnable = apvts.getRawParameterValue("CHO_ENABLE")->load() > 0.5f;
    bool delEnable = apvts.getRawParameterValue("DEL_ENABLE")->load() > 0.5f;

    // FIX #7: updateFilters solo cuando hay cambios, no cada bloque
        updateFilters();

        auto stereoBlock = juce::dsp::AudioBlock<float>(buffer);
        auto monoBlock = stereoBlock.getSingleChannelBlock(0); // Tomamos solo el lado izquierdo
        auto monoCtx = juce::dsp::ProcessContextReplacing<float>(monoBlock);
        auto stereoCtx = juce::dsp::ProcessContextReplacing<float>(stereoBlock);

    // =========================================
    // 1. INPUT GAIN
    // =========================================
    inputGain.setGainDecibels(inputdB);
    inputGain.process(monoCtx);

    // Acumula muestras limpias pre-distorsión en el buffer del tuner
    processTuner(buffer.getReadPointer(0), buffer.getNumSamples());

    // =========================================
    // 2. NOISE GATE
    // =========================================
    noiseGate.process(monoCtx);

    // =========================================
    // 3. PRE-GAIN EQ — Tightening ANTES de distorsionar
    // FIX #3: movido ANTES del OD para que el HPF corte graves
    //         antes de que el OD los distorsione y genere barro
    // =========================================
    preHPF.process(monoCtx);
    preLowMidCut.process(monoCtx);
    preAttackBoost.process(monoCtx);

    // =========================================
    // 4. OVERDRIVE (paralelo dry/wet)
    // FIX #3: OD ahora recibe señal ya filtrada por el HPF
    // =========================================
    if (odEnable)
    {
        float driveGain = 1.0f + (odDrive * 15.0f);

        // PROBLEMA 1: El overdrive afectaba los 2 canales mal. Ahora solo afecta al Izquierdo.
        auto* data = buffer.getWritePointer(0);
        for (int s = 0; s < buffer.getNumSamples(); ++s)
            data[s] *= driveGain;

        overdriveWS.process(monoCtx);
        overdriveToneFilter.process(monoCtx);

        for (int s = 0; s < buffer.getNumSamples(); ++s)
            data[s] *= (odLevel * 0.5f);
    }

    // =========================================
    // 5. PRE-AMP MULTI-STAGE CON OVERSAMPLING
    // =========================================
    auto oversampledBlock = oversampler.processSamplesUp(monoBlock);
    auto oversampledCtx = juce::dsp::ProcessContextReplacing<float>(oversampledBlock);

    // FIX #6: gain controlado — reduce headroom máximo para preservar dinámica
    // antes: 3.5 + gain*8.0 = hasta x11.5 aplastaba todo con gain alto
    for (int ch = 0; ch < (int)oversampledBlock.getNumChannels(); ++ch)
    {
        auto* data = oversampledBlock.getChannelPointer(ch);
        int   n = (int)oversampledBlock.getNumSamples();
        for (int s = 0; s < n; ++s)
            data[s] *= (3.0f + gain * 7.0f); // max x7 con gain al máximo
    }

    // Stage 1: soft clip asimétrico (cathode follower)
    preampStage1.process(oversampledCtx);

    // Gain inter-stage y DC Blocker rápido
    static float dcBlockerState[2] = { 0.0f, 0.0f }; // Idealmente pon esto en tu .h por canal
    const float R = 0.995f; // Coeficiente para cortar cerca de 10-20Hz

    for (int ch = 0; ch < (int)oversampledBlock.getNumChannels(); ++ch)
    {
        auto* data = oversampledBlock.getChannelPointer(ch);
        int   n = (int)oversampledBlock.getNumSamples();
        for (int s = 0; s < n; ++s)
        {

            // Ganancia
            data[s] *= 0.5f;
        }
    }

    // Stage 2
    preampStage2.process(oversampledCtx);

    // Gain inter-stage 2 — menos agresivo
    for (int ch = 0; ch < (int)oversampledBlock.getNumChannels(); ++ch)
    {
        auto* data = oversampledBlock.getChannelPointer(ch);
        int   n = (int)oversampledBlock.getNumSamples();
        for (int s = 0; s < n; ++s)
            data[s] *= 1.2f; // antes: 1.8
    }

    // Stage 3: power amp hard clip
    preampStage3.process(oversampledCtx);

    // Baja de 4x a sample rate original (el oversampler aplica anti-aliasing aquí)
    oversampler.processSamplesDown(monoBlock);

    // FIX #10: compensación de volumen post-oversampling
    // el filtro anti-aliasing del oversampler atenúa ~3-6dB
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* data = buffer.getWritePointer(ch);
        for (int s = 0; s < buffer.getNumSamples(); ++s)
            data[s] *= 1.6f;
    }

    // =========================================
    // 6. POST-CLIPPING CLEANUP
    // =========================================
    postBodyCut.process(monoCtx);
    postFizzCut.process(monoCtx);

    // =========================================
    // 7. CARÁCTER DEL AMP
    // =========================================
    charBassFilter.process(monoCtx);
    charMidFilter.process(monoCtx);
    charTrebleFilter.process(monoCtx);
    charAirFilter.process(monoCtx);

    // =========================================
    // 8. TONE STACK (Bass/Mid/Treble)
    // =========================================
    bassFilter.process(monoCtx);
    midFilter.process(monoCtx);
    trebleFilter.process(monoCtx);

    // FIX #1: presenceFilter movido aquí — eliminado del paso "POWER AMP" donde
    //         preampStage3 se llamaba por segunda vez causando doble clipping
    presenceFilter.process(monoCtx);
    // =========================================
    // 10. CABINET FILTERS
    // =========================================
    lowCutFilter.process(monoCtx);
    highCutFilter.process(monoCtx);
    if (!irBypassed.load())
    {
        irLoader.process(monoCtx);
        irOutputGain.setGainDecibels(apvts.getRawParameterValue("IR_GAIN")->load());
        irOutputGain.process(monoCtx);
    }

    // =========================================
    // 11. EQ GRÁFICO 6 BANDAS
    // =========================================
    for (auto& f : eqFilters)
        f.process(monoCtx);

    // =========================================
    // 11.5 COPIAR CANAL IZQ AL DER (MONO A STEREO)
    // =========================================
    for (int ch = 1; ch < buffer.getNumChannels(); ++ch)
        buffer.copyFrom(ch, 0, buffer.getReadPointer(0), buffer.getNumSamples());

    // =========================================
        // SOLUCIÓN PROBLEMA 1: CLONACIÓN COHERENTE DE CANALES
        // =========================================
        // Tomamos el canal izquierdo (0) que ya fue procesado por el amplificador, 
        // filtros, EQ e IR, y lo copiamos de forma idéntica al canal derecho (1).
    if (buffer.getNumChannels() > 1)
    {
        buffer.copyFrom(1, 0, buffer.getReadPointer(0), buffer.getNumSamples());
    }
    
    // =========================================
    // 12. DELAY
    // FIX #16: delay va ANTES del chorus — orden correcto de cadena de efectos
    // =========================================
    int delaySamples = (int)(delTime * currentSampleRate);
    delaySamples = juce::jmax(1, delaySamples);
    delayLine.setDelay((float)delaySamples);

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* data = buffer.getWritePointer(ch);
        for (int s = 0; s < buffer.getNumSamples(); ++s)
        {
            float delayed = delayLine.popSample(ch);
            float toWrite = data[s] + delayed * delFb;
            toWrite = juce::jlimit(-1.0f, 1.0f, toWrite);
            delayLine.pushSample(ch, toWrite);

            // Si el pedal está encendido, mezclamos el delay. Si no, la señal limpia pasa intacta.
            if (delEnable)
                data[s] = data[s] * (1.0f - delMix) + delayed * delMix;
        }
    }

    // =========================================
    // 13. CHORUS
    // FIX #16: chorus va DESPUÉS del delay
    // =========================================

    if (choEnable)
        chorus.process(stereoCtx);

    // =========================================
    // 14. MASTER + OUTPUT GAIN
    // =========================================
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* data = buffer.getWritePointer(ch);
        for (int s = 0; s < buffer.getNumSamples(); ++s)
            data[s] *= master;
    }

    // FIX #17: soft limiter de seguridad antes del output — evita clipping duro en el DAW
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* data = buffer.getWritePointer(ch);
        for (int s = 0; s < buffer.getNumSamples(); ++s)
        {
            float x = data[s];
            data[s] = x / (1.0f + std::abs(x)); // nunca clipea duro, solo redondea
        }
    }

    // FIX #30: sanitizer de NaN/infinitos — evita crashes o ruido en el DAW
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* data = buffer.getWritePointer(ch);
        for (int s = 0; s < buffer.getNumSamples(); ++s)
        {
            if (!std::isfinite(data[s]))
                data[s] = 0.0f;
        }
    }

    outputGain.setGainDecibels(outputdB);
    outputGain.process(stereoCtx);
}

// ===================================================
// TUNER — processTuner solo acumula muestras y dispara el thread
// FIX #24: el YIN corre en tunerThread, no aquí
// ===================================================
void MESABOOGIEINGSOFTAMPAudioProcessor::processTuner(
    const float* data, int numSamples)
{
    for (int i = 0; i < numSamples && tunerBufferPos < TUNER_BUFFER_SIZE; ++i)
    {
        tunerBuffer[tunerBufferPos] = data[i];
        tunerBufferPos = (tunerBufferPos + 1) % TUNER_BUFFER_SIZE;
    }

    if (tunerBufferPos == 0)
    {
        // Copia al buffer compartido y señala al thread que hay datos listos
        {
            std::lock_guard<std::mutex> lock(tunerMutex);
            tunerInputCopy = tunerBuffer;
        }
        tunerReady.store(true);
    }
}

// ===================================================
// YIN PITCH DETECTION
// ===================================================
float MESABOOGIEINGSOFTAMPAudioProcessor::yinPitchDetect(
    const std::vector<float>& buf, double sr)
{
    int N = (int)buf.size();
    int halfN = N / 2;
    std::vector<float> d(halfN, 0.0f);

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

    std::vector<float> cmn(halfN, 0.0f);
    cmn[0] = 1.0f;
    float runningSum = 0.0f;
    for (int tau = 1; tau < halfN; ++tau)
    {
        runningSum += d[tau];
        cmn[tau] = d[tau] / ((1.0f / tau) * runningSum);
    }

    float threshold = 0.15f;
    int tau = -1;
    for (int t = 2; t < halfN; ++t)
    {
        if (cmn[t] < threshold)
        {
            while (t + 1 < halfN && cmn[t + 1] < cmn[t])
                ++t;
            tau = t;
            break;
        }
    }

    if (tau == -1) return 0.0f;

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
// STATE INFORMATION
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

// ===================================================
// LOAD PRESET FROM DB
// ===================================================
void MESABOOGIEINGSOFTAMPAudioProcessor::loadPresetFromDB(int presetID)
{
    auto p = database.getPresetByID(presetID);
    if (p.id == 0) return;

    if (p.id == 1)
        clearCabinetIR();

    // Captura p por valor para el lambda — thread-safe
    juce::MessageManager::callAsync([this, p]()
        {
            apvts.getParameter("GAIN")->setValueNotifyingHost(
                apvts.getParameter("GAIN")->convertTo0to1(p.gain));
            apvts.getParameter("BASS")->setValueNotifyingHost(
                apvts.getParameter("BASS")->convertTo0to1(p.bass));
            apvts.getParameter("MID")->setValueNotifyingHost(
                apvts.getParameter("MID")->convertTo0to1(p.mid));
            apvts.getParameter("TREBLE")->setValueNotifyingHost(
                apvts.getParameter("TREBLE")->convertTo0to1(p.treble));
            apvts.getParameter("PRESENCE")->setValueNotifyingHost(
                apvts.getParameter("PRESENCE")->convertTo0to1(p.presence));
            apvts.getParameter("MASTER")->setValueNotifyingHost(
                apvts.getParameter("MASTER")->convertTo0to1(p.master));
            apvts.getParameter("INPUT")->setValueNotifyingHost(
                apvts.getParameter("INPUT")->convertTo0to1(p.input));
            apvts.getParameter("OUTPUT")->setValueNotifyingHost(
                apvts.getParameter("OUTPUT")->convertTo0to1(p.output));
            apvts.getParameter("LOWCUT")->setValueNotifyingHost(
                apvts.getParameter("LOWCUT")->convertTo0to1(p.lowcut));
            apvts.getParameter("HIGHCUT")->setValueNotifyingHost(
                apvts.getParameter("HIGHCUT")->convertTo0to1(p.highcut));
            apvts.getParameter("OD_DRIVE")->setValueNotifyingHost(
                apvts.getParameter("OD_DRIVE")->convertTo0to1(p.odDrive));
            apvts.getParameter("OD_TONE")->setValueNotifyingHost(
                apvts.getParameter("OD_TONE")->convertTo0to1(p.odTone));
            apvts.getParameter("OD_LEVEL")->setValueNotifyingHost(
                apvts.getParameter("OD_LEVEL")->convertTo0to1(p.odLevel));
            apvts.getParameter("CHO_RATE")->setValueNotifyingHost(
                apvts.getParameter("CHO_RATE")->convertTo0to1(p.choRate));
            apvts.getParameter("CHO_DEPTH")->setValueNotifyingHost(
                apvts.getParameter("CHO_DEPTH")->convertTo0to1(p.choDepth));
            apvts.getParameter("CHO_LEVEL")->setValueNotifyingHost(
                apvts.getParameter("CHO_LEVEL")->convertTo0to1(p.choLevel));
            apvts.getParameter("DEL_TIME")->setValueNotifyingHost(
                apvts.getParameter("DEL_TIME")->convertTo0to1(p.delTime));
            apvts.getParameter("DEL_FB")->setValueNotifyingHost(
                apvts.getParameter("DEL_FB")->convertTo0to1(p.delFb));
            apvts.getParameter("DEL_MIX")->setValueNotifyingHost(
                apvts.getParameter("DEL_MIX")->convertTo0to1(p.delMix));

            apvts.getParameter("OD_ENABLE")->setValueNotifyingHost(1.0f);
            apvts.getParameter("CHO_ENABLE")->setValueNotifyingHost(1.0f);
            apvts.getParameter("DEL_ENABLE")->setValueNotifyingHost(1.0f);

            float eqVals[6] = { p.eq1, p.eq2, p.eq3, p.eq4, p.eq5, p.eq6 };
            for (int i = 0; i < 6; ++i)
            {
                auto paramID = "EQ" + juce::String(i + 1);
                apvts.getParameter(paramID)->setValueNotifyingHost(
                    apvts.getParameter(paramID)->convertTo0to1(eqVals[i]));
            }

            apvts.getParameter("IR_GAIN")->setValueNotifyingHost(
                apvts.getParameter("IR_GAIN")->convertTo0to1(p.irGain));

            // Marca filtros para recalcular después de cargar preset
            filtersNeedUpdate.store(true);

            DBG("Preset cargado: " << p.nombre);
        });
}

// ===================================================
// RELEASE RESOURCES
// FIX #21: ya no está vacío — resetea todo el estado del DSP
// ===================================================
void MESABOOGIEINGSOFTAMPAudioProcessor::releaseResources()
{
    oversampler.reset();
    delayLine.reset();
    chorus.reset();
    resetAllFilters();
}

void MESABOOGIEINGSOFTAMPAudioProcessor::clearCabinetIR()
{
    irBypassed.store(true);
    irLoader.reset();
}

void MESABOOGIEINGSOFTAMPAudioProcessor::loadCabinetIR(const juce::File& file)
{
    irBypassed.store(false);
    irLoader.reset();
    irLoader.loadImpulseResponse(file,
        juce::dsp::Convolution::Stereo::yes,
        juce::dsp::Convolution::Trim::yes,
        0,
        juce::dsp::Convolution::Normalise::yes);
}

// ===================================================
// CREATE EDITOR / PLUGIN ENTRY POINT
// ===================================================
juce::AudioProcessorEditor* MESABOOGIEINGSOFTAMPAudioProcessor::createEditor()
{
    return new MESABOOGIEINGSOFTAMPAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MESABOOGIEINGSOFTAMPAudioProcessor();
}