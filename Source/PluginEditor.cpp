#include "BinaryData.h"
#include "PluginProcessor.h"
#include "PluginEditor.h"

MESABOOGIEINGSOFTAMPAudioProcessorEditor::MESABOOGIEINGSOFTAMPAudioProcessorEditor(MESABOOGIEINGSOFTAMPAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    // --- PRESETS Y TUNER ---
    tunerBtn.setButtonText("TUNER");
    tunerBtn.setClickingTogglesState(true);
    tunerBtn.setLookAndFeel(&neuralLF);
    tunerBtn.onClick = [this]
        {
            updateVisibility();
            resized();
            repaint();
        };    addAndMakeVisible(tunerBtn);

    presetMenu.setTextWhenNothingSelected("Seleccionar Preset...");
    presetMenu.addItem("Clean Sparkle", 1);
    presetMenu.addItem("Crunchy Blues", 2);
    presetMenu.addItem("Mesa High Gain", 3);
    presetMenu.addItem("Liquid Lead", 4);
    presetMenu.onChange = [this] {
        // Lógica simple para cargar presets de fábrica basados en el índice
        int id = presetMenu.getSelectedId();
        auto& apvts = audioProcessor.apvts;
        if (id == 1) { // Clean
            apvts.getRawParameterValue("GAIN")->store(2.0f);
            apvts.getRawParameterValue("BASS")->store(4.0f);
            apvts.getRawParameterValue("MID")->store(5.0f);
            apvts.getRawParameterValue("TREBLE")->store(6.0f);
        } else if (id == 3) { // High Gain
            apvts.getRawParameterValue("GAIN")->store(8.5f);
            apvts.getRawParameterValue("BASS")->store(7.0f);
            apvts.getRawParameterValue("MID")->store(3.0f);
            apvts.getRawParameterValue("TREBLE")->store(8.0f);
        }
    };
    addAndMakeVisible(presetMenu);

    savePresetBtn.setButtonText("SAVE");
    savePresetBtn.onClick = [this] {
        fileChooser = std::make_unique<juce::FileChooser>("Guardar Preset",
            juce::File::getSpecialLocation(juce::File::userDocumentsDirectory),
            "*.xml");
        
        auto folderChooserFlags = juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles;
        fileChooser->launchAsync(folderChooserFlags, [this](const juce::FileChooser& fc) {
            auto file = fc.getResult();
            if (file != juce::File()) {
                juce::MemoryBlock destData;
                audioProcessor.getStateInformation(destData);
                file.replaceWithData(destData.getData(), destData.getSize());
            }
        });
    };
    addAndMakeVisible(savePresetBtn);

    prevPresetBtn.setButtonText("<");
    prevPresetBtn.onClick = [this] {
        int nextId = presetMenu.getSelectedId() - 1;
        if (nextId < 1) nextId = presetMenu.getNumItems();
        presetMenu.setSelectedId(nextId);
    };
    addAndMakeVisible(prevPresetBtn);

    nextPresetBtn.setButtonText(">");
    nextPresetBtn.onClick = [this] {
        int nextId = presetMenu.getSelectedId() + 1;
        if (nextId > presetMenu.getNumItems()) nextId = 1;
        presetMenu.setSelectedId(nextId);
    };
    addAndMakeVisible(nextPresetBtn);

    // --- INPUT / OUTPUT ---
    auto setupModernKnob = [this](juce::Slider& s, juce::Label& l, juce::String name) {
        s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        s.setRange(-20.0, 20.0, 0.1);
        s.setValue(0.0);
        s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 40, 15);
        s.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        s.setLookAndFeel(&neuralLF);
        s.setName("io");
        addAndMakeVisible(s);

        l.setText(name, juce::dontSendNotification);
        l.setJustificationType(juce::Justification::centred);
        l.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.7f));
        l.setFont(juce::Font(juce::FontOptions().withHeight(13.0f).withStyle("Bold")));
        addAndMakeVisible(l);
        };

    setupModernKnob(inputSlider, inputLabel, "INPUT");
    setupModernKnob(outputSlider, outputLabel, "OUTPUT");

    // --- CONFIGURACIÓN DE AMP ---
    auto setupAmpSlider = [this](juce::Slider& s) {
        s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        s.setLookAndFeel(&neuralLF);
        addAndMakeVisible(s);
        };

    setupAmpSlider(gainSlider);
    setupAmpSlider(bassSlider);
    setupAmpSlider(midSlider);
    setupAmpSlider(trebleSlider);
    setupAmpSlider(presenceSlider);
    setupAmpSlider(masterSlider);

    // --- 4. GABINETE / FX / EQ ---
    micSelector.addItem("Dynamic SM57", 1);
    micSelector.addItem("Condenser U87", 2);
    micSelector.addItem("Ribbon R121", 3);
    micSelector.addItem("MD421", 4);
    micSelector.setSelectedItemIndex(0);
    addAndMakeVisible(micSelector);

    auto setupFilterSlider = [this](juce::Slider& s, juce::String suffix) {
        s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
        s.setNumDecimalPlacesToDisplay(0);
        s.setTextValueSuffix(suffix);
        s.setColour(juce::Slider::thumbColourId, juce::Colours::orange);
        s.setLookAndFeel(&neuralLF);
        s.setName("noNumbers");
        addAndMakeVisible(s);
        };

    setupFilterSlider(lowCutSlider, " Hz");
    lowCutSlider.setRange(10.0, 200.0, 1.0);
    lowCutSlider.setValue(10.0);

    setupFilterSlider(highCutSlider, " Hz");
    highCutSlider.setRange(5000.0, 20000.0, 1.0);
    highCutSlider.setValue(20000.0);

    auto setupFXSlider = [this](juce::Slider& s, juce::Colour col)
        {
            s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
            s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
            s.setColour(juce::Slider::thumbColourId, col);
            s.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::black.withAlpha(0.3f));
            s.setLookAndFeel(&neuralLF);
            s.setName("fx");
            addAndMakeVisible(s);
        };

    setupFXSlider(odDrive, juce::Colours::green);
    setupFXSlider(odTone, juce::Colours::green);
    setupFXSlider(odLevel, juce::Colours::green);
    setupFXSlider(choRate, juce::Colours::blue);
    setupFXSlider(choDepth, juce::Colours::blue);
    setupFXSlider(choLevel, juce::Colours::blue);
    setupFXSlider(delTime, juce::Colours::orange);
    setupFXSlider(delFb, juce::Colours::orange);
    setupFXSlider(delMix, juce::Colours::orange);

    auto setupEQSlider = [this](juce::Slider& s) {
        s.setSliderStyle(juce::Slider::LinearVertical);
        s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
        s.setRange(-12.0, 12.0, 0.1);
        s.setValue(0.0);
        s.setColour(juce::Slider::thumbColourId, juce::Colours::white);
        s.setColour(juce::Slider::trackColourId, juce::Colours::orange.withAlpha(0.6f));
        s.setLookAndFeel(&neuralLF);
        addAndMakeVisible(s);
        };

    setupEQSlider(eqBand1);
    setupEQSlider(eqBand2);
    setupEQSlider(eqBand3);
    setupEQSlider(eqBand4);
    setupEQSlider(eqBand5);
    setupEQSlider(eqBand6);

    // --- 5. TABS ---
    auto setupTab = [this](juce::TextButton& b, int tabID, juce::String name)
        {
            b.setButtonText(name);
            b.setRadioGroupId(1);
            b.setClickingTogglesState(true);
            b.setLookAndFeel(&neuralLF);

            b.onClick = [this, tabID]()
                {
                    currentTab = tabID;
                    updateVisibility();
                    resized();   
                    repaint();
                };

            addAndMakeVisible(b);
        };

    setupTab(ampTab, 0, "AMPLIFICADOR");
    setupTab(cabTab, 1, "GABINETE");
    setupTab(fxTab, 2, "EFECTOS");
    setupTab(eqTab, 3, "ECUALIZADOR");

    ampTab.setToggleState(true, juce::sendNotification);
    currentTab = 0;

    // --- Noise Gate Controls ---
    // Sliders rotatorios pequeños que controlan el comportamiento del noise gate.
    auto setupNGSlider = [this](juce::Slider& s, double min, double max, double def) {
        s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        s.setRange(min, max, 0.1);
        s.setValue(def);
        s.setLookAndFeel(&neuralLF);
        addAndMakeVisible(s);
    };
    setupNGSlider(ngThreshSlider,  -96.0, 0.0,   -60.0);
    setupNGSlider(ngAttackSlider,    1.0, 100.0,    5.0);
    setupNGSlider(ngReleaseSlider,  10.0, 500.0,  200.0);

    // =========================================================
    // FASE 1: CONEXIONES UI → AUDIO (SliderAttachments)
    // Cada attachment sincroniza el slider con el parámetro del
    // AudioProcessorValueTreeState, permitiendo que los cambios
    // en la UI se reflejen en el procesamiento de audio en tiempo real.
    // =========================================================

    // --- Amp ---
    gainAtt      = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "GAIN",     gainSlider);
    bassAtt      = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "BASS",     bassSlider);
    midAtt       = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "MID",      midSlider);
    trebleAtt    = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "TREBLE",   trebleSlider);
    presenceAtt  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "PRESENCE", presenceSlider);
    masterAtt    = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "MASTER",   masterSlider);

    // --- Input / Output ---
    inputAtt     = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "INPUT",    inputSlider);
    outputAtt    = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "OUTPUT",   outputSlider);

    // --- Gabinete ---
    lowCutAtt    = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "LOWCUT",   lowCutSlider);
    highCutAtt   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "HIGHCUT",  highCutSlider);
    micAtt       = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.apvts, "MIC",    micSelector);

    // --- Overdrive ---
    odDriveAtt   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "OD_DRIVE", odDrive);
    odToneAtt    = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "OD_TONE",  odTone);
    odLevelAtt   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "OD_LEVEL", odLevel);

    // --- Chorus ---
    choRateAtt   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "CHO_RATE",  choRate);
    choDepthAtt  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "CHO_DEPTH", choDepth);
    choLevelAtt  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "CHO_LEVEL", choLevel);

    // --- Delay ---
    delTimeAtt   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "DEL_TIME", delTime);
    delFbAtt     = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "DEL_FB",   delFb);
    delMixAtt    = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "DEL_MIX",  delMix);

    // --- EQ 6 Bandas ---
    eqBand1Att   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "EQ1", eqBand1);
    eqBand2Att   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "EQ2", eqBand2);
    eqBand3Att   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "EQ3", eqBand3);
    eqBand4Att   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "EQ4", eqBand4);
    eqBand5Att   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "EQ5", eqBand5);
    eqBand6Att   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "EQ6", eqBand6);

    // --- Noise Gate ---
    ngThreshAtt  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "NG_THRESH",  ngThreshSlider);
    ngAttackAtt  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "NG_ATTACK",  ngAttackSlider);
    ngReleaseAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "NG_RELEASE", ngReleaseSlider);

    setSize(900, 600);
    updateVisibility();
    startTimerHz(24); // Fase 2: Timer para actualizar el Tuner a 24 fps
}

