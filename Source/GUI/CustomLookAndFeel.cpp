#include "CustomLookAndFeel.h"
#include "DesignTokens.h"

namespace tk = nec::tokens;

namespace
{
    // Punto sobre la circunferencia (0 rad = 12 en punto, sentido horario, como JUCE).
    juce::Point<float> polar (juce::Point<float> c, float r, float angle)
    {
        return { c.x + r * std::sin (angle), c.y - r * std::cos (angle) };
    }
}

CustomLookAndFeel::CustomLookAndFeel()
{
    setColour (juce::Slider::rotarySliderFillColourId,    tk::colour::silk);
    setColour (juce::Slider::rotarySliderOutlineColourId, tk::colour::line);
    setColour (juce::Slider::thumbColourId,               tk::colour::silk);
}

void CustomLookAndFeel::applyKnobRotaryParameters (juce::Slider& s)
{
    s.setRotaryParameters (tk::knob::startAngle, tk::knob::endAngle, true);
}

// -----------------------------------------------------------------------------
//  Geometría: todo se deriva del tamaño de la caja y de los tokens
// -----------------------------------------------------------------------------
CustomLookAndFeel::KnobGeometry CustomLookAndFeel::computeGeometry (juce::Rectangle<float> bounds)
{
    KnobGeometry geo;
    geo.centre      = bounds.getCentre();
    geo.outerRadius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f - (float) tk::space::xs;
    geo.tickOuter   = geo.outerRadius - tk::type::scaleSize - tk::knob::scaleGap;
    geo.ringRadius  = geo.tickOuter - tk::knob::majorTickLength - tk::knob::ringGap;
    geo.bodyRadius  = geo.ringRadius - tk::knob::ringWidth;
    return geo;
}

// -----------------------------------------------------------------------------
//  Capa estática: escala 0–10, marcas, anillo y cuerpo con relieve
// -----------------------------------------------------------------------------
void CustomLookAndFeel::paintStaticKnobLayer (juce::Graphics& g, juce::Rectangle<float> bounds,
                                              float startAngle, float endAngle)
{
    const auto geo = computeGeometry (bounds);
    if (geo.bodyRadius <= 0.0f)
        return;

    const auto c     = geo.centre;
    const float span = endAngle - startAngle;

    // --- Marcas de escala (mayores en cada número, menores entre ellos) ---
    const int totalTicks = tk::knob::scaleSteps * (tk::knob::minorPerStep + 1);
    for (int i = 0; i <= totalTicks; ++i)
    {
        const bool  isMajor = (i % (tk::knob::minorPerStep + 1)) == 0;
        const float a       = startAngle + span * ((float) i / (float) totalTicks);
        const float len     = isMajor ? tk::knob::majorTickLength : tk::knob::minorTickLength;

        g.setColour (isMajor ? tk::colour::silk
                             : tk::colour::silk.withAlpha (tk::alpha::silkMuted));
        g.drawLine ({ polar (c, geo.tickOuter, a), polar (c, geo.tickOuter - len, a) },
                    tk::knob::tickWidth);
    }

    // --- Números 0..10 ---
    g.setColour (tk::colour::silk);
    g.setFont (tk::type::make (tk::type::scaleSize));
    const float numRadius = geo.outerRadius - tk::type::scaleSize * 0.5f;
    const float boxW      = tk::type::scaleSize * 2.0f;

    for (int i = 0; i <= tk::knob::scaleSteps; ++i)
    {
        const float a  = startAngle + span * ((float) i / (float) tk::knob::scaleSteps);
        const auto  p  = polar (c, numRadius, a);
        g.drawText (juce::String (i),
                    juce::Rectangle<float> (boxW, tk::type::scaleSize).withCentre (p),
                    juce::Justification::centred, false);
    }

    // --- Sombra proyectada ---
    const auto ringBounds = juce::Rectangle<float> (geo.ringRadius * 2.0f, geo.ringRadius * 2.0f).withCentre (c);
    g.setColour (tk::colour::chassis.withAlpha (tk::alpha::shadow));
    g.fillEllipse (ringBounds.translated (0.0f, tk::knob::shadowOffset));

    // --- Anillo exterior (bisel metálico oscuro) ---
    g.setGradientFill (juce::ColourGradient (tk::colour::metalDark.brighter (tk::knob::bodyTopLighten),
                                             c.x, ringBounds.getY(),
                                             tk::colour::metalDark.darker (tk::knob::bodyBottomDark),
                                             c.x, ringBounds.getBottom(), false));
    g.fillEllipse (ringBounds);
    g.setColour (tk::colour::line);
    g.drawEllipse (ringBounds, tk::stroke::hairline);

    // --- Cuerpo con relieve sutil (luz cenital) ---
    const auto bodyBounds = juce::Rectangle<float> (geo.bodyRadius * 2.0f, geo.bodyRadius * 2.0f).withCentre (c);
    g.setGradientFill (juce::ColourGradient (tk::colour::panel.brighter (tk::knob::bodyTopLighten),
                                             c.x, bodyBounds.getY(),
                                             tk::colour::chassis,
                                             c.x, bodyBounds.getBottom(), false));
    g.fillEllipse (bodyBounds);

    // Brillo superior muy tenue
    g.setGradientFill (juce::ColourGradient (tk::colour::silk.withAlpha (tk::alpha::highlight),
                                             c.x, bodyBounds.getY(),
                                             tk::colour::silk.withAlpha (0.0f),
                                             c.x, c.y, false));
    g.fillEllipse (bodyBounds.reduced (tk::stroke::hairline));

    g.setColour (tk::colour::chassis);
    g.drawEllipse (bodyBounds, tk::stroke::hairline);
}

const juce::Image& CustomLookAndFeel::getStaticKnobLayer (int width, int height, float physicalScale,
                                                          float startAngle, float endAngle)
{
    const CacheKey key { width, height, juce::roundToInt (physicalScale * 100.0f) };

    if (auto it = knobCache.find (key); it != knobCache.end())
        return it->second;

    if (knobCache.size() >= maxCacheEntries)
        knobCache.clear();

    const int pw = juce::jmax (1, juce::roundToInt ((float) width  * physicalScale));
    const int ph = juce::jmax (1, juce::roundToInt ((float) height * physicalScale));

    juce::Image img (juce::Image::ARGB, pw, ph, true);
    {
        juce::Graphics ig (img);
        ig.addTransform (juce::AffineTransform::scale (physicalScale));
        paintStaticKnobLayer (ig, { 0.0f, 0.0f, (float) width, (float) height }, startAngle, endAngle);
    }

    return knobCache.emplace (key, std::move (img)).first->second;
}

// -----------------------------------------------------------------------------
//  drawRotarySlider: imagen cacheada + indicador dinámico
// -----------------------------------------------------------------------------
void CustomLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                          float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                                          juce::Slider& slider)
{
    if (width <= 0 || height <= 0)
        return;

    const auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat();
    const float scale = g.getInternalContext().getPhysicalPixelScaleFactor();
    const float alpha = slider.isEnabled() ? 1.0f : tk::alpha::disabled;

    // 1) Capa estática
    const auto& layer = getStaticKnobLayer (width, height, scale, rotaryStartAngle, rotaryEndAngle);
    g.setOpacity (alpha);
    g.drawImage (layer, bounds);

    // 2) Indicador (lo único que cambia con el valor)
    const auto  geo   = computeGeometry (bounds);
    if (geo.bodyRadius <= 0.0f)
        return;

    const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    juce::Path pointer;
    pointer.startNewSubPath (polar (geo.centre, geo.bodyRadius * tk::knob::pointerInner, angle));
    pointer.lineTo          (polar (geo.centre, geo.bodyRadius * tk::knob::pointerOuter, angle));

    g.setColour (tk::colour::silk.withMultipliedAlpha (alpha));
    g.strokePath (pointer, juce::PathStrokeType (tk::knob::pointerWidth,
                                                 juce::PathStrokeType::curved,
                                                 juce::PathStrokeType::rounded));
}
