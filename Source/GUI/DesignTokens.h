#pragma once
#include <JuceHeader.h>

// =============================================================================
//  NoEraClean — Design Tokens
//  Única fuente de verdad para colores, tipografía, espaciados y medidas de la
//  GUI. Ningún paint()/LookAndFeel debe usar valores literales fuera de aquí.
// =============================================================================
namespace nec::tokens
{
    // -------------------------------------------------------------------------
    //  Paleta (casi monocromática + UN acento)
    // -------------------------------------------------------------------------
    namespace colour
    {
        inline const juce::Colour chassis   { 0xFF0D0D0E }; // Fondo / tolex
        inline const juce::Colour panel     { 0xFF19191B }; // Placa frontal
        inline const juce::Colour metalDark { 0xFF26282B }; // Biseles, anillos
        inline const juce::Colour line      { 0xFF3A3A3C }; // Separadores, bordes
        inline const juce::Colour silk      { 0xFFE8E6E1 }; // Serigrafía (blanco hueso)
        inline const juce::Colour accent    { 0xFF3B8EEA }; // Único acento: LED, pestaña activa, selección
    }

    // -------------------------------------------------------------------------
    //  Opacidades (relieve, sombras, estados)
    // -------------------------------------------------------------------------
    namespace alpha
    {
        inline constexpr float shadow       = 0.60f; // Sombra proyectada bajo la perilla
        inline constexpr float highlight    = 0.10f; // Brillo superior del cuerpo
        inline constexpr float silkMuted    = 0.55f; // Marcas menores de escala
        inline constexpr float disabled     = 0.35f; // Control deshabilitado
    }

    // -------------------------------------------------------------------------
    //  Espaciado: rejilla de 4 px
    // -------------------------------------------------------------------------
    namespace space
    {
        inline constexpr int unit = 4;
        inline constexpr int xs   = unit * 1;  //  4
        inline constexpr int sm   = unit * 2;  //  8
        inline constexpr int md   = unit * 3;  // 12
        inline constexpr int lg   = unit * 4;  // 16
        inline constexpr int xl   = unit * 6;  // 24
        inline constexpr int xxl  = unit * 8;  // 32
    }

    // -------------------------------------------------------------------------
    //  Radios y trazos
    // -------------------------------------------------------------------------
    namespace radius
    {
        inline constexpr float small = 4.0f;
        inline constexpr float panel = 8.0f;
    }

    namespace stroke
    {
        inline constexpr float hairline = 1.0f;
        inline constexpr float regular  = 2.0f;
    }

    // -------------------------------------------------------------------------
    //  Tipografía — cambiar SOLO `fontName()` para cambiar la fuente global
    // -------------------------------------------------------------------------
    namespace type
    {
        // Pendiente de decidir: por ahora la sans genérica de JUCE.
        inline juce::String fontName() { return juce::Font::getDefaultSansSerifFontName(); }

        inline constexpr float scaleSize  =  9.0f; // Números de escala 0–10
        inline constexpr float labelSize  = 11.0f; // GAIN, BASS, ...
        inline constexpr float valueSize  = 10.0f; // Lecturas de valor
        inline constexpr float titleSize  = 18.0f; // Marca / títulos

        inline juce::Font make (float height, bool bold = false)
        {
            return juce::Font (juce::FontOptions()
                                   .withName (fontName())
                                   .withHeight (height)
                                   .withStyle (bold ? "Bold" : "Regular"));
        }
    }

    // -------------------------------------------------------------------------
    //  Perilla rotativa
    // -------------------------------------------------------------------------
    namespace knob
    {
        // Tamaños de caja recomendados (incluye escala impresa)
        inline constexpr int sizeSmall  = space::unit * 16; //  64
        inline constexpr int sizeMedium = space::unit * 22; //  88
        inline constexpr int sizeLarge  = space::unit * 28; // 112

        // Barrido de 270°: de 7:30 a 4:30 (0 rad = 12 en punto, sentido horario)
        inline constexpr float startAngle = juce::MathConstants<float>::pi * 1.25f;
        inline constexpr float endAngle   = juce::MathConstants<float>::pi * 2.75f;

        // Escala impresa 0–10
        inline constexpr int   scaleSteps      = 10;   // Números 0..10
        inline constexpr int   minorPerStep    = 1;    // Marcas menores entre números
        inline constexpr float majorTickLength = 5.0f;
        inline constexpr float minorTickLength = 3.0f;
        inline constexpr float tickWidth       = 1.0f;
        inline constexpr float scaleGap        = 2.0f; // Entre números y marcas
        inline constexpr float ringGap         = 3.0f; // Entre marcas y anillo

        // Cuerpo
        inline constexpr float ringWidth       = 3.0f;  // Anillo exterior (bisel)
        inline constexpr float shadowOffset    = 2.0f;  // Desplazamiento de la sombra
        inline constexpr float pointerWidth    = 2.0f;
        inline constexpr float pointerInner    = 0.35f; // Fracción del radio del cuerpo
        inline constexpr float pointerOuter    = 0.88f;
        inline constexpr float bodyTopLighten  = 0.25f; // Relieve: cuánto aclarar arriba
        inline constexpr float bodyBottomDark  = 0.40f; // Relieve: cuánto oscurecer abajo
    }
}
