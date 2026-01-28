"""
Virtual Guitar Amplifier
Combines audio engine and effects processing
"""

import numpy as np
from src.audio_engine import AudioEngine
from src.effects import GuitarAmpEffects


class VirtualAmp:
    """Virtual guitar amplifier with real-time processing"""
    
    def __init__(self, sample_rate: int = 44100, chunk_size: int = 1024):
        """
        Initialize the virtual amplifier
        
        Args:
            sample_rate: Sample rate in Hz
            chunk_size: Audio buffer size
        """
        self.audio_engine = AudioEngine(sample_rate, chunk_size)
        self.effects = GuitarAmpEffects(sample_rate)
        
        # Set up audio processing callback
        self.audio_engine.set_process_callback(self._process_audio)
        
        # Amp presets
        self.presets = {
            'clean': {
                'gain': 1.0,
                'distortion': 0.0,
                'bass': 0.0,
                'mid': 0.0,
                'treble': 0.0,
                'reverb_mix': 0.2
            },
            'crunch': {
                'gain': 2.0,
                'distortion': 0.3,
                'bass': 0.2,
                'mid': 0.1,
                'treble': 0.1,
                'reverb_mix': 0.15
            },
            'overdrive': {
                'gain': 2.5,
                'distortion': 0.5,
                'bass': 0.1,
                'mid': 0.3,
                'treble': 0.2,
                'reverb_mix': 0.1
            },
            'distortion': {
                'gain': 3.0,
                'distortion': 0.7,
                'bass': 0.3,
                'mid': 0.2,
                'treble': 0.3,
                'reverb_mix': 0.1
            },
            'metal': {
                'gain': 4.0,
                'distortion': 0.9,
                'bass': 0.4,
                'mid': -0.2,
                'treble': 0.5,
                'reverb_mix': 0.05
            }
        }
        
        self.current_preset = 'clean'
        
    def _process_audio(self, audio: np.ndarray) -> np.ndarray:
        """Internal audio processing callback"""
        return self.effects.process(audio)
        
    def start(self):
        """Start the amplifier"""
        self.audio_engine.start()
        
    def stop(self):
        """Stop the amplifier"""
        self.audio_engine.stop()
        
    def cleanup(self):
        """Clean up resources"""
        self.audio_engine.cleanup()
        
    def load_preset(self, preset_name: str):
        """
        Load an amplifier preset
        
        Args:
            preset_name: Name of the preset to load
        """
        if preset_name not in self.presets:
            print(f"Preset '{preset_name}' not found. Available: {list(self.presets.keys())}")
            return
            
        self.effects.set_parameters(**self.presets[preset_name])
        self.current_preset = preset_name
        print(f"Loaded preset: {preset_name}")
        
    def set_parameters(self, **kwargs):
        """
        Set amplifier parameters
        
        Keyword Args:
            gain: Clean gain (0.1 - 5.0)
            distortion: Distortion amount (0.0 - 1.0)
            bass: Bass EQ (-1.0 to 1.0)
            mid: Mid EQ (-1.0 to 1.0)
            treble: Treble EQ (-1.0 to 1.0)
            reverb_mix: Reverb mix (0.0 - 1.0)
        """
        self.effects.set_parameters(**kwargs)
        
    def get_current_settings(self) -> dict:
        """Get current amplifier settings"""
        return {
            'preset': self.current_preset,
            'gain': self.effects.gain,
            'distortion': self.effects.distortion,
            'bass': self.effects.bass,
            'mid': self.effects.mid,
            'treble': self.effects.treble,
            'reverb_mix': self.effects.reverb_mix
        }
        
    def list_presets(self) -> list:
        """Get list of available presets"""
        return list(self.presets.keys())
