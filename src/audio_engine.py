"""
Audio Engine for Virtual Guitar Amplifier
Handles real-time audio input/output and processing
"""

import pyaudio
import numpy as np
from typing import Optional, Callable


class AudioEngine:
    """Real-time audio processing engine"""
    
    def __init__(
        self,
        sample_rate: int = 44100,
        chunk_size: int = 1024,
        channels: int = 1
    ):
        """
        Initialize the audio engine
        
        Args:
            sample_rate: Sample rate in Hz (default: 44100)
            chunk_size: Buffer size in samples (default: 1024)
            channels: Number of audio channels (default: 1 - mono)
        """
        self.sample_rate = sample_rate
        self.chunk_size = chunk_size
        self.channels = channels
        self.is_running = False
        
        self.audio = pyaudio.PyAudio()
        self.stream: Optional[pyaudio.Stream] = None
        self.process_callback: Optional[Callable] = None
        
    def set_process_callback(self, callback: Callable[[np.ndarray], np.ndarray]):
        """
        Set the audio processing callback function
        
        Args:
            callback: Function that takes input audio array and returns processed audio
        """
        self.process_callback = callback
        
    def _audio_callback(self, in_data, frame_count, time_info, status):
        """Internal callback for PyAudio stream"""
        # Note: Avoid I/O operations in audio callback for best performance
            
        # Convert input bytes to numpy array
        audio_data = np.frombuffer(in_data, dtype=np.float32)
        
        # Process audio if callback is set
        if self.process_callback:
            processed_data = self.process_callback(audio_data)
        else:
            processed_data = audio_data
        
        # Validate processed data length
        if len(processed_data) != len(audio_data):
            processed_data = audio_data
            
        # Ensure output is in valid range [-1, 1]
        processed_data = np.clip(processed_data, -1.0, 1.0)
        
        # Convert back to bytes
        output_bytes = processed_data.astype(np.float32).tobytes()
        
        return (output_bytes, pyaudio.paContinue)
        
    def start(self):
        """Start the audio stream"""
        if self.is_running:
            print("Audio engine already running")
            return
            
        self.stream = self.audio.open(
            format=pyaudio.paFloat32,
            channels=self.channels,
            rate=self.sample_rate,
            input=True,
            output=True,
            frames_per_buffer=self.chunk_size,
            stream_callback=self._audio_callback
        )
        
        self.is_running = True
        self.stream.start_stream()
        print(f"Audio engine started (SR: {self.sample_rate}Hz, Chunk: {self.chunk_size})")
        
    def stop(self):
        """Stop the audio stream"""
        if not self.is_running:
            return
            
        if self.stream:
            self.stream.stop_stream()
            self.stream.close()
            
        self.is_running = False
        print("Audio engine stopped")
        
    def cleanup(self):
        """Clean up audio resources"""
        self.stop()
        self.audio.terminate()
        
    def is_active(self) -> bool:
        """Check if audio stream is active"""
        return self.is_running and self.stream and self.stream.is_active()
