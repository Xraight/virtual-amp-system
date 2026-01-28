"""
Digital Signal Processing Effects for Virtual Guitar Amplifier
Includes distortion, tone controls, reverb, and other effects
"""

import numpy as np
from scipy import signal
from typing import Tuple


class GuitarAmpEffects:
    """Collection of guitar amplifier effects and DSP algorithms"""
    
    def __init__(self, sample_rate: int = 44100):
        """
        Initialize effects processor
        
        Args:
            sample_rate: Sample rate in Hz
        """
        self.sample_rate = sample_rate
        
        # Effect parameters
        self.gain = 1.0  # Clean gain (1.0 = unity)
        self.distortion = 0.0  # Distortion amount (0.0 - 1.0)
        self.bass = 0.0  # Bass EQ (-1.0 to 1.0)
        self.mid = 0.0  # Mid EQ (-1.0 to 1.0)
        self.treble = 0.0  # Treble EQ (-1.0 to 1.0)
        self.reverb_mix = 0.0  # Reverb wet/dry mix (0.0 - 1.0)
        
        # Reverb delay buffer
        self.reverb_buffer_size = int(0.05 * sample_rate)  # 50ms
        self.reverb_buffer = np.zeros(self.reverb_buffer_size)
        self.reverb_index = 0
        
    def apply_gain(self, audio: np.ndarray) -> np.ndarray:
        """Apply gain to the signal"""
        return audio * self.gain
        
    def apply_distortion(self, audio: np.ndarray) -> np.ndarray:
        """
        Apply distortion effect using soft clipping
        
        Args:
            audio: Input audio signal
            
        Returns:
            Distorted audio signal
        """
        if self.distortion == 0.0:
            return audio
            
        # Calculate distortion factor (1.0 to 50.0)
        dist_factor = 1.0 + self.distortion * 49.0
        
        # Apply pre-gain for distortion
        audio = audio * dist_factor
        
        # Soft clipping using hyperbolic tangent
        distorted = np.tanh(audio)
        
        # Compensate for level
        distorted = distorted / (1.0 + self.distortion * 0.5)
        
        return distorted
        
    def apply_eq(self, audio: np.ndarray) -> np.ndarray:
        """
        Apply 3-band EQ (bass, mid, treble)
        
        Args:
            audio: Input audio signal
            
        Returns:
            EQ'd audio signal
        """
        result = audio.copy()
        
        # Bass filter (low shelf around 100 Hz)
        if self.bass != 0.0:
            bass_gain = 1.0 + self.bass
            sos_bass = signal.butter(2, 100, 'low', fs=self.sample_rate, output='sos')
            bass_signal = signal.sosfilt(sos_bass, audio)
            result = result + (bass_signal - audio) * bass_gain
            
        # Treble filter (high shelf around 3000 Hz)
        if self.treble != 0.0:
            treble_gain = 1.0 + self.treble
            sos_treble = signal.butter(2, 3000, 'high', fs=self.sample_rate, output='sos')
            treble_signal = signal.sosfilt(sos_treble, audio)
            result = result + (treble_signal - audio) * treble_gain
            
        # Mid boost/cut (bandpass around 800 Hz)
        if self.mid != 0.0:
            mid_gain = 1.0 + self.mid
            sos_mid = signal.butter(2, [400, 1600], 'band', fs=self.sample_rate, output='sos')
            mid_signal = signal.sosfilt(sos_mid, audio)
            result = result + mid_signal * mid_gain * 0.5
            
        return result
        
    def apply_reverb(self, audio: np.ndarray) -> np.ndarray:
        """
        Apply simple reverb effect using delay buffer
        
        Args:
            audio: Input audio signal
            
        Returns:
            Audio with reverb applied
        """
        if self.reverb_mix == 0.0:
            return audio
            
        output = np.zeros_like(audio)
        
        for i in range(len(audio)):
            # Get delayed sample
            delayed = self.reverb_buffer[self.reverb_index]
            
            # Mix input with feedback
            self.reverb_buffer[self.reverb_index] = audio[i] + delayed * 0.3
            
            # Mix dry and wet signal
            output[i] = audio[i] * (1.0 - self.reverb_mix) + delayed * self.reverb_mix
            
            # Advance buffer index
            self.reverb_index = (self.reverb_index + 1) % self.reverb_buffer_size
            
        return output
        
    def process(self, audio: np.ndarray) -> np.ndarray:
        """
        Process audio through the complete effect chain
        
        Args:
            audio: Input audio signal
            
        Returns:
            Processed audio signal
        """
        # Effect chain order: Gain -> Distortion -> EQ -> Reverb
        processed = self.apply_gain(audio)
        processed = self.apply_distortion(processed)
        processed = self.apply_eq(processed)
        processed = self.apply_reverb(processed)
        
        return processed
        
    def set_parameters(self, **kwargs):
        """
        Set effect parameters
        
        Keyword Args:
            gain: Clean gain (0.1 - 5.0)
            distortion: Distortion amount (0.0 - 1.0)
            bass: Bass EQ (-1.0 to 1.0)
            mid: Mid EQ (-1.0 to 1.0)
            treble: Treble EQ (-1.0 to 1.0)
            reverb_mix: Reverb mix (0.0 - 1.0)
        """
        if 'gain' in kwargs:
            self.gain = np.clip(kwargs['gain'], 0.1, 5.0)
        if 'distortion' in kwargs:
            self.distortion = np.clip(kwargs['distortion'], 0.0, 1.0)
        if 'bass' in kwargs:
            self.bass = np.clip(kwargs['bass'], -1.0, 1.0)
        if 'mid' in kwargs:
            self.mid = np.clip(kwargs['mid'], -1.0, 1.0)
        if 'treble' in kwargs:
            self.treble = np.clip(kwargs['treble'], -1.0, 1.0)
        if 'reverb_mix' in kwargs:
            self.reverb_mix = np.clip(kwargs['reverb_mix'], 0.0, 1.0)
