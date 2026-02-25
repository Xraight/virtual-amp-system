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
    tunerBtn.onClick = [this] { repaint(); };
    addAndMakeVisible(tunerBtn);

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
        s.setName("master_io");
        addAndMakeVisible(s);

        l.setText(name, juce::dontSendNotification);
        l.setJustificationType(juce::Justification::centred);
        l.setColour(juce::Label::textColourId, juce::Colours::black.withAlpha(0.8f));
        l.setFont(juce::Font(13.0f, juce::Font::bold));
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
        addAndMakeVisible(s);
        };

    setupFilterSlider(lowCutSlider, " Hz");
    lowCutSlider.setRange(10.0, 200.0, 1.0);
    lowCutSlider.setValue(10.0);

    setupFilterSlider(highCutSlider, " Hz");
    highCutSlider.setRange(5000.0, 20000.0, 1.0);
    highCutSlider.setValue(20000.0);

    auto setupFXSlider = [this](juce::Slider& s, juce::Colour col) {
        s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 15);
        s.setNumDecimalPlacesToDisplay(0);
        s.setColour(juce::Slider::thumbColourId, col);
        addAndMakeVisible(s);
        };

    setupFXSlider(odDrive, juce::Colours::limegreen); setupFXSlider(odTone, juce::Colours::limegreen); setupFXSlider(odLevel, juce::Colours::limegreen);
    setupFXSlider(choRate, juce::Colours::cyan); setupFXSlider(choDepth, juce::Colours::cyan);
    setupFXSlider(delTime, juce::Colours::mediumpurple); setupFXSlider(delFb, juce::Colours::mediumpurple); setupFXSlider(delMix, juce::Colours::mediumpurple);

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

    auto setupTab = [this](juce::TextButton& b, int tabID, juce::String name) {
        b.setButtonText(name);
        b.setRadioGroupId(1);
        b.setClickingTogglesState(true);
        b.setColour(juce::TextButton::textColourOffId, juce::Colours::grey);
        b.setColour(juce::TextButton::textColourOnId, juce::Colours::orange);
        b.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
        b.onClick = [this, tabID] { currentTab = tabID; updateVisibility(); };
        addAndMakeVisible(b);
        };

    setupTab(ampTab, 0, "AMPLIFICADOR");
    setupTab(cabTab, 1, "GABINETE");
    setupTab(fxTab, 2, "EFECTOS");
    setupTab(eqTab, 3, "ECUALIZADOR");

    ampTab.setToggleState(true, juce::dontSendNotification);
    updateVisibility();
    setSize(1000, 550);
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
    choRate.setVisible(isFx); choDepth.setVisible(isFx);
    delTime.setVisible(isFx); delFb.setVisible(isFx); delMix.setVisible(isFx);
    eqBand1.setVisible(isEq); eqBand2.setVisible(isEq); eqBand3.setVisible(isEq);
    eqBand4.setVisible(isEq); eqBand5.setVisible(isEq); eqBand6.setVisible(isEq);

    repaint();
}

