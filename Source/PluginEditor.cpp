#include "BinaryData.h"
#include "PluginProcessor.h"
#include "PluginEditor.h"

MESABOOGIEINGSOFTAMPAudioProcessorEditor::MESABOOGIEINGSOFTAMPAudioProcessorEditor(MESABOOGIEINGSOFTAMPAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    juce::Colour mesaOrange = juce::Colour(0xFFFF9900);

    // --- PRESETS Y TUNER ---
    tunerBtn.setButtonText("TUNER");
    tunerBtn.setClickingTogglesState(true);
    tunerBtn.setColour(juce::TextButton::buttonOnColourId, juce::Colours::red.withAlpha(0.6f));
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
    addAndMakeVisible(presetMenu);

    savePresetBtn.setButtonText("SAVE");
    addAndMakeVisible(savePresetBtn);
    prevPresetBtn.setButtonText("<");
    nextPresetBtn.setButtonText(">");
    addAndMakeVisible(prevPresetBtn);
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
            s.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
            s.setNumDecimalPlacesToDisplay(0);
            s.setColour(juce::Slider::thumbColourId, col);
            s.setLookAndFeel(&neuralLF);
            s.setName("pedal");
            addAndMakeVisible(s);
        };


    setupFXSlider(odDrive, juce::Colours::limegreen); setupFXSlider(odTone, juce::Colours::limegreen); setupFXSlider(odLevel, juce::Colours::limegreen);
    setupFXSlider(choRate, juce::Colours::cyan); setupFXSlider(choDepth, juce::Colours::cyan); setupFXSlider(choLevel, juce::Colours::cyan);
    setupFXSlider(delTime, juce::Colours::mediumpurple); setupFXSlider(delFb, juce::Colours::mediumpurple); setupFXSlider(delMix, juce::Colours::mediumpurple);

    // ===== OVERDRIVE LABELS =====
    odDriveLabel.setText("DRIVE", juce::dontSendNotification);
    odToneLabel.setText("TONE", juce::dontSendNotification);
    odLevelLabel.setText("LEVEL", juce::dontSendNotification);

    for (auto* l : { &odDriveLabel, &odToneLabel, &odLevelLabel })
    {
        l->setJustificationType(juce::Justification::centred);
        l->setColour(juce::Label::textColourId, juce::Colours::white);
        addAndMakeVisible(l);
    }

    // ===== CHORUS LABELS =====
    choRateLabel.setText("RATE", juce::dontSendNotification);
    choDepthLabel.setText("DEPTH", juce::dontSendNotification);
    choLevelLabel.setText("LEVEL", juce::dontSendNotification);

    for (auto* l : { &choRateLabel, &choDepthLabel, &choLevelLabel })
    {
        l->setJustificationType(juce::Justification::centred);
        l->setColour(juce::Label::textColourId, juce::Colours::white);
        addAndMakeVisible(l);
    }

    // ===== DELAY LABELS =====
    delTimeLabel.setText("TIME", juce::dontSendNotification);
    delFbLabel.setText("FEEDBACK", juce::dontSendNotification);
    delMixLabel.setText("MIX", juce::dontSendNotification);

    for (auto* l : { &delTimeLabel, &delFbLabel, &delMixLabel })
    {
        l->setJustificationType(juce::Justification::centred);
        l->setColour(juce::Label::textColourId, juce::Colours::white);
        addAndMakeVisible(l);
    }

    auto setupEQSlider = [this](juce::Slider& s) {
        s.setSliderStyle(juce::Slider::LinearVertical);
        s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
        s.setRange(-12.0, 12.0, 0.1);
        s.setValue(0.0);
        s.setColour(juce::Slider::trackColourId, juce::Colours::orange);
        s.setColour(juce::Slider::thumbColourId, juce::Colours::white);
        addAndMakeVisible(s);
        };

    setupEQSlider(eqBand1); setupEQSlider(eqBand2); setupEQSlider(eqBand3);
    setupEQSlider(eqBand4); setupEQSlider(eqBand5); setupEQSlider(eqBand6);

    auto setupTab = [this](juce::TextButton& b, int tabID, juce::String name)
        {
            b.setButtonText(name);
            b.setRadioGroupId(1);
            b.setClickingTogglesState(true);
            b.setColour(juce::TextButton::textColourOffId, juce::Colours::grey);
            b.setColour(juce::TextButton::textColourOnId, juce::Colours::orange);
            b.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);

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

    ampTab.setToggleState(true, juce::dontSendNotification);
    updateVisibility();

    auto& apvts = audioProcessor.apvts;

    gainAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "GAIN", gainSlider);
    bassAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "BASS", bassSlider);
    midAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "MID", midSlider);
    trebleAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "TREBLE", trebleSlider);
    presenceAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "PRESENCE", presenceSlider);
    masterAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "MASTER", masterSlider);
    inputAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "INPUT", inputSlider);
    outputAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "OUTPUT", outputSlider);
    lowCutAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "LOWCUT", lowCutSlider);
    highCutAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "HIGHCUT", highCutSlider);
    odDriveAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "OD_DRIVE", odDrive);
    odToneAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "OD_TONE", odTone);
    odLevelAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "OD_LEVEL", odLevel);
    choRateAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "CHO_RATE", choRate);
    choDepthAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "CHO_DEPTH", choDepth);
    choLevelAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "CHO_LEVEL", choLevel);
    delTimeAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "DEL_TIME", delTime);
    delFbAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "DEL_FB", delFb);
    delMixAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "DEL_MIX", delMix);
    eqBand1Att = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "EQ1", eqBand1);
    eqBand2Att = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "EQ2", eqBand2);
    eqBand3Att = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "EQ3", eqBand3);
    eqBand4Att = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "EQ4", eqBand4);
    eqBand5Att = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "EQ5", eqBand5);
    eqBand6Att = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "EQ6", eqBand6);
    micAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, "MIC", micSelector);


    setSize(1000, 700);
}

