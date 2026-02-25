#include <JuceHeader.h>
#include "PluginProcessor.h"
#include <cmath>
#include "BinaryData.h" 

// --- CLASE DE DISEÑO DEL AMP---
class NeuralLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
        float sliderPos, const float rotaryStartAngle,
        const float rotaryEndAngle, juce::Slider& slider) override
    {
        auto radius = (float)juce::jmin(width / 2, height / 2) - 25.0f;
        auto centreX = (float)x + (float)width * 0.5f;
        auto centreY = (float)y + (float)height * 0.5f;
        auto rx = centreX - radius;
        auto ry = centreY - radius;
        auto rw = radius * 2.0f;
        auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

        // --- Rotación ---
        if (slider.getName() != "master_io")
        {
            g.setColour(juce::Colours::black.withAlpha(0.8f));
            g.setFont(juce::Font("Arial", 12.0f, juce::Font::bold));

            for (int i = 0; i <= 10; ++i)
            {
                auto nAngle = rotaryStartAngle + (i / 10.0f) * (rotaryEndAngle - rotaryStartAngle);
                auto numRadius = radius + 14.0f;
                auto nX = centreX + numRadius * std::sin(nAngle) - 10.0f;
                auto nY = centreY - numRadius * std::cos(nAngle) - 10.0f;
                g.drawText(juce::String(i), (int)nX, (int)nY, 20, 20, juce::Justification::centred);
            }
        }

        // --- SOMBRA Y CUERPO DE PERILLA ---
        g.setColour(juce::Colours::black.withAlpha(0.3f));
        g.fillEllipse(rx + 3, ry + 3, rw, rw);

        juce::ColourGradient knobGradient(juce::Colour(0xFF333333), centreX, ry,
            juce::Colour(0xFF050505), centreX, ry + rw, false);
        g.setGradientFill(knobGradient);
        g.fillEllipse(rx, ry, rw, rw);

        g.setColour(juce::Colours::white.withAlpha(0.1f));
        g.drawEllipse(rx + 0.5f, ry + 0.5f, rw - 1.0f, rw - 1.0f, 1.5f);

        // --- INDICADOR ---
        juce::Path p;
        p.addRectangle(-1.5f, -radius + 2.0f, 3.0f, radius * 0.5f);
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
    // --- Procesador ---
    MESABOOGIEINGSOFTAMPAudioProcessor& audioProcessor;

    // --- Estética---
    NeuralLookAndFeel neuralLF;
    void updateVisibility();
    int currentTab = 0;

    // --- Componentes de Navegación (Tabs) ---
    juce::TextButton ampTab, cabTab, fxTab, eqTab;

    // --- Sliders de Control ---
    juce::Slider gainSlider, bassSlider, midSlider, trebleSlider, presenceSlider, masterSlider;
    juce::Slider inputSlider, outputSlider;
    juce::Label inputLabel, outputLabel;

    // --- Gabinete  ---
    juce::ComboBox micSelector;
    juce::Slider lowCutSlider, highCutSlider;

    // --- Efectos ---
    juce::Slider odDrive, odTone, odLevel;
    juce::Slider choRate, choDepth;
    juce::Slider delTime, delFb, delMix;

    // --- Ecualizador Gráfico ---
    juce::Slider eqBand1, eqBand2, eqBand3, eqBand4, eqBand5, eqBand6;

    // --- Presets y Tuner ---
    juce::ComboBox presetMenu;
    juce::TextButton prevPresetBtn{ "<" }, nextPresetBtn{ ">" }, savePresetBtn{ "SAVE" };
    juce::TextButton tunerBtn{ "TUNER" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MESABOOGIEINGSOFTAMPAudioProcessorEditor)
};