void MESABOOGIEINGSOFTAMPAudioProcessorEditor::paint(juce::Graphics& g)
{
    // --- FONDO ---
    g.fillAll(juce::Colours::black);

    // --- HEADER SUPERIOR ---
    g.setColour(juce::Colour(0xFF0A0A0A));
    g.fillRect(0, 0, getWidth(), 90);

    g.setColour(juce::Colours::orange);
    g.setFont(juce::Font(24.0f, juce::Font::bold));
    g.drawText("MESA/Boogie", 30, 20, 200, 30, juce::Justification::left);

    if (tunerBtn.getToggleState()) { return; }

    // --- CABEZAL  ---
    int margenX = 80;
    int margenY = 140;
    int margenAbajo = 50;

    auto areaMueble = juce::Rectangle<int>(margenX, margenY,
        getWidth() - (margenX * 2),
        getHeight() - margenY - margenAbajo);

    auto texturaPiel = juce::ImageCache::getFromMemory(BinaryData::PIEL_png, BinaryData::PIEL_pngSize);

    if (texturaPiel.isValid())
    {
        juce::FillType fillType(texturaPiel, juce::AffineTransform());
        g.setFillType(fillType);
        g.fillRoundedRectangle(areaMueble.toFloat(), 15.0f);

        g.setColour(juce::Colours::black.withAlpha(0.7f));
        g.drawRoundedRectangle(areaMueble.toFloat(), 15.0f, 3.0f);
    }

    // --- PANEL PRINCIPAL---
    auto panelArea = areaMueble.reduced(25, 30).toFloat();

    if (currentTab == 0) {
        g.setColour(juce::Colour(0xFFD4B483));
        g.fillRoundedRectangle(panelArea, 10.0f);

        g.setColour(juce::Colours::black.withAlpha(0.5f));
        g.drawRoundedRectangle(panelArea, 10.0f, 1.5f);

        // NOMBRE AMP
        g.setColour(juce::Colours::black.withAlpha(0.12f));
        g.setFont(juce::Font("Impact", 80.0f, juce::Font::italic));
        g.drawText("NoEraClean", panelArea.getX(), panelArea.getY() + 40, panelArea.getWidth(), 100, juce::Justification::centred);

        // ETIQUETAS AMPLI
        g.setColour(juce::Colours::black.withAlpha(0.8f));
        g.setFont(juce::Font("Arial", 11.0f, juce::Font::bold));

        int startX = (int)panelArea.getX() + 140;
        int knobW = ((int)panelArea.getWidth() - 280) / 6;
        juce::StringArray labels = { "GAIN", "BASS", "MID", "TREBLE", "PRESENCE", "MASTER" };
        for (int i = 0; i < 6; ++i) {
            g.drawText(labels[i], startX + (knobW * i), (int)panelArea.getBottom() - 35, knobW, 15, juce::Justification::centred);
        }
    }
    else {
        g.setColour(juce::Colour(0xFF151515));
        g.fillRoundedRectangle(panelArea, 10.0f);
        g.setColour(juce::Colours::orange.withAlpha(0.3f));
        g.drawRoundedRectangle(panelArea, 10.0f, 1.0f);
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
    ampTab.setBounds(tabsArea.removeFromLeft(tabW).reduced(5, 2));
    cabTab.setBounds(tabsArea.removeFromLeft(tabW).reduced(5, 2));
    fxTab.setBounds(tabsArea.removeFromLeft(tabW).reduced(5, 2));
    eqTab.setBounds(tabsArea.removeFromLeft(tabW).reduced(5, 2));

    int margenX = 80;
    int margenY = 140;
    int margenAbajo = 50;
    auto areaMueble = juce::Rectangle<int>(margenX, margenY, getWidth() - (margenX * 2), getHeight() - margenY - margenAbajo);
    auto panelArea = areaMueble.reduced(25, 30);

    // INPUT / OUTPUT
    inputSlider.setBounds(panelArea.getX() + 20, panelArea.getY() + 40, 70, 90);
    inputLabel.setBounds(panelArea.getX() + 20, panelArea.getY() + 20, 70, 20);

    outputSlider.setBounds(panelArea.getRight() - 90, panelArea.getY() + 40, 70, 90);
    outputLabel.setBounds(panelArea.getRight() - 90, panelArea.getY() + 20, 70, 20);

    // PERILLAS PRINCIPALES
    int startX = panelArea.getX() + 140;
    int knobW = (panelArea.getWidth() - 280) / 6;
    int knobY = panelArea.getBottom() - 130;

    gainSlider.setBounds(startX + (knobW * 0), knobY, knobW, 90);
    bassSlider.setBounds(startX + (knobW * 1), knobY, knobW, 90);
    midSlider.setBounds(startX + (knobW * 2), knobY, knobW, 90);
    trebleSlider.setBounds(startX + (knobW * 3), knobY, knobW, 90);
    presenceSlider.setBounds(startX + (knobW * 4), knobY, knobW, 90);
    masterSlider.setBounds(startX + (knobW * 5), knobY, knobW, 90);

    // GABINETE
    micSelector.setBounds(panelArea.getCentreX() - 100, panelArea.getY() + 40, 200, 30);
    lowCutSlider.setBounds(panelArea.getX() + 50, panelArea.getCentreY(), 100, 100);
    highCutSlider.setBounds(panelArea.getRight() - 150, panelArea.getCentreY(), 100, 100);

    // EFECTOS
    auto fxRow = panelArea.reduced(20);
    int fw = fxRow.getWidth() / 3;
    odDrive.setBounds(fxRow.getX(), fxRow.getY(), fw, 80);
    choRate.setBounds(fxRow.getX() + fw, fxRow.getY(), fw, 80);
    delTime.setBounds(fxRow.getX() + fw * 2, fxRow.getY(), fw, 80);

	// ECUALIZADOR
    int ew = panelArea.getWidth() / 6;
    eqBand1.setBounds(panelArea.getX() + (ew * 0), panelArea.getY() + 20, ew, panelArea.getHeight() - 60);
    eqBand2.setBounds(panelArea.getX() + (ew * 1), panelArea.getY() + 20, ew, panelArea.getHeight() - 60);
    eqBand3.setBounds(panelArea.getX() + (ew * 2), panelArea.getY() + 20, ew, panelArea.getHeight() - 60);
    eqBand4.setBounds(panelArea.getX() + (ew * 3), panelArea.getY() + 20, ew, panelArea.getHeight() - 60);
    eqBand5.setBounds(panelArea.getX() + (ew * 4), panelArea.getY() + 20, ew, panelArea.getHeight() - 60);
    eqBand6.setBounds(panelArea.getX() + (ew * 5), panelArea.getY() + 20, ew, panelArea.getHeight() - 60);
}