MESABOOGIEINGSOFTAMPAudioProcessorEditor::~MESABOOGIEINGSOFTAMPAudioProcessorEditor()
{
    gainSlider.setLookAndFeel(nullptr);
    bassSlider.setLookAndFeel(nullptr);
    midSlider.setLookAndFeel(nullptr);
    trebleSlider.setLookAndFeel(nullptr);
    presenceSlider.setLookAndFeel(nullptr);
    masterSlider.setLookAndFeel(nullptr);
    inputSlider.setLookAndFeel(nullptr);
    outputSlider.setLookAndFeel(nullptr);
    lowCutSlider.setLookAndFeel(nullptr);
    highCutSlider.setLookAndFeel(nullptr);
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

    odDriveLabel.setVisible(isFx);
    odToneLabel.setVisible(isFx);
    odLevelLabel.setVisible(isFx);

    choRateLabel.setVisible(isFx);
    choDepthLabel.setVisible(isFx);
    choLevelLabel.setVisible(isFx);

    delTimeLabel.setVisible(isFx);
    delFbLabel.setVisible(isFx);
    delMixLabel.setVisible(isFx);

    repaint();
}

void MESABOOGIEINGSOFTAMPAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xFF0A0A0A));

    g.setColour(juce::Colours::black);
    g.fillRect(0, 0, getWidth(), 90);

    g.setColour(juce::Colours::orange);
    g.setFont(juce::Font(juce::FontOptions().withHeight(24.0f).withStyle("Bold")));
    g.drawText("NoEraClean AMP", 30, 20, 200, 30, juce::Justification::left);

    if (tunerBtn.getToggleState())
    {
        g.fillAll(juce::Colour(0xFF050505));

        auto area = getLocalBounds().reduced(200, 180).toFloat();

        g.setColour(juce::Colour(0xFF151515));
        g.fillRoundedRectangle(area, 20.0f);

        g.setColour(juce::Colours::orange.withAlpha(0.4f));
        g.drawRoundedRectangle(area, 20.0f, 2.0f);

        g.setColour(juce::Colours::orange);
        g.setFont(juce::Font(juce::FontOptions().withHeight(80.0f).withStyle("Bold")));
        g.drawText("A",
            area.toNearestInt(),
            juce::Justification::centred);

        g.setFont(juce::Font(juce::FontOptions().withHeight(20.0f)));
        g.drawText("440 Hz",
            area.removeFromBottom(40).toNearestInt(),
            juce::Justification::centred);

        return;
    }
    if (currentTab == 1) // --- PESTAÑA GABINETE ACTUALIZADA ---
    {
        auto areaGabinete = getLocalBounds().reduced(60, 160).withTrimmedTop(-20).toFloat();

        g.setColour(juce::Colour(0xFF151515));
        g.fillRoundedRectangle(areaGabinete, 15.0f);
        g.setColour(juce::Colours::white.withAlpha(0.05f));
        g.drawRoundedRectangle(areaGabinete, 15.0f, 2.0f);

        auto drawSpeaker = [&](juce::Rectangle<float> r) {
            g.setColour(juce::Colours::black.withAlpha(0.6f));
            g.fillEllipse(r);
            g.setColour(juce::Colour(0xFF1A1A1A));
            g.fillEllipse(r.reduced(r.getWidth() * 0.05f));
            g.setColour(juce::Colours::black);
            g.fillEllipse(r.reduced(r.getWidth() * 0.35f));
            g.setColour(juce::Colours::white.withAlpha(0.03f));
            g.drawEllipse(r.reduced(r.getWidth() * 0.1f), 2.0f);
            };

        float speakerSize = 220.0f;
        float yPos = areaGabinete.getY() + 40;
        drawSpeaker(juce::Rectangle<float>(areaGabinete.getX() + 60, yPos, speakerSize, speakerSize));
        drawSpeaker(juce::Rectangle<float>(areaGabinete.getRight() - 60 - speakerSize, yPos, speakerSize, speakerSize));

        g.setColour(juce::Colours::white.withAlpha(0.7f));
        g.setFont(juce::Font(juce::FontOptions().withHeight(14.0f).withStyle("Bold")));
        g.drawText("LOW CUT", lowCutSlider.getX(), lowCutSlider.getY() - 25, lowCutSlider.getWidth(), 20, juce::Justification::centred);
        g.drawText("HIGH CUT", highCutSlider.getX(), highCutSlider.getY() - 25, highCutSlider.getWidth(), 20, juce::Justification::centred);
        g.drawText("MICROPHONE SELECT", micSelector.getX(), micSelector.getY() - 25, micSelector.getWidth(), 20, juce::Justification::centred);
    }
    else if (currentTab == 2) // ===== EFECTOS =====
    {
        // Fondo completo independiente
        g.fillAll(juce::Colour(0xFF101010));

        juce::ColourGradient bg(
            juce::Colour(0xFF181818),
            getWidth() / 2, 0,
            juce::Colour(0xFF080808),
            getWidth() / 2, getHeight(),
            false);

        g.setGradientFill(bg);
        g.fillRect(getLocalBounds());

        // ===== PEDALES =====

        int pedalWidth = 180;
        int pedalHeight = 260;
        int spacing = 40;

        int totalWidth = pedalWidth * 3 + spacing * 2;
        int startX = (getWidth() - totalWidth) / 2;
        int y = (getHeight() - pedalHeight) / 2;

        juce::Rectangle<int> odArea(startX, y, pedalWidth, pedalHeight);
        juce::Rectangle<int> chArea(startX + pedalWidth + spacing, y, pedalWidth, pedalHeight);
        juce::Rectangle<int> dlArea(startX + (pedalWidth + spacing) * 2, y, pedalWidth, pedalHeight);


        // ===== FUNCION PARA DIBUJAR PEDAL REALISTA =====
        auto drawPedal = [&](juce::Rectangle<int> area,
            juce::Colour color,
            juce::String name,
            juce::Colour ledColor)
            {
                auto r = area.toFloat();

                g.setColour(juce::Colours::black.withAlpha(0.5f));
                g.fillRoundedRectangle(r.translated(4.0f, 6.0f), 12.0f);

                juce::ColourGradient grad(
                    color.brighter(0.4f),
                    r.getCentreX(), r.getY(),
                    color.darker(0.6f),
                    r.getCentreX(), r.getBottom(),
                    false);

                g.setGradientFill(grad);
                g.fillRoundedRectangle(r, 12);

                g.setColour(color.brighter(0.7f));
                g.drawRoundedRectangle(r, 12, 2);

                int screwSize = 10;

                g.setColour(juce::Colours::silver);

                g.fillEllipse(r.getX() + 8, r.getY() + 8, screwSize, screwSize);
                g.fillEllipse(r.getRight() - 18, r.getY() + 8, screwSize, screwSize);
                g.fillEllipse(r.getX() + 8, r.getBottom() - 18, screwSize, screwSize);
                g.fillEllipse(r.getRight() - 18, r.getBottom() - 18, screwSize, screwSize);

                g.setColour(ledColor);
                g.fillEllipse(r.getCentreX() - 6, r.getY() + 12, 12, 12);

                g.setColour(juce::Colours::white);
                g.setFont(16);
                g.drawFittedText(name,
                    area.removeFromTop(40),
                    juce::Justification::centred,
                    1);

                g.setColour(juce::Colours::silver);
                g.fillEllipse(r.getCentreX() - 14.0f, r.getBottom() - 40.0f, 28.0f, 28.0f);

                g.setColour(juce::Colours::black);
                g.drawEllipse(r.getCentreX() - 14.0f, r.getBottom() - 40.0f, 28.0f, 28.0f, 2.0f);
            };


        // ===== DIBUJAR PEDALES =====

        drawPedal(odArea, juce::Colour(0xFF228B22), "OVERDRIVE", juce::Colours::lime);

        drawPedal(chArea, juce::Colour(0xFF1E90FF), "CHORUS", juce::Colours::cyan);

        drawPedal(dlArea, juce::Colour(0xFFD2691E), "DELAY", juce::Colours::orange);
    }
    else if (currentTab == 0) // --- SOLO AMPLIFICADOR ---
    {
        auto areaGabinete = juce::Rectangle<float>(0, getHeight() * 0.65f, getWidth(), getHeight() * 0.35f);
        g.setColour(juce::Colour(0xFF0F0F0F));
        g.fillRect(areaGabinete);

        auto rejillaArea = areaGabinete.reduced(50, 0).withTrimmedTop(15);
        auto frame = rejillaArea.expanded(20);

        juce::ColourGradient frameGradient(
            juce::Colour(0xFF101010),
            frame.getTopLeft(),
            juce::Colour(0xFF000000),
            frame.getBottomLeft(),
            false);
        g.setGradientFill(frameGradient);
        g.fillRoundedRectangle(frame.toFloat(), 8.0f);

        juce::ColourGradient cabGradient(
            juce::Colour(0xFF3A2E22),
            rejillaArea.getTopLeft(),
            juce::Colour(0xFF1F1812),
            rejillaArea.getBottomLeft(),
            false);
        g.setGradientFill(cabGradient);
        g.fillRoundedRectangle(rejillaArea.toFloat(), 6.0f);

        for (int y = rejillaArea.getY(); y < rejillaArea.getBottom(); y += 4)
        {
            g.setColour(juce::Colours::black.withAlpha(0.45f));
            g.drawLine((float)rejillaArea.getX(),
                (float)y,
                (float)rejillaArea.getRight(),
                (float)y);
        }

        for (int x = rejillaArea.getX(); x < rejillaArea.getRight(); x += 4)
        {
            g.setColour(juce::Colours::black.withAlpha(0.20f));
            g.drawLine((float)x,
                (float)rejillaArea.getY(),
                (float)x,
                (float)rejillaArea.getBottom());
        }

        g.setColour(juce::Colours::silver.withAlpha(0.6f));
        g.setFont(juce::Font(juce::FontOptions().withName("Impact").withHeight(40.0f)));
        g.drawText("CAB", rejillaArea.withHeight(100).translated(0, 30), juce::Justification::centred);

        if (currentTab == 2)
        {
            auto drawPedal = [&](juce::Rectangle<int> area, juce::String name, juce::Colour colour)
                {
                    g.setColour(colour.withAlpha(1.0f)); 
                    g.fillRoundedRectangle(area.toFloat(), 12);

                    g.setColour(juce::Colours::black);
                    g.drawRoundedRectangle(area.toFloat(), 10, 2);

                    g.setColour(juce::Colours::white);
                    g.setFont(16);
                    g.drawText(name, area.getX(), area.getY() + 10, area.getWidth(), 20, juce::Justification::centred);
                };

            int pedalWidth = 180;
            int pedalHeight = 220;
            int spacing = 40;

            int totalWidth = pedalWidth * 3 + spacing * 2;
            int startX = (getWidth() - totalWidth) / 2;
            int y = getHeight() * 0.35f;

            drawPedal({ startX, y, pedalWidth, pedalHeight }, "OVERDRIVE", juce::Colours::darkgreen);
            drawPedal({ startX + pedalWidth + spacing, y, pedalWidth, pedalHeight }, "CHORUS", juce::Colours::darkblue);
            drawPedal({ startX + (pedalWidth + spacing) * 2, y, pedalWidth, pedalHeight }, "DELAY", juce::Colours::darkred);
        }

        int headW = getWidth() * 0.75f;
        int headH = 280;
        int headY = (getHeight() * 0.65f) - headH + 5;
        auto areaMueble = juce::Rectangle<float>((getWidth() - headW) / 2.0f, (float)headY, (float)headW, (float)headH);

        g.setColour(juce::Colours::black.withAlpha(0.5f));
        g.fillRoundedRectangle(areaMueble.translated(0, 5), 12.0f);

        auto texturaPiel = juce::ImageCache::getFromMemory(BinaryData::PIEL_png, BinaryData::PIEL_pngSize);
        if (texturaPiel.isValid()) {
            g.setTiledImageFill(texturaPiel, 0, 0, 0.4f);
            g.fillRoundedRectangle(areaMueble, 12.0f);
            g.setColour(juce::Colours::black.withAlpha(0.8f));
            g.drawRoundedRectangle(areaMueble, 12.0f, 3.0f);
        }

        auto panelArea = areaMueble.reduced(25, 35);
        if (currentTab == 0) {
            g.setColour(juce::Colour(0xFFD4B483));
            g.fillRoundedRectangle(panelArea, 8.0f);
            g.setColour(juce::Colours::black.withAlpha(0.15f));
            g.setFont(juce::Font(juce::FontOptions().withName("Brush Script MT").withHeight(65.0f).withStyle("Italic")));
            g.drawText("NoEraClean", panelArea, juce::Justification::centredTop);
            g.setColour(juce::Colours::black.withAlpha(0.7f));
            g.setFont(juce::Font(juce::FontOptions().withName("Helvetica").withHeight(11.0f).withStyle("Bold")));
            int startX = (int)panelArea.getX() + 110;
            int knobW = ((int)panelArea.getWidth() - 220) / 6;
            juce::StringArray labels = { "GAIN", "BASS", "MID", "TREBLE", "PRESENCE", "MASTER" };
            for (int i = 0; i < 6; ++i) {
                g.drawText(labels[i], startX + (knobW * i), (int)panelArea.getBottom() - 32, knobW, 15, juce::Justification::centred);
            }
        }
    }

