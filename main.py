#!/usr/bin/env python3
"""
Virtual Guitar Amplifier - Main Application
Command-line interface for the virtual amp system
"""

import sys
import time
import signal
from src.amplifier import VirtualAmp


class AmpCLI:
    """Command-line interface for virtual amplifier"""
    
    def __init__(self):
        self.amp = VirtualAmp(sample_rate=44100, chunk_size=1024)
        self.running = False
        
    def print_banner(self):
        """Print application banner"""
        print("=" * 60)
        print("  🎸 Virtual Guitar Amplifier System 🎸")
        print("=" * 60)
        print()
        
    def print_help(self):
        """Print help information"""
        print("\nAvailable Commands:")
        print("  preset <name>  - Load a preset (clean, crunch, overdrive, distortion, metal)")
        print("  gain <value>   - Set gain (0.1 - 5.0)")
        print("  dist <value>   - Set distortion (0.0 - 1.0)")
        print("  bass <value>   - Set bass EQ (-1.0 to 1.0)")
        print("  mid <value>    - Set mid EQ (-1.0 to 1.0)")
        print("  treble <value> - Set treble EQ (-1.0 to 1.0)")
        print("  reverb <value> - Set reverb mix (0.0 - 1.0)")
        print("  status         - Show current settings")
        print("  presets        - List available presets")
        print("  help           - Show this help")
        print("  quit           - Exit the application")
        print()
        
    def print_status(self):
        """Print current amplifier status"""
        settings = self.amp.get_current_settings()
        print("\n" + "=" * 60)
        print(f"Current Preset: {settings['preset'].upper()}")
        print("-" * 60)
        print(f"  Gain:       {settings['gain']:.2f}")
        print(f"  Distortion: {settings['distortion']:.2f}")
        print(f"  Bass:       {settings['bass']:.2f}")
        print(f"  Mid:        {settings['mid']:.2f}")
        print(f"  Treble:     {settings['treble']:.2f}")
        print(f"  Reverb:     {settings['reverb_mix']:.2f}")
        print("=" * 60 + "\n")
        
    def handle_command(self, command: str):
        """Handle user commands"""
        parts = command.strip().lower().split()
        
        if not parts:
            return True
            
        cmd = parts[0]
        
        if cmd in ['quit', 'exit', 'q']:
            return False
            
        elif cmd == 'help' or cmd == '?':
            self.print_help()
            
        elif cmd == 'status':
            self.print_status()
            
        elif cmd == 'presets':
            presets = self.amp.list_presets()
            print(f"\nAvailable presets: {', '.join(presets)}\n")
            
        elif cmd == 'preset':
            if len(parts) < 2:
                print("Usage: preset <name>")
            else:
                self.amp.load_preset(parts[1])
                
        elif cmd == 'gain':
            if len(parts) < 2:
                print("Usage: gain <value>")
            else:
                try:
                    value = float(parts[1])
                    self.amp.set_parameters(gain=value)
                    print(f"Gain set to {value:.2f}")
                except ValueError:
                    print("Invalid value. Must be a number.")
                    
        elif cmd in ['dist', 'distortion']:
            if len(parts) < 2:
                print("Usage: dist <value>")
            else:
                try:
                    value = float(parts[1])
                    self.amp.set_parameters(distortion=value)
                    print(f"Distortion set to {value:.2f}")
                except ValueError:
                    print("Invalid value. Must be a number.")
                    
        elif cmd == 'bass':
            if len(parts) < 2:
                print("Usage: bass <value>")
            else:
                try:
                    value = float(parts[1])
                    self.amp.set_parameters(bass=value)
                    print(f"Bass set to {value:.2f}")
                except ValueError:
                    print("Invalid value. Must be a number.")
                    
        elif cmd == 'mid':
            if len(parts) < 2:
                print("Usage: mid <value>")
            else:
                try:
                    value = float(parts[1])
                    self.amp.set_parameters(mid=value)
                    print(f"Mid set to {value:.2f}")
                except ValueError:
                    print("Invalid value. Must be a number.")
                    
        elif cmd == 'treble':
            if len(parts) < 2:
                print("Usage: treble <value>")
            else:
                try:
                    value = float(parts[1])
                    self.amp.set_parameters(treble=value)
                    print(f"Treble set to {value:.2f}")
                except ValueError:
                    print("Invalid value. Must be a number.")
                    
        elif cmd == 'reverb':
            if len(parts) < 2:
                print("Usage: reverb <value>")
            else:
                try:
                    value = float(parts[1])
                    self.amp.set_parameters(reverb_mix=value)
                    print(f"Reverb set to {value:.2f}")
                except ValueError:
                    print("Invalid value. Must be a number.")
                    
        else:
            print(f"Unknown command: {cmd}. Type 'help' for available commands.")
            
        return True
        
    def run(self):
        """Run the CLI application"""
        self.print_banner()
        
        print("Starting virtual amplifier...")
        print("Note: Make sure your audio input (guitar/mic) and output are configured.\n")
        
        # Load default preset
        self.amp.load_preset('clean')
        
        # Start the amplifier
        try:
            self.amp.start()
            self.running = True
        except Exception as e:
            print(f"Error starting amplifier: {e}")
            print("Make sure you have audio input/output devices available.")
            return
            
        print("\n✓ Amplifier is running!")
        self.print_help()
        self.print_status()
        
        # Main command loop
        try:
            while self.running:
                try:
                    command = input("amp> ")
                    if not self.handle_command(command):
                        break
                except EOFError:
                    break
                except KeyboardInterrupt:
                    print("\nUse 'quit' to exit.")
                    
        finally:
            print("\nStopping amplifier...")
            self.amp.stop()
            self.amp.cleanup()
            print("Goodbye! 🎸")


def signal_handler(sig, frame):
    """Handle Ctrl+C gracefully"""
    print("\nShutting down...")
    sys.exit(0)


if __name__ == "__main__":
    signal.signal(signal.SIGINT, signal_handler)
    
    cli = AmpCLI()
    cli.run()
