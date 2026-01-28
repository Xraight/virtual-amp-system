# Virtual Guitar Amplifier - Quick Start Guide

## Installation

```bash
# Install dependencies
pip install -r requirements.txt
```

## Running the Amplifier

```bash
# Start the virtual amplifier
python main.py
```

## Example Session

```
============================================================
  🎸 Virtual Guitar Amplifier System 🎸
============================================================

Starting virtual amplifier...
Loaded preset: clean

✓ Amplifier is running!

Available Commands:
  preset <name>  - Load a preset (clean, crunch, overdrive, distortion, metal)
  gain <value>   - Set gain (0.1 - 5.0)
  dist <value>   - Set distortion (0.0 - 1.0)
  ...

amp> preset metal
Loaded preset: metal

amp> status
============================================================
Current Preset: METAL
------------------------------------------------------------
  Gain:       4.00
  Distortion: 0.90
  Bass:       0.40
  Mid:        -0.20
  Treble:     0.50
  Reverb:     0.05
============================================================

amp> dist 0.7
Distortion set to 0.70

amp> gain 3.5
Gain set to 3.50

amp> status
============================================================
Current Preset: CUSTOM
------------------------------------------------------------
  Gain:       3.50
  Distortion: 0.70
  ...
============================================================

amp> quit
Goodbye! 🎸
```

## Testing Without Audio Hardware

```bash
# Run the test suite
python tests/test_amplifier.py

# Run the demo (generates visualizations)
python demo.py
```

## Preset Descriptions

- **clean**: Natural tone, light reverb - for jazz, funk, clean rhythm
- **crunch**: Light overdrive, slightly boosted mids - for blues, classic rock
- **overdrive**: Medium distortion, balanced tone - for rock, alternative
- **distortion**: Heavy distortion, boosted lows/highs - for hard rock
- **metal**: Maximum distortion, scooped mids - for metal, extreme genres

## Tips

1. Start with a preset, then fine-tune parameters
2. Keep gain < 3.0 for cleaner tones, > 3.0 for heavy distortion
3. Use bass/treble for overall tone, mid for presence/scooping
4. Reverb adds space - use sparingly for tight metal tones
5. The 'custom' preset appears when you modify any parameter

