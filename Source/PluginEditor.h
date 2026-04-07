#include <JuceHeader.h>
#include "PluginProcessor.h"
#include <cmath>
#include "BinaryData.h"

// --- CLASE DE DISEÑO DEL AMP ---
class NeuralLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
        float sliderPos, const float rotaryStartAngle,
        const float rotaryEndAngle, juce::Slider& slider) override
    {
        auto radius = (float)juce::jmin(width / 2, height / 2) - 15.0f;
        auto centreX = (float)x + (float)width * 0.5f;
        auto centreY = (float)y + (float)height * 0.5f;
        auto rx = centreX - radius;
        auto ry = centreY - radius;
        auto rw = radius * 2.0f;
        auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

        if (slider.getName() != "pedal" &&
            slider.getName() != "noNumbers" &&
            slider.getName() != "io")
        {
            g.setColour(juce::Colours::black.withAlpha(0.7f));
            g.setFont(juce::Font(juce::FontOptions().withName("Helvetica").withHeight(10.0f).withStyle("Bold")));
            for (int i = 0; i <= 10; ++i)
            {
                auto nAngle = rotaryStartAngle + (i / 10.0f) * (rotaryEndAngle - rotaryStartAngle);
                auto numRadius = radius + 10.0f;
                auto nX = centreX + numRadius * std::sin(nAngle) - 10.0f;
                auto nY = centreY - numRadius * std::cos(nAngle) - 10.0f;
                g.drawText(juce::String(i), (int)nX, (int)nY, 20, 20, juce::Justification::centred);
            }
        }

        g.setColour(juce::Colours::black.withAlpha(0.4f));
        g.fillEllipse(rx + 2, ry + 2, rw, rw);

        juce::ColourGradient knobGradient(juce::Colour(0xFF444444), centreX, ry,
            juce::Colour(0xFF111111), centreX, ry + rw, false);
        g.setGradientFill(knobGradient);
        g.fillEllipse(rx, ry, rw, rw);

        g.setColour(juce::Colours::white.withAlpha(0.2f));
        g.drawEllipse(rx + 0.5f, ry + 0.5f, rw - 1.0f, rw - 1.0f, 1.0f);

        juce::Path p;
        p.addRoundedRectangle(-1.0f, -radius + 1.0f, 2.0f, radius * 0.4f, 1.0f);
        p.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));
        g.setColour(juce::Colours::white.withAlpha(0.9f));
        g.fillPath(p);
    }
};

class MESABOOGIEINGSOFTAMPAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    MESABOOGIEINGSOFTAMPAudioProcessorEditor(MESABOOGIEINGSOFTAMPAudioProcessor&);
    ~MESABOOGIEINGSOFTAMPAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

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
        eqBand4Att, eqBand5Att, eqBand6Att;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>
        micAtt;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MESABOOGIEINGSOFTAMPAudioProcessorEditor)
};