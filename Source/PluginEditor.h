#include <JuceHeader.h>
#include "PluginProcessor.h"
#include <cmath>
#include "BinaryData.h"
#include "GUI/CustomLookAndFeel.h"

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
            g.setColour(juce::Colours::white.withAlpha(0.85f));
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

        // Gradiente tipo cromo metálico
        juce::ColourGradient knobGradient(
            juce::Colour(0xFF888888), centreX - radius * 0.3f, ry,
            juce::Colour(0xFF111111), centreX + radius * 0.3f, ry + rw, false);
        knobGradient.addColour(0.3, juce::Colour(0xFF555555));
        knobGradient.addColour(0.6, juce::Colour(0xFF222222));
        knobGradient.addColour(0.85, juce::Colour(0xFF444444));
        g.setGradientFill(knobGradient);
        g.fillEllipse(rx, ry, rw, rw);

        // Anillo exterior tipo níquel
        g.setColour(juce::Colour(0xFF999999).withAlpha(0.5f));
        g.drawEllipse(rx, ry, rw, rw, 2.5f);

        g.setColour(juce::Colours::white.withAlpha(0.2f));
        g.drawEllipse(rx + 0.5f, ry + 0.5f, rw - 1.0f, rw - 1.0f, 1.0f);

        // Highlight superior
        juce::ColourGradient highlight(
            juce::Colours::white.withAlpha(0.25f), centreX - radius * 0.2f, ry + 2,
            juce::Colours::transparentWhite, centreX, ry + radius * 0.6f, false);
        g.setGradientFill(highlight);
        g.fillEllipse(rx + rw * 0.2f, ry + rw * 0.05f, rw * 0.5f, rw * 0.35f);

        juce::Path p;
        p.addRoundedRectangle(-1.0f, -radius + 1.0f, 2.0f, radius * 0.4f, 1.0f);
        p.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));
        g.setColour(juce::Colours::white.withAlpha(0.9f));
        g.fillPath(p);
    }

    void drawButtonText(juce::Graphics& g, juce::TextButton& b,
        bool, bool) override
    {
        g.setColour(b.getToggleState()
            ? juce::Colours::white
            : juce::Colours::white.withAlpha(0.5f));
        g.setFont(juce::Font(juce::FontOptions()
            .withName("Helvetica Neue").withHeight(13.0f).withStyle("Bold")));
        g.drawFittedText(b.getButtonText(), b.getLocalBounds(),
            juce::Justification::centred, 1);
    }
};

class FootswitchLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawButtonBackground(juce::Graphics& g, juce::Button& b,
        const juce::Colour&, bool, bool) override
    {
        auto r = b.getLocalBounds().toFloat();
        bool on = b.getToggleState();

        g.setColour(juce::Colours::black.withAlpha(0.6f));
        g.fillEllipse(r.translated(0, on ? 1.0f : 3.0f));

        juce::ColourGradient grad(
            on ? juce::Colour(0xFF1A1A1A) : juce::Colour(0xFF4A4A4A),
            r.getCentreX(), r.getY(),
            on ? juce::Colour(0xFF0A0A0A) : juce::Colour(0xFF222222),
            r.getCentreX(), r.getBottom(), false);
        g.setGradientFill(grad);
        g.fillEllipse(r.reduced(2.0f).translated(0, on ? 1.0f : 0.0f));

        g.setColour(juce::Colour(0xFF666666).withAlpha(0.8f));
        g.drawEllipse(r.reduced(2.0f).translated(0, on ? 1.0f : 0.0f), 2.0f);

        if (!on) {
            g.setColour(juce::Colours::white.withAlpha(0.12f));
            g.fillEllipse(r.reduced(8.0f).removeFromTop(r.getHeight() * 0.35f));
        }
    }
};

class MESABOOGIEINGSOFTAMPAudioProcessorEditor : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    MESABOOGIEINGSOFTAMPAudioProcessorEditor(MESABOOGIEINGSOFTAMPAudioProcessor&);
    ~MESABOOGIEINGSOFTAMPAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    MESABOOGIEINGSOFTAMPAudioProcessor& audioProcessor;
    juce::String currentNoteName = "-";
    juce::String currentCents = "";
    void timerCallback() override;

    NeuralLookAndFeel neuralLF;
    FootswitchLookAndFeel footswitchLF;
    CustomLookAndFeel ampKnobLF; // Paso 1 del rediseño: solo las 6 perillas del amp
    void updateVisibility();
    int currentTab = 0;

    // --- NUEVO: Manejador para cargar archivos WAV (IRs) ---
    std::unique_ptr<juce::FileChooser> fileChooser;

    // --- Componentes de Navegación ---
    juce::TextButton ampTab, cabTab, fxTab, eqTab;
    juce::TextButton loadIRBtn{ "CARGAR IR (.WAV)" };
    juce::TextButton clearIRBtn{ "QUITAR IR" };


    // BD tabs:
    juce::TextButton presetsTab;
    juce::TextButton deletePresetBtn{ "BORRAR" };
    juce::TextButton updatePresetBtn{ "ACTUALIZAR" };

    // --- Sliders de Control ---
    juce::Slider gainSlider, bassSlider, midSlider, trebleSlider, presenceSlider, masterSlider;
    juce::Slider inputSlider, outputSlider;
    juce::Label inputLabel, outputLabel;

    // --- Gabinete ---
    juce::Label irNameLabel;
    juce::Slider irGainSlider;
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

	//footswitches
    juce::TextButton odFootswitch, choFootswitch, delFootswitch;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
        gainAtt, bassAtt, midAtt, trebleAtt, presenceAtt, masterAtt,
        inputAtt, outputAtt, lowCutAtt, highCutAtt,
        odDriveAtt, odToneAtt, odLevelAtt,
        choRateAtt, choDepthAtt, choLevelAtt,
        delTimeAtt, delFbAtt, delMixAtt,
        eqBand1Att, eqBand2Att, eqBand3Att,
        eqBand4Att, eqBand5Att, eqBand6Att;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> irGainAtt;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> odEnableAtt, choEnableAtt, delEnableAtt;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MESABOOGIEINGSOFTAMPAudioProcessorEditor)
};