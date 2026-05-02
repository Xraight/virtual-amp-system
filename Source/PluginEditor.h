#include <JuceHeader.h>
#include "PluginProcessor.h"
#include <cmath>
#include "BinaryData.h"

// --- CLASE DE DISEÑO DEL AMP ---
class NeuralLookAndFeel : public juce::LookAndFeel_V4
{
public:
    NeuralLookAndFeel()
    {
        setColour(juce::Slider::thumbColourId, juce::Colour(0xFFCCFF00)); // Neón lima
        setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
        setColour(juce::TextButton::buttonOnColourId, juce::Colour(0x33FFFFFF));
        setColour(juce::TextButton::textColourOffId, juce::Colours::white.withAlpha(0.7f));
        setColour(juce::TextButton::textColourOnId, juce::Colours::white);
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
        float sliderPos, const float rotaryStartAngle,
        const float rotaryEndAngle, juce::Slider& slider) override
    {
        auto outline = juce::Colour(0x33FFFFFF);
        auto fill = juce::Colour(0xFFCCFF00); // Neón

        auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(10);
        auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
        auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        auto lineW = 3.0f;
        auto arcRadius = radius - lineW * 0.5f;

        juce::Path backgroundArc;
        backgroundArc.addCentredArc(bounds.getCentreX(), bounds.getCentreY(), arcRadius, arcRadius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
        g.setColour(outline);
        g.strokePath(backgroundArc, juce::PathStrokeType(lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        if (sliderPos > 0.0f)
        {
            juce::Path valueArc;
            valueArc.addCentredArc(bounds.getCentreX(), bounds.getCentreY(), arcRadius, arcRadius, 0.0f, rotaryStartAngle, toAngle, true);
            g.setColour(fill);
            g.strokePath(valueArc, juce::PathStrokeType(lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            
            // Glow
            g.setColour(fill.withAlpha(0.3f));
            g.strokePath(valueArc, juce::PathStrokeType(lineW * 2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }

        // Thumb dot
        auto thumbRadius = 4.0f;
        juce::Point<float> thumbPos(bounds.getCentreX() + arcRadius * std::cos(toAngle - juce::MathConstants<float>::halfPi),
                                    bounds.getCentreY() + arcRadius * std::sin(toAngle - juce::MathConstants<float>::halfPi));
        
        g.setColour(juce::Colours::white);
        g.fillEllipse(juce::Rectangle<float>(thumbRadius * 2.0f, thumbRadius * 2.0f).withCentre(thumbPos));
    }

    void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
                              bool isMouseOverButton, bool isButtonDown) override
    {
        auto bounds = button.getLocalBounds().toFloat();
        auto cornerSize = bounds.getHeight() * 0.5f;

        if (isButtonDown || button.getToggleState())
            g.setColour(juce::Colour(0x4DFFFFFF));
        else if (isMouseOverButton)
            g.setColour(juce::Colour(0x1AFFFFFF));
        else
            g.setColour(juce::Colour(0x0DFFFFFF));

        g.fillRoundedRectangle(bounds, cornerSize);
        
        g.setColour(juce::Colour(0x33FFFFFF));
        g.drawRoundedRectangle(bounds.reduced(0.5f), cornerSize, 1.0f);
    }
};

static void drawDotMatrixNumber(juce::Graphics& g, float value, juce::Rectangle<float> area, juce::Colour dotColor)
{
    juce::String text = juce::String(value, 1);
    float dotSize = 5.0f;
    float spacing = 1.5f;
    
    auto drawDigit = [&](int digit, float startX, float startY) {
        static const unsigned char patterns[11][7] = {
            {0x1F, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1F}, // 0
            {0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E}, // 1
            {0x1F, 0x01, 0x01, 0x1F, 0x10, 0x10, 0x1F}, // 2
            {0x1F, 0x01, 0x01, 0x1F, 0x01, 0x01, 0x1F}, // 3
            {0x11, 0x11, 0x11, 0x1F, 0x01, 0x01, 0x01}, // 4
            {0x1F, 0x10, 0x10, 0x1F, 0x01, 0x01, 0x1F}, // 5
            {0x1F, 0x10, 0x10, 0x1F, 0x11, 0x11, 0x1F}, // 6
            {0x1F, 0x01, 0x01, 0x02, 0x04, 0x08, 0x08}, // 7
            {0x1F, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x1F}, // 8
            {0x1F, 0x11, 0x11, 0x1F, 0x01, 0x01, 0x1F}, // 9
            {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04}  // .
        };

        const unsigned char* p = patterns[digit];
        for (int row = 0; row < 7; ++row) {
            for (int col = 0; col < 5; ++col) {
                if (p[row] & (1 << (4 - col))) {
                    g.setColour(dotColor);
                    g.fillEllipse(startX + col * (dotSize + spacing), startY + row * (dotSize + spacing), dotSize, dotSize);
                    g.setColour(dotColor.withAlpha(0.2f));
                    g.fillEllipse(startX + col * (dotSize + spacing) - 1, startY + row * (dotSize + spacing) - 1, dotSize + 2, dotSize + 2);
                }
            }
        }
    };

    float charW = 5 * (dotSize + spacing);
    float totalW = text.length() * (charW + 8.0f);
    float startX = area.getCentreX() - totalW * 0.5f;
    float startY = area.getCentreY() - (7 * (dotSize + spacing)) * 0.5f;

    for (int i = 0; i < text.length(); ++i) {
        juce::juce_wchar c = text[i];
        int idx = (c == '.') ? 10 : (c - '0');
        if (idx >= 0 && idx <= 10)
            drawDigit(idx, startX + i * (charW + 8.0f), startY);
    }
}

static void drawGlassPanel(juce::Graphics& g, juce::Rectangle<float> area, float cornerSize)
{
    // Sombra exterior
    g.setColour(juce::Colours::black.withAlpha(0.4f));
    g.fillRoundedRectangle(area.translated(2, 4), cornerSize);

    // Fondo principal (Frosted)
    g.setColour(juce::Colour(0x12FFFFFF));
    g.fillRoundedRectangle(area, cornerSize);
    
    // Brillo de superficie
    juce::ColourGradient glassGrad(juce::Colours::white.withAlpha(0.08f), area.getX(), area.getY(),
                                 juce::Colours::transparentWhite, area.getCentreX(), area.getCentreY(), true);
    g.setGradientFill(glassGrad);
    g.fillRoundedRectangle(area, cornerSize);
    
    // Borde brillante (Glass Edge)
    juce::ColourGradient edgeGrad(juce::Colours::white.withAlpha(0.4f), area.getX(), area.getY(),
                                 juce::Colours::white.withAlpha(0.1f), area.getRight(), area.getBottom(), false);
    g.setGradientFill(edgeGrad);
    g.drawRoundedRectangle(area.reduced(0.5f), cornerSize, 1.5f);
}

class MESABOOGIEINGSOFTAMPAudioProcessorEditor : public juce::AudioProcessorEditor,
                                                  public juce::Timer
{
public:
    MESABOOGIEINGSOFTAMPAudioProcessorEditor(MESABOOGIEINGSOFTAMPAudioProcessor&);
    ~MESABOOGIEINGSOFTAMPAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override; // Fase 2: actualiza el tuner en tiempo real

private:
    MESABOOGIEINGSOFTAMPAudioProcessor& audioProcessor;

    NeuralLookAndFeel neuralLF;
    void updateVisibility();
    int currentTab = 0;

    // --- NUEVO: Manejador para cargar archivos WAV (IRs) ---
    std::unique_ptr<juce::FileChooser> fileChooser;

    // --- Componentes de Navegación ---
    juce::TextButton ampTab, cabTab, fxTab, eqTab;

    // --- Sliders de Control ---
    juce::Slider gainSlider, bassSlider, midSlider, trebleSlider, presenceSlider, masterSlider;
    juce::Slider inputSlider, outputSlider;
    juce::Label inputLabel, outputLabel;

    // --- Gabinete ---
    juce::ComboBox micSelector;
    juce::Slider lowCutSlider, highCutSlider;

    // --- Efectos ---
// --- Overdrive ---
    juce::Slider odDrive, odTone, odLevel;
    juce::Label odDriveLabel, odToneLabel, odLevelLabel;

    // --- Chorus ---
    juce::Slider choRate, choDepth, choLevel;
    juce::Label choRateLabel, choDepthLabel, choLevelLabel;

    // --- Delay ---
    juce::Slider delTime, delFb, delMix;
    juce::Label delTimeLabel, delFbLabel, delMixLabel;

    // --- Ecualizador Gráfico ---
    juce::Slider eqBand1, eqBand2, eqBand3, eqBand4, eqBand5, eqBand6;

    // --- Noise Gate (Fase 3) ---
    // Controla el umbral por debajo del cual se silencia la señal,
    // el tiempo de apertura (Attack) y el tiempo de cierre (Release).
    juce::Slider ngThreshSlider, ngAttackSlider, ngReleaseSlider;

    // --- Presets y Tuner ---
    juce::ComboBox presetMenu;
    juce::TextButton prevPresetBtn{ "<" }, nextPresetBtn{ ">" }, savePresetBtn{ "SAVE" };
    juce::TextButton tunerBtn{ "TUNER" };



    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
        gainAtt, bassAtt, midAtt, trebleAtt, presenceAtt, masterAtt,
        inputAtt, outputAtt, lowCutAtt, highCutAtt,
        odDriveAtt, odToneAtt, odLevelAtt,
        choRateAtt, choDepthAtt, choLevelAtt,
        delTimeAtt, delFbAtt, delMixAtt,
        eqBand1Att, eqBand2Att, eqBand3Att,
        eqBand4Att, eqBand5Att, eqBand6Att,
        ngThreshAtt, ngAttackAtt, ngReleaseAtt; // Noise Gate

    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>
        micAtt;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MESABOOGIEINGSOFTAMPAudioProcessorEditor)
};