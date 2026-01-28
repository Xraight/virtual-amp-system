#!/usr/bin/env python3
"""
Demo script for Virtual Guitar Amplifier
Demonstrates audio processing without actually starting audio I/O
"""

import sys
import os
import numpy as np
import os
import tempfile
import matplotlib
matplotlib.use('Agg')  # Use non-interactive backend
import matplotlib.pyplot as plt

# Add parent directory to path
sys.path.insert(0, os.path.abspath(os.path.dirname(__file__)))

from src.effects import GuitarAmpEffects


def generate_test_tone(frequency=440, duration=1.0, sample_rate=44100):
    """Generate a test tone (sine wave)"""
    t = np.linspace(0, duration, int(sample_rate * duration), False)
    tone = np.sin(2 * np.pi * frequency * t) * 0.5
    return tone


def demo_distortion_effect():
    """Demonstrate distortion effect on a test tone"""
    print("\n" + "=" * 60)
    print("Demo: Distortion Effect")
    print("=" * 60)
    
    effects = GuitarAmpEffects(sample_rate=44100)
    
    # Generate test signal (A440 note)
    test_signal = generate_test_tone(440, 0.1)
    
    # Process with different distortion levels
    distortion_levels = [0.0, 0.3, 0.6, 0.9]
    
    fig, axes = plt.subplots(2, 2, figsize=(12, 8))
    fig.suptitle('Distortion Effect on 440Hz Tone', fontsize=16)
    
    for i, dist in enumerate(distortion_levels):
        effects.set_parameters(distortion=dist, gain=1.0)
        processed = effects.apply_distortion(test_signal)
        
        ax = axes[i // 2, i % 2]
        time = np.arange(len(processed[:500])) / 44100 * 1000  # Convert to ms
        ax.plot(time, processed[:500])
        ax.set_title(f'Distortion: {dist}')
        ax.set_xlabel('Time (ms)')
        ax.set_ylabel('Amplitude')
        ax.grid(True, alpha=0.3)
        ax.set_ylim(-1.0, 1.0)
    
    plt.tight_layout()
    output_file = os.path.join(tempfile.gettempdir(), 'distortion_demo.png')
    plt.savefig(output_file, dpi=150)
    print(f"\n✓ Distortion waveform plot saved to: {output_file}")
    
    # Print statistics
    for dist in distortion_levels:
        effects.set_parameters(distortion=dist)
        processed = effects.apply_distortion(test_signal)
        peak = np.max(np.abs(processed))
        rms = np.sqrt(np.mean(processed ** 2))
        print(f"  Distortion {dist:.1f}: Peak={peak:.3f}, RMS={rms:.3f}")


def demo_eq_effect():
    """Demonstrate EQ effect"""
    print("\n" + "=" * 60)
    print("Demo: EQ (Tone Control) Effect")
    print("=" * 60)
    
    effects = GuitarAmpEffects(sample_rate=44100)
    
    # Generate a complex test signal with multiple frequencies
    signal_100hz = generate_test_tone(100, 0.5) * 0.3
    signal_800hz = generate_test_tone(800, 0.5) * 0.3
    signal_3000hz = generate_test_tone(3000, 0.5) * 0.3
    test_signal = signal_100hz + signal_800hz + signal_3000hz
    
    # Test different EQ settings
    eq_settings = [
        ("Flat", {'bass': 0.0, 'mid': 0.0, 'treble': 0.0}),
        ("Bass Boost", {'bass': 0.8, 'mid': 0.0, 'treble': 0.0}),
        ("Treble Boost", {'bass': 0.0, 'mid': 0.0, 'treble': 0.8}),
        ("Mid Scoop", {'bass': 0.5, 'mid': -0.8, 'treble': 0.5}),
    ]
    
    for name, params in eq_settings:
        effects.set_parameters(**params)
        processed = effects.apply_eq(test_signal)
        rms = np.sqrt(np.mean(processed ** 2))
        peak = np.max(np.abs(processed))
        print(f"  {name:15s}: RMS={rms:.3f}, Peak={peak:.3f}")


def demo_complete_chain():
    """Demonstrate complete effect chain with presets"""
    print("\n" + "=" * 60)
    print("Demo: Complete Effect Chain")
    print("=" * 60)
    
    from src.amplifier import VirtualAmp
    
    amp = VirtualAmp()
    
    # Generate test signal
    test_signal = generate_test_tone(440, 0.5)
    
    # Test each preset
    for preset in amp.list_presets():
        amp.load_preset(preset)
        settings = amp.get_current_settings()
        processed = amp.effects.process(test_signal)
        
        rms = np.sqrt(np.mean(processed ** 2))
        peak = np.max(np.abs(processed))
        
        print(f"\n  Preset: {preset.upper()}")
        print(f"    Gain: {settings['gain']:.2f}, Distortion: {settings['distortion']:.2f}")
        print(f"    Output RMS: {rms:.3f}, Peak: {peak:.3f}")


def demo_reverb():
    """Demonstrate reverb effect"""
    print("\n" + "=" * 60)
    print("Demo: Reverb Effect")
    print("=" * 60)
    
    effects = GuitarAmpEffects(sample_rate=44100)
    
    # Generate a short pulse
    pulse = np.zeros(22050)  # 0.5 seconds
    pulse[0:100] = 0.5  # Short impulse
    
    reverb_mixes = [0.0, 0.3, 0.6, 1.0]
    
    for mix in reverb_mixes:
        effects.set_parameters(reverb_mix=mix)
        processed = effects.apply_reverb(pulse)
        
        # Calculate decay time (time to -40dB)
        energy = processed ** 2
        cumsum = np.cumsum(energy)
        total_energy = cumsum[-1]
        decay_idx = np.where(cumsum >= total_energy * 0.99)[0]
        decay_time = decay_idx[0] / 44100 * 1000 if len(decay_idx) > 0 else 0
        
        print(f"  Reverb Mix {mix:.1f}: Decay time ~{decay_time:.1f}ms")


def main():
    """Run all demos"""
    print("\n" + "=" * 60)
    print("  🎸 Virtual Guitar Amplifier - Demo")
    print("=" * 60)
    print("\nThis demo shows the DSP algorithms working with synthetic signals.")
    print("No actual audio I/O is performed.\n")
    
    try:
        demo_distortion_effect()
        demo_eq_effect()
        demo_reverb()
        demo_complete_chain()
        
        print("\n" + "=" * 60)
        print("  ✓ All demos completed successfully!")
        print("=" * 60 + "\n")
        
    except Exception as e:
        print(f"\n✗ Error during demo: {e}")
        import traceback
        traceback.print_exc()
        return 1
    
    return 0


if __name__ == "__main__":
    sys.exit(main())
