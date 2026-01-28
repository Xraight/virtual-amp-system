# 🎸 Virtual Guitar Amplifier System

A real-time digital guitar amplifier application with professional-grade audio processing, multiple effects, and amp simulation presets.

## Features

- **Real-time Audio Processing**: Low-latency audio input/output using PyAudio
- **Guitar Amp Simulation**: Multiple classic amp tones (Clean, Crunch, Overdrive, Distortion, Metal)
- **Digital Effects**:
  - Gain/Volume control
  - Distortion with soft clipping algorithm
  - 3-band EQ (Bass, Mid, Treble)
  - Reverb effect
- **Preset System**: Quick access to pre-configured amp sounds
- **Interactive CLI**: Easy-to-use command-line interface for real-time control

## Requirements

- Python 3.8 or higher
- Audio input device (guitar interface, microphone, or line-in)
- Audio output device (speakers or headphones)

## Installation

1. Clone the repository:
```bash
git clone https://github.com/Xraight/virtual-amp-system.git
cd virtual-amp-system
```

2. Install dependencies:
```bash
pip install -r requirements.txt
```

**Note for Linux users**: You may need to install PortAudio:
```bash
sudo apt-get install portaudio19-dev  # Ubuntu/Debian
```

**Note for macOS users**: You may need to install PortAudio via Homebrew:
```bash
brew install portaudio
```

## Usage

### Starting the Amplifier

Run the main application:
```bash
python main.py
```

The amplifier will start with the "clean" preset. Connect your guitar to your audio interface before starting.

### Available Commands

Once running, you can use these commands in the interactive prompt:

- `preset <name>` - Load a preset (clean, crunch, overdrive, distortion, metal)
- `gain <value>` - Set gain (0.1 - 5.0)
- `dist <value>` - Set distortion (0.0 - 1.0)
- `bass <value>` - Set bass EQ (-1.0 to 1.0)
- `mid <value>` - Set mid EQ (-1.0 to 1.0)
- `treble <value>` - Set treble EQ (-1.0 to 1.0)
- `reverb <value>` - Set reverb mix (0.0 - 1.0)
- `status` - Show current settings
- `presets` - List available presets
- `help` - Show help
- `quit` - Exit the application

### Example Session

```
amp> preset crunch
Loaded preset: crunch

amp> status
Current Preset: CRUNCH
  Gain:       2.00
  Distortion: 0.30
  Bass:       0.20
  ...

amp> dist 0.5
Distortion set to 0.50

amp> gain 3.0
Gain set to 3.00

amp> quit
```

## Presets

### Clean
- Natural, uncolored tone
- Light reverb
- Perfect for jazz, funk, and clean rhythm

### Crunch
- Light overdrive
- Slightly boosted mids
- Great for blues and classic rock

### Overdrive
- Medium distortion
- Balanced tone
- Ideal for rock and alternative

### Distortion
- Heavy distortion
- Boosted lows and highs
- Perfect for hard rock

### Metal
- Maximum distortion
- Scooped mids
- Designed for heavy metal and extreme genres

## Architecture

The system consists of three main components:

1. **Audio Engine** (`src/audio_engine.py`): Handles real-time audio I/O using PyAudio
2. **Effects Processor** (`src/effects.py`): Implements DSP algorithms for distortion, EQ, and reverb
3. **Virtual Amplifier** (`src/amplifier.py`): Combines audio engine and effects with preset management

## Technical Details

- **Sample Rate**: 44.1 kHz (CD quality)
- **Buffer Size**: 1024 samples (~23ms latency)
- **Audio Format**: 32-bit float
- **Channels**: Mono (1 channel)
- **Processing**: Real-time callback-based processing

## Troubleshooting

### No audio input/output
- Check your audio interface is properly connected
- Verify audio input/output settings in your system
- Try adjusting the buffer size in the code if you experience dropouts

### High latency
- Reduce the buffer size (chunk_size) in `main.py`
- Note: Smaller buffers may cause audio glitches on slower systems

### Audio clipping/distortion
- Reduce the gain setting
- Check your input level isn't too hot
- Adjust your audio interface input gain

## Future Enhancements

Potential features for future development:
- GUI interface with visual controls
- Additional effects (chorus, flanger, delay)
- Cabinet simulation
- MIDI control support
- Preset saving/loading from files
- Multi-channel support for stereo processing

## License

This project is open source and available for educational and personal use.

## Contributing

Contributions are welcome! Feel free to submit issues or pull requests.

## Acknowledgments

Built with Python, NumPy, SciPy, and PyAudio for real-time digital signal processing.