else if (currentTab == 3) // ===== ECUALIZADOR =====
{
    g.fillAll(juce::Colour(0xFF0B0B0B));

    auto eqArea = getLocalBounds().reduced(100, 140).toFloat();

    // ===== PANEL PRINCIPAL =====
    g.setColour(juce::Colour(0xFF1A1A1A));
    g.fillRoundedRectangle(eqArea, 20.0f);

    g.setColour(juce::Colours::orange.withAlpha(0.4f));
    g.drawRoundedRectangle(eqArea, 20.0f, 2.0f);

    // ===== dB GRID =====
    g.setColour(juce::Colours::orange.withAlpha(0.15f));

    auto gridArea = eqArea;
    gridArea.removeFromTop(80);
    gridArea.removeFromBottom(60);

    float centerY = gridArea.getCentreY();

    // Línea 0 dB (más fuerte)
    g.setColour(juce::Colours::orange.withAlpha(0.4f));
    g.drawLine(gridArea.getX(), centerY,
        gridArea.getRight(), centerY, 2.0f);

    // Líneas adicionales
    g.setColour(juce::Colours::orange.withAlpha(0.15f));

    float step = gridArea.getHeight() / 4.0f;

    for (int i = 1; i <= 2; ++i)
    {
        g.drawLine(gridArea.getX(),
            centerY - step * i,
            gridArea.getRight(),
            centerY - step * i,
            1.0f);

        g.drawLine(gridArea.getX(),
            centerY + step * i,
            gridArea.getRight(),
            centerY + step * i,
            1.0f);
    }

    // ===== dB LABELS =====
    g.setFont(juce::Font(juce::FontOptions().withHeight(14.0f)));
    g.setColour(juce::Colours::orange.withAlpha(0.6f));

    juce::String labels[] = { "+12db", "+6db", "0", "-6db", "-12db" };

    for (int i = 0; i < 5; ++i)
    {
        float y = gridArea.getY() + (gridArea.getHeight() / 4.0f) * i;

        g.drawText(labels[i],
            (int)(gridArea.getX()) - 50,
            (int)(y - 10.0f),
            40,
            20,
            juce::Justification::centredRight);
    }

    // ===== TITULO =====
    g.setColour(juce::Colours::orange);
    g.setFont(juce::Font(juce::FontOptions().withHeight(26.0f).withStyle("Bold")));
    g.drawText("ECUALIZADOR 6 BANDAS",
        eqArea.removeFromTop(60).toNearestInt(),
        juce::Justification::centred);



    auto graphArea = eqArea.reduced(40, 20);

    // ===== FRECUENCIAS =====
    g.setColour(juce::Colours::white.withAlpha(0.8f));
    g.setFont(juce::Font(14.0f));

    juce::String freqs[6] = { "100 Hz", "200 Hz", "400 Hz", "800 Hz", "1.6 kHz", "3.2 kHz" };

    float bandWidth = graphArea.getWidth() / 6.0f;

    for (int i = 0; i < 6; ++i)
    {
        float x = graphArea.getX() + bandWidth * i;

        juce::Rectangle<float> textArea(x,
            graphArea.getBottom() - 20,
            bandWidth,
            20);

        g.drawText(freqs[i],
            textArea.toNearestInt(),
            juce::Justification::centred);
    }
}

}

void MESABOOGIEINGSOFTAMPAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    auto headerArea = area.removeFromTop(80);
    tunerBtn.setBounds(230, 25, 80, 30);

    auto presetArea = headerArea.removeFromRight(450).reduced(20, 25);
    savePresetBtn.setBounds(presetArea.removeFromRight(60));
    nextPresetBtn.setBounds(presetArea.removeFromRight(30));
    presetMenu.setBounds(presetArea.removeFromRight(180).reduced(5, 0));
    prevPresetBtn.setBounds(presetArea.removeFromRight(30));

    auto tabsArea = area.removeFromTop(40);
    int tabW = tabsArea.getWidth() / 4;
    ampTab.setBounds(tabsArea.removeFromLeft(tabW).reduced(15, 5));
    cabTab.setBounds(tabsArea.removeFromLeft(tabW).reduced(15, 5));
    fxTab.setBounds(tabsArea.removeFromLeft(tabW).reduced(15, 5));
    eqTab.setBounds(tabsArea.removeFromLeft(tabW).reduced(15, 5));

    if (currentTab == 1) // --- GABINETE LAYOUT ---
    {
        auto areaTotal = getLocalBounds();
        int cX = getWidth() / 2;
        int bottomY = areaTotal.getBottom() - 180;

        // 1. Bloque Central
        int centerW = 260;
        micSelector.setBounds(cX - (centerW / 2), bottomY, centerW, 35);

        savePresetBtn.setBounds(cX - (centerW / 2), bottomY + 45, centerW, 50);
        savePresetBtn.setButtonText("CARGAR IR (.WAV)");
        savePresetBtn.setColour(juce::TextButton::buttonColourId, juce::Colours::orange);
        savePresetBtn.setColour(juce::TextButton::textColourOffId, juce::Colours::black);

        // 2. Perillas laterales (Low y High Cut) flanqueando el bloque central
        int knobSize = 100;
        lowCutSlider.setBounds(cX - (centerW / 2) - knobSize - 40, bottomY - 10, knobSize, knobSize + 20);
        highCutSlider.setBounds(cX + (centerW / 2) + 40, bottomY - 10, knobSize, knobSize + 20);

        savePresetBtn.onClick = [this] {
            fileChooser = std::make_unique<juce::FileChooser>("Selecciona un Impulse Response...",
                juce::File::getSpecialLocation(juce::File::userHomeDirectory),
                "*.wav");
            auto flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;
            fileChooser->launchAsync(flags, [this](const juce::FileChooser& fc) {
                auto file = fc.getResult();
                if (file.existsAsFile()) {
                    // audioProcessor.loadIR(file); 
                }
                });
            };
    }

    else if (currentTab == 2) // ===== EFECTOS TAB =====
    {
        int pedalWidth = 180;
        int pedalHeight = 220;
        int spacing = 40;

        int totalWidth = pedalWidth * 3 + spacing * 2;
        int startX = (getWidth() - totalWidth) / 2;
        int y = getHeight() * 0.35f;

        // ================= OVERDRIVE =================
        juce::Rectangle<int> odArea(startX, y, pedalWidth, pedalHeight);

        odDrive.setBounds(odArea.getX() + 20, odArea.getY() + 50, 60, 60);
        odTone.setBounds(odArea.getX() + 100, odArea.getY() + 50, 60, 60);
        odLevel.setBounds(odArea.getX() + 60, odArea.getY() + 130, 60, 60);

        odDriveLabel.setBounds(odDrive.getX(), odDrive.getBottom() - 5, odDrive.getWidth(), 20);
        odToneLabel.setBounds(odTone.getX(), odTone.getBottom() - 5, odTone.getWidth(), 20);
        odLevelLabel.setBounds(odLevel.getX(), odLevel.getY() + odLevel.getHeight() / 2 + 15, odLevel.getWidth(), 20);

        // ================= CHORUS =================
        juce::Rectangle<int> choArea(startX + pedalWidth + spacing, y, pedalWidth, pedalHeight);

        choRate.setBounds(choArea.getX() + 20, choArea.getY() + 50, 60, 60);
        choDepth.setBounds(choArea.getX() + 100, choArea.getY() + 50, 60, 60);
        choLevel.setBounds(choArea.getX() + 60, choArea.getY() + 130, 60, 60);

        choRateLabel.setBounds(choRate.getX(), choRate.getBottom() - 5, choRate.getWidth(), 20);
        choDepthLabel.setBounds(choDepth.getX(), choDepth.getBottom() - 5, choDepth.getWidth(), 20);
        choLevelLabel.setBounds(choLevel.getX(), choLevel.getY() + choLevel.getHeight() / 2 + 15, choLevel.getWidth(), 20);

        // ================= DELAY =================
        juce::Rectangle<int> delArea(startX + (pedalWidth + spacing) * 2, y, pedalWidth, pedalHeight);

        delTime.setBounds(delArea.getX() + 20, delArea.getY() + 50, 60, 60);
        delFb.setBounds(delArea.getX() + 100, delArea.getY() + 50, 60, 60);
        delMix.setBounds(delArea.getX() + 60, delArea.getY() + 130, 60, 60);

        delTimeLabel.setBounds(delTime.getX(), delTime.getBottom() - 5, delTime.getWidth(), 20);
        delFbLabel.setBounds(delFb.getX(), delFb.getBottom() - 5, delFb.getWidth(), 20);
        delMixLabel.setBounds(delMix.getX(), delMix.getY() + delMix.getHeight() / 2 + 15, delMix.getWidth(), 20);
    }

    else if (currentTab == 3) // ===== EQ TAB =====
    {
        auto eqArea = getLocalBounds().reduced(100, 140);

        auto sliderArea = eqArea;
        sliderArea.removeFromTop(80);     // espacio título
        sliderArea.removeFromBottom(60);  // espacio frecuencias

        int bandWidth = sliderArea.getWidth() / 6;

        eqBand1.setBounds(sliderArea.getX() + bandWidth * 0, sliderArea.getY(), bandWidth, sliderArea.getHeight());
        eqBand2.setBounds(sliderArea.getX() + bandWidth * 1, sliderArea.getY(), bandWidth, sliderArea.getHeight());
        eqBand3.setBounds(sliderArea.getX() + bandWidth * 2, sliderArea.getY(), bandWidth, sliderArea.getHeight());
        eqBand4.setBounds(sliderArea.getX() + bandWidth * 3, sliderArea.getY(), bandWidth, sliderArea.getHeight());
        eqBand5.setBounds(sliderArea.getX() + bandWidth * 4, sliderArea.getY(), bandWidth, sliderArea.getHeight());
        eqBand6.setBounds(sliderArea.getX() + bandWidth * 5, sliderArea.getY(), bandWidth, sliderArea.getHeight());
    }

    else
    {
        // Restaurar botón para presets original
        savePresetBtn.setButtonText("SAVE");
        savePresetBtn.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
        savePresetBtn.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        savePresetBtn.onClick = nullptr;

        int headW = getWidth() * 0.75f;
        int headH = 280;
        int headY = (getHeight() * 0.65f) - headH + 5;
        auto areaMueble = juce::Rectangle<int>((getWidth() - headW) / 2, headY, headW, headH);
        auto panelArea = areaMueble.reduced(25, 35);

        int ioY = 140; 
        inputSlider.setBounds(30, ioY, 70, 90);
        inputLabel.setBounds(30, ioY - 18, 70, 20);
        outputSlider.setBounds(getWidth() - 110, ioY, 70, 90);
        outputLabel.setBounds(getWidth() - 110, ioY - 18, 70, 20);

        int startX = panelArea.getX() + 110;
        int knobW = (panelArea.getWidth() - 220) / 6;
        int knobY = panelArea.getBottom() - 130;

        gainSlider.setBounds(startX + (knobW * 0), knobY, knobW, 100);
        bassSlider.setBounds(startX + (knobW * 1), knobY, knobW, 100);
        midSlider.setBounds(startX + (knobW * 2), knobY, knobW, 100);
        trebleSlider.setBounds(startX + (knobW * 3), knobY, knobW, 100);
        presenceSlider.setBounds(startX + (knobW * 4), knobY, knobW, 100);
        masterSlider.setBounds(startX + (knobW * 5), knobY, knobW, 100);

        auto fxRow = panelArea.reduced(20);
        int fw = fxRow.getWidth() / 3;
        odDrive.setBounds(fxRow.getX(), fxRow.getY(), fw, 80);
        choRate.setBounds(fxRow.getX() + fw, fxRow.getY(), fw, 80);
        delTime.setBounds(fxRow.getX() + fw * 2, fxRow.getY(), fw, 80);
    }
}