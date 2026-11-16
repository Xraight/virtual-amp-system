#pragma once
#include <JuceHeader.h>
#include <map>
#include <tuple>

// =============================================================================
//  NoEraClean — CustomLookAndFeel
//  Todo el dibujo personalizado de la GUI vive aquí. Reemplazará
//  progresivamente a NeuralLookAndFeel (Paso 1: solo perillas del amplificador).
//  Uso exclusivo del Message Thread.
// =============================================================================
class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CustomLookAndFeel();
    ~CustomLookAndFeel() override = default;

    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPosProportional,
                           float rotaryStartAngle, float rotaryEndAngle,
                           juce::Slider&) override;

    // Aplica a un slider el barrido de 270° definido en los tokens.
    static void applyKnobRotaryParameters (juce::Slider&);

private:
    // Capa estática (escala impresa + anillo + cuerpo), pre-renderizada una vez
    // por (ancho, alto, escala física) y reutilizada en cada repintado.
    using CacheKey = std::tuple<int, int, int>;
    std::map<CacheKey, juce::Image> knobCache;
    static constexpr size_t maxCacheEntries = 32;

    const juce::Image& getStaticKnobLayer (int width, int height, float physicalScale,
                                           float startAngle, float endAngle);

    static void paintStaticKnobLayer (juce::Graphics&, juce::Rectangle<float> bounds,
                                      float startAngle, float endAngle);

    struct KnobGeometry
    {
        juce::Point<float> centre;
        float outerRadius;   // Borde exterior de los números
        float tickOuter;     // Inicio de las marcas
        float ringRadius;    // Anillo exterior (bisel)
        float bodyRadius;    // Cuerpo de la perilla
    };

    static KnobGeometry computeGeometry (juce::Rectangle<float> bounds);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CustomLookAndFeel)
};