MESABOOGIEINGSOFTAMPAudioProcessorEditor::~MESABOOGIEINGSOFTAMPAudioProcessorEditor()
{
    stopTimer(); // Detener el timer del tuner al destruir el editor
}

// ============================================================
// FASE 2: TIMER CALLBACK — Actualización del Tuner en tiempo real
// Se ejecuta a 24fps para refrescar la pantalla del tuner con la
// frecuencia detectada por el algoritmo YIN en el processor.
// ============================================================
void MESABOOGIEINGSOFTAMPAudioProcessorEditor::timerCallback()
{
    if (tunerBtn.getToggleState())
        repaint(); // Solo repintar si el tuner está activo
}

void MESABOOGIEINGSOFTAMPAudioProcessorEditor::updateVisibility()
{
    bool isTuner = tunerBtn.getToggleState();
    bool isAmp = (currentTab == 0 && !isTuner);
    bool isCab = (currentTab == 1 && !isTuner);
    bool isFx = (currentTab == 2 && !isTuner);
    bool isEq = (currentTab == 3 && !isTuner);

    inputSlider.setVisible(isAmp); inputLabel.setVisible(isAmp);
    outputSlider.setVisible(isAmp); outputLabel.setVisible(isAmp);
    gainSlider.setVisible(isAmp); bassSlider.setVisible(isAmp); midSlider.setVisible(isAmp);
    trebleSlider.setVisible(isAmp); presenceSlider.setVisible(isAmp); masterSlider.setVisible(isAmp);

    micSelector.setVisible(isCab); lowCutSlider.setVisible(isCab); highCutSlider.setVisible(isCab);
    odDrive.setVisible(isFx); odTone.setVisible(isFx); odLevel.setVisible(isFx);
    choRate.setVisible(isFx); choDepth.setVisible(isFx); choLevel.setVisible(isFx);
    delTime.setVisible(isFx); delFb.setVisible(isFx); delMix.setVisible(isFx);
    eqBand1.setVisible(isEq); eqBand2.setVisible(isEq); eqBand3.setVisible(isEq);
    eqBand4.setVisible(isEq); eqBand5.setVisible(isEq); eqBand6.setVisible(isEq);

    // Noise Gate — visible en la pestaña de Amplificador
    ngThreshSlider.setVisible(isAmp);
    ngAttackSlider.setVisible(isAmp);
    ngReleaseSlider.setVisible(isAmp);

    for (auto* l : { &odDriveLabel, &odToneLabel, &odLevelLabel, &choRateLabel, &choDepthLabel, &choLevelLabel, &delTimeLabel, &delFbLabel, &delMixLabel, &inputLabel, &outputLabel })
        l->setVisible(false);

    repaint();
}

void MESABOOGIEINGSOFTAMPAudioProcessorEditor::paint(juce::Graphics& g)
{
    // --- 1. FONDO MESH GRADIENT COMPLEJO ---
    g.fillAll(juce::Colour(0xFF030308)); // Fondo base más oscuro

    // Capa 1: Púrpura Profundo (Top Left)
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xFF1A0A2E), 0, 0,
                                         juce::Colours::transparentBlack, getWidth() * 0.7f, getHeight() * 0.7f, true));
    g.fillAll();

    // Capa 2: Azul Eléctrico (Bottom Right)
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xFF081A3A), getWidth(), getHeight(),
                                         juce::Colours::transparentBlack, getWidth() * 0.5f, getHeight() * 0.5f, true));
    g.fillAll();
    
    // Capa 3: Brillo central sutil
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xFF102040).withAlpha(0.3f), getWidth() * 0.5f, getHeight() * 0.5f,
                                         juce::Colours::transparentBlack, getWidth() * 0.4f, getHeight() * 0.4f, true));
    g.fillAll();

    // --- 2. HEADER ---
    auto fullArea = getLocalBounds().toFloat();
    auto headerArea = fullArea.removeFromTop(90);
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(juce::FontOptions().withHeight(26.0f).withStyle("Bold")));
    g.drawText("VIRTUAL AMP SYSTEM", 40, 30, 400, 30, juce::Justification::left);
    
    g.setColour(juce::Colour(0xFFCCFF00).withAlpha(0.6f));
    g.setFont(juce::Font(juce::FontOptions().withHeight(11.0f).withStyle("Bold")));
    g.drawText("NEO AMPLIFIER v2.0", 40, 58, 400, 20, juce::Justification::left);

    // --- 3. CONTENIDO PRINCIPAL ---
    if (tunerBtn.getToggleState())
    {
        auto area = getLocalBounds().reduced(160, 120).toFloat();
        drawGlassPanel(g, area, 35.0f);

        // Leer la frecuencia detectada por el algoritmo YIN
        float freq = audioProcessor.detectedFrequency.load();

        // Convertir Hz a nota musical (A4 = 440Hz, escala temperada)
        juce::String noteName = "--";
        float centsOff = 0.0f;
        if (freq > 20.0f && freq < 5000.0f)
        {
            static const char* noteNames[] = { "C","C#","D","D#","E","F","F#","G","G#","A","A#","B" };
            float semitones = 12.0f * std::log2(freq / 440.0f) + 69.0f;
            int midiNote = juce::roundToInt(semitones);
            centsOff = (semitones - midiNote) * 100.0f;
            noteName = juce::String(noteNames[midiNote % 12]);
        }

        // Nota grande en el centro
        g.setColour(juce::Colour(0xFFCCFF00));
        g.setFont(juce::Font(juce::FontOptions().withHeight(120.0f).withStyle("Bold")));
        g.drawText(noteName, area.reduced(20, 40).toNearestInt(), juce::Justification::centred);

        // Frecuencia en Hz
        g.setColour(juce::Colours::white.withAlpha(0.7f));
        g.setFont(juce::Font(juce::FontOptions().withHeight(18.0f)));
        juce::String freqStr = freq > 20.0f ? juce::String(freq, 1) + " Hz" : "No signal";
        g.drawText(freqStr, area.removeFromBottom(80).reduced(30, 10).toNearestInt(), juce::Justification::centred);

        // Barra de afinación (cents)
        auto barArea = area.removeFromBottom(30).reduced(40, 5);
        float barW = barArea.getWidth();
        float barCenter = barArea.getCentreX();
        g.setColour(juce::Colours::white.withAlpha(0.15f));
        g.fillRoundedRectangle(barArea, 5.0f);
        // Indicador de posición
        float indicatorX = barCenter + (centsOff / 50.0f) * (barW * 0.5f);
        indicatorX = juce::jlimit(barArea.getX() + 5.0f, barArea.getRight() - 5.0f, indicatorX);
        auto indicatorColor = std::abs(centsOff) < 5.0f ? juce::Colour(0xFFCCFF00) : juce::Colours::orangered;
        g.setColour(indicatorColor);
        g.fillRoundedRectangle(indicatorX - 3, barArea.getY(), 6, barArea.getHeight(), 3.0f);
        // Línea central
        g.setColour(juce::Colours::white.withAlpha(0.5f));
        g.fillRect(barCenter - 1, barArea.getY(), 2.0f, barArea.getHeight());

        return;
    }

    // Área después de la barra lateral (140px de margen izquierdo para la sidebar)
    auto mainArea = getLocalBounds().withTrimmedTop(110).withTrimmedLeft(140).reduced(25).toFloat();

    if (currentTab == 0) // AMPLIFICADOR
    {
        auto gainArea = mainArea.removeFromLeft(mainArea.getWidth() * 0.45f).reduced(10);
        drawGlassPanel(g, gainArea, 25.0f);

        // --- Pantalla LED de Ganancia ---
        auto matrixArea = gainArea.removeFromTop(160).reduced(20);
        drawDotMatrixNumber(g, (float)gainSlider.getValue(), matrixArea, juce::Colour(0xFFCCFF00));
        g.setColour(juce::Colours::white.withAlpha(0.4f));
        g.setFont(juce::Font(juce::FontOptions().withHeight(11.0f).withStyle("Bold")));
        g.drawText("GAIN LEVEL", matrixArea.removeFromBottom(16).toNearestInt(), juce::Justification::centred);

        // --- Sección Noise Gate ---
        auto ngArea = gainArea.removeFromBottom(100).reduced(10);
        g.setColour(juce::Colours::white.withAlpha(0.15f));
        g.drawRoundedRectangle(ngArea.toFloat(), 8.0f, 1.0f);
        g.setColour(juce::Colour(0xFFCCFF00).withAlpha(0.7f));
        g.setFont(juce::Font(juce::FontOptions().withHeight(10.0f).withStyle("Bold")));
        g.drawText("NOISE GATE", ngArea.removeFromTop(18).toNearestInt(), juce::Justification::centred);
        int ngW = ngArea.getWidth() / 3;
        g.setColour(juce::Colours::white.withAlpha(0.5f));
        g.setFont(juce::Font(juce::FontOptions().withHeight(9.0f)));
        g.drawText("THRESH",  ngArea.removeFromLeft(ngW).removeFromBottom(12).toNearestInt(), juce::Justification::centred);
        g.drawText("ATTACK",  ngArea.removeFromLeft(ngW).removeFromBottom(12).toNearestInt(), juce::Justification::centred);
        g.drawText("RELEASE", ngArea.removeFromBottom(12).toNearestInt(), juce::Justification::centred);

        // --- Tone Shaping ---
        auto toneArea = mainArea.reduced(10);
        drawGlassPanel(g, toneArea, 25.0f);
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(juce::FontOptions().withHeight(18.0f).withStyle("Bold")));
        g.drawText("TONE SHAPING", toneArea.removeFromTop(40).reduced(20, 0).toNearestInt(), juce::Justification::left);
    }
    else if (currentTab == 1) // GABINETE
    {
        drawGlassPanel(g, mainArea, 25.0f);
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(juce::FontOptions().withHeight(18.0f).withStyle("Bold")));
        g.drawText("CABINET SIMULATION", mainArea.removeFromTop(40).reduced(20, 0).toNearestInt(), juce::Justification::left);
    }
    else if (currentTab == 2) // EFECTOS
    {
        float w = mainArea.getWidth() / 3.0f;
        auto effectArea = mainArea;
        
        for (int i = 0; i < 3; ++i)
        {
            auto panel = effectArea.removeFromLeft(w).reduced(10);
            drawGlassPanel(g, panel, 20.0f);
            
            g.setColour(juce::Colours::white);
            juce::String names[] = { "OVERDRIVE", "CHORUS", "DELAY" };
            g.drawText(names[i], panel.removeFromTop(35).toNearestInt(), juce::Justification::centred);
        }
    }
    else if (currentTab == 3) // ECUALIZADOR
    {
        drawGlassPanel(g, mainArea, 25.0f);
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(juce::FontOptions().withHeight(18.0f).withStyle("Bold")));
        g.drawText("MASTER EQUALIZER", mainArea.removeFromTop(40).reduced(20, 0).toNearestInt(), juce::Justification::left);
    }
}

void MESABOOGIEINGSOFTAMPAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    
    // Header
    auto headerArea = area.removeFromTop(100);
    tunerBtn.setBounds(330, 25, 80, 35);
    
    auto presetArea = headerArea.removeFromRight(450).reduced(20, 25);
    prevPresetBtn.setBounds(presetArea.removeFromLeft(35).reduced(2));
    presetMenu.setBounds(presetArea.removeFromLeft(200).reduced(5));
    nextPresetBtn.setBounds(presetArea.removeFromLeft(35).reduced(2));
    savePresetBtn.setBounds(presetArea.reduced(5));

    // Sidebar (Navegación lateral)
    auto sidebarArea = area.removeFromLeft(140).reduced(20, 40);
    int tabH = sidebarArea.getHeight() / 4;
    ampTab.setBounds(sidebarArea.removeFromTop(tabH).reduced(5, 10));
    cabTab.setBounds(sidebarArea.removeFromTop(tabH).reduced(5, 10));
    fxTab.setBounds(sidebarArea.removeFromTop(tabH).reduced(5, 10));
    eqTab.setBounds(sidebarArea.removeFromTop(tabH).reduced(5, 10));

    // Contenido Principal (Después de la sidebar)
    auto mainArea = area.reduced(25);

    if (currentTab == 0) // AMPLIFICADOR
    {
        auto gainCol = mainArea.removeFromLeft(mainArea.getWidth() * 0.45f).reduced(10);
        // Gain Slider debajo de la matriz
        gainSlider.setBounds(gainCol.getCentreX() - 80, gainCol.getY() + 160, 160, 160);
        
        // Noise Gate Sliders en la parte inferior de gainCol
        auto ngArea = gainCol.withTrimmedTop(gainCol.getHeight() - 100).reduced(10);
        int ngW = ngArea.getWidth() / 3;
        ngThreshSlider.setBounds(ngArea.removeFromLeft(ngW).reduced(5));
        ngAttackSlider.setBounds(ngArea.removeFromLeft(ngW).reduced(5));
        ngReleaseSlider.setBounds(ngArea.reduced(5));

        masterSlider.setBounds(gainCol.getCentreX() - 50, gainCol.getBottom() - 180, 100, 100);

        auto eqCol = mainArea.reduced(10);
        int knobSize = juce::jmin(eqCol.getWidth() / 2, eqCol.getHeight() / 2) - 20;
        int startX = eqCol.getX() + (eqCol.getWidth() - (knobSize * 2)) / 2;
        int startY = eqCol.getY() + (eqCol.getHeight() - (knobSize * 2)) / 2 + 30;
        
        bassSlider.setBounds(startX, startY, knobSize, knobSize);
        midSlider.setBounds(startX + knobSize, startY, knobSize, knobSize);
        trebleSlider.setBounds(startX, startY + knobSize, knobSize, knobSize);
        presenceSlider.setBounds(startX + knobSize, startY + knobSize, knobSize, knobSize);
    }
    else if (currentTab == 1) // GABINETE
    {
        int itemW = mainArea.getWidth() / 3;
        lowCutSlider.setBounds(mainArea.removeFromLeft(itemW).reduced(30));
        micSelector.setBounds(mainArea.removeFromLeft(itemW).reduced(20, mainArea.getHeight() * 0.4f));
        highCutSlider.setBounds(mainArea.reduced(30));
    }
    else if (currentTab == 2) // EFECTOS
    {
        int pedalW = mainArea.getWidth() / 3;
        auto setupFXLayout = [&](juce::Rectangle<int> r, juce::Slider& s1, juce::Slider& s2, juce::Slider& s3) {
            auto inner = r.reduced(20, 50);
            int h = inner.getHeight() / 3;
            s1.setBounds(inner.removeFromTop(h).reduced(15, 5));
            s2.setBounds(inner.removeFromTop(h).reduced(15, 5));
            s3.setBounds(inner.reduced(15, 5));
        };

        setupFXLayout(mainArea.removeFromLeft(pedalW), odDrive, odTone, odLevel);
        setupFXLayout(mainArea.removeFromLeft(pedalW), choRate, choDepth, choLevel);
        setupFXLayout(mainArea, delTime, delFb, delMix);
    }
    else if (currentTab == 3) // EQ
    {
        int bandW = mainArea.getWidth() / 6;
        eqBand1.setBounds(mainArea.removeFromLeft(bandW).reduced(12, 40));
        eqBand2.setBounds(mainArea.removeFromLeft(bandW).reduced(12, 40));
        eqBand3.setBounds(mainArea.removeFromLeft(bandW).reduced(12, 40));
        eqBand4.setBounds(mainArea.removeFromLeft(bandW).reduced(12, 40));
        eqBand5.setBounds(mainArea.removeFromLeft(bandW).reduced(12, 40));
        eqBand6.setBounds(mainArea.reduced(12, 40));
    }
}