"""
Test suite for Virtual Guitar Amplifier System
Tests components without requiring audio hardware
"""

import sys
import os
import numpy as np

# Add parent directory to path
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

from src.effects import GuitarAmpEffects
from src.amplifier import VirtualAmp


def test_effects_initialization():
    """Test that effects processor initializes correctly"""
    print("Testing effects initialization...")
    effects = GuitarAmpEffects(sample_rate=44100)
    
    assert effects.sample_rate == 44100
    assert effects.gain == 1.0
    assert effects.distortion == 0.0
    assert effects.bass == 0.0
    assert effects.mid == 0.0
    assert effects.treble == 0.0
    assert effects.reverb_mix == 0.0
    
    print("✓ Effects initialization test passed")


def test_effects_parameter_setting():
    """Test setting effect parameters"""
    print("Testing parameter setting...")
    effects = GuitarAmpEffects()
    
    effects.set_parameters(
        gain=2.0,
        distortion=0.5,
        bass=0.3,
        mid=-0.2,
        treble=0.4,
        reverb_mix=0.3
    )
    
    assert effects.gain == 2.0
    assert effects.distortion == 0.5
    assert effects.bass == 0.3
    assert effects.mid == -0.2
    assert effects.treble == 0.4
    assert effects.reverb_mix == 0.3
    
    print("✓ Parameter setting test passed")


def test_effects_parameter_clamping():
    """Test that parameters are clamped to valid ranges"""
    print("Testing parameter clamping...")
    effects = GuitarAmpEffects()
    
    # Test out-of-range values
    effects.set_parameters(
        gain=10.0,  # Should clamp to 5.0
        distortion=2.0,  # Should clamp to 1.0
        bass=-5.0,  # Should clamp to -1.0
        treble=5.0,  # Should clamp to 1.0
    )
    
    assert effects.gain == 5.0
    assert effects.distortion == 1.0
    assert effects.bass == -1.0
    assert effects.treble == 1.0
    
    print("✓ Parameter clamping test passed")


def test_gain_processing():
    """Test gain processing"""
    print("Testing gain processing...")
    effects = GuitarAmpEffects()
    
    # Create test signal
    test_signal = np.array([0.1, 0.2, 0.3, 0.4, 0.5])
    
    # Test unity gain
    effects.set_parameters(gain=1.0)
    result = effects.apply_gain(test_signal)
    np.testing.assert_array_almost_equal(result, test_signal)
    
    # Test 2x gain
    effects.set_parameters(gain=2.0)
    result = effects.apply_gain(test_signal)
    expected = test_signal * 2.0
    np.testing.assert_array_almost_equal(result, expected)
    
    print("✓ Gain processing test passed")


def test_distortion_processing():
    """Test distortion effect"""
    print("Testing distortion processing...")
    effects = GuitarAmpEffects()
    
    # Create test signal
    test_signal = np.array([0.1, 0.2, 0.3, 0.4, 0.5])
    
    # Test no distortion
    effects.set_parameters(distortion=0.0)
    result = effects.apply_distortion(test_signal)
    np.testing.assert_array_almost_equal(result, test_signal)
    
    # Test with distortion
    effects.set_parameters(distortion=0.5)
    result = effects.apply_distortion(test_signal)
    
    # Result should be different from input
    assert not np.array_equal(result, test_signal)
    # Result should be in valid range
    assert np.all(result >= -1.0) and np.all(result <= 1.0)
    
    print("✓ Distortion processing test passed")


def test_complete_processing_chain():
    """Test complete effect processing chain"""
    print("Testing complete processing chain...")
    effects = GuitarAmpEffects()
    
    # Set some parameters
    effects.set_parameters(
        gain=2.0,
        distortion=0.3,
        bass=0.2,
        mid=0.1,
        treble=0.1,
        reverb_mix=0.2
    )
    
    # Create test signal
    test_signal = np.sin(2 * np.pi * 440 * np.arange(1024) / 44100) * 0.5
    
    # Process through complete chain
    result = effects.process(test_signal)
    
    # Check result is valid
    assert len(result) == len(test_signal)
    assert np.all(np.isfinite(result))
    
    print("✓ Complete processing chain test passed")


def test_amplifier_presets():
    """Test amplifier preset system"""
    print("Testing amplifier presets...")
    
    # Note: This test doesn't actually start audio
    amp = VirtualAmp()
    
    # Check presets exist
    presets = amp.list_presets()
    expected_presets = ['clean', 'crunch', 'overdrive', 'distortion', 'metal']
    
    for preset in expected_presets:
        assert preset in presets, f"Preset '{preset}' not found"
    
    # Test loading each preset
    for preset in expected_presets:
        amp.load_preset(preset)
        settings = amp.get_current_settings()
        assert settings['preset'] == preset
    
    print("✓ Amplifier presets test passed")


def test_amplifier_parameter_control():
    """Test amplifier parameter control"""
    print("Testing amplifier parameter control...")
    
    amp = VirtualAmp()
    
    # Set parameters
    amp.set_parameters(
        gain=3.0,
        distortion=0.6,
        bass=0.5
    )
    
    settings = amp.get_current_settings()
    assert settings['gain'] == 3.0
    assert settings['distortion'] == 0.6
    assert settings['bass'] == 0.5
    
    print("✓ Amplifier parameter control test passed")


def run_all_tests():
    """Run all tests"""
    print("=" * 60)
    print("  Running Virtual Amplifier Tests")
    print("=" * 60)
    print()
    
    tests = [
        test_effects_initialization,
        test_effects_parameter_setting,
        test_effects_parameter_clamping,
        test_gain_processing,
        test_distortion_processing,
        test_complete_processing_chain,
        test_amplifier_presets,
        test_amplifier_parameter_control,
    ]
    
    failed = 0
    for test in tests:
        try:
            test()
        except AssertionError as e:
            print(f"✗ Test failed: {test.__name__}")
            print(f"  Error: {e}")
            failed += 1
        except Exception as e:
            print(f"✗ Test error: {test.__name__}")
            print(f"  Error: {e}")
            failed += 1
    
    print()
    print("=" * 60)
    if failed == 0:
        print("  All tests passed! ✓")
    else:
        print(f"  {failed} test(s) failed")
    print("=" * 60)
    
    return failed == 0


if __name__ == "__main__":
    success = run_all_tests()
    sys.exit(0 if success else 1)
