import serial
import serial.tools.list_ports
import struct
import wave
import time
import argparse
import sys
import threading
import signal

# Configuration
BAUD_RATE = 2000000
MAGIC_WORD = b'\x5A\xA5' # Little Endian for 0xA55A
PACKET_SIZE = 245
PAYLOAD_SIZE = 240
SAMPLE_RATE = 8000 # Configured in app_config.h
CHANNELS = 1
SAMPLE_WIDTH = 2 # 16-bit

running = True

def signal_handler(sig, frame):
    global running
    print("\nStopping recording...")
    running = False

def list_ports():
    ports = serial.tools.list_ports.comports()
    return [p.device for p in ports]

def record_audio(port, output_filename):
    print(f"Opening {port} at {BAUD_RATE} baud...")
    try:
        ser = serial.Serial(port, BAUD_RATE, timeout=1)
        # Flush input buffer
        ser.reset_input_buffer()
    except Exception as e:
        print(f"Error opening serial port: {e}")
        return

    print("Syncing with stream... (Press Ctrl+C to stop)")
    
    audio_data = bytearray()
    rssi_log = []
    
    packet_count = 0
    start_time = 0 # timestamp of first packet in current batch
    last_packet_time = 0
    
    buffer = b''
    
    while running:
        # Read chunks
        if ser.in_waiting > 0:
            buffer += ser.read(ser.in_waiting)
        
        # Look for magic word
        while len(buffer) >= PACKET_SIZE:
            # Check for magic word at start
            # 0xA55A (Little Endian: 5A A5)
            # The firmware sends magic_word field which is 0xA55A.
            # In memory (Little Endian), it is 5A A5.
            # Python 'H' is unsigned short (2 bytes).
            
            # Simple sync: find 0x5A, 0xA5
            try:
                # Find occurrences of magic byte 1
                idx = buffer.find(b'\x5A')
                if idx == -1:
                    buffer = b'' # Discard
                    break
                
                # Check next byte
                if idx + 1 < len(buffer):
                    if buffer[idx+1] == 0xA5:
                        # Found Sync
                        if len(buffer) < idx + PACKET_SIZE:
                            break # Wait for more data
                            
                        # Extract Packet
                        packet_data = buffer[idx:idx+PACKET_SIZE]
                        buffer = buffer[idx+PACKET_SIZE:]
                        
                        # Parse Structure:
                        # Magic (2), Seq (2), RSSI (1), Payload (240)
                        # Format: <H H b 120h
                        
                        # Just grab bytes directly
                        seq_bytes = packet_data[2:4]
                        rssi_byte = packet_data[4]
                        payload_bytes = packet_data[5:245]
                        
                        seq = struct.unpack('<H', seq_bytes)[0]
                        rssi = struct.unpack('b', bytes([rssi_byte]))[0]
                        
                        # Append Audio
                        audio_data.extend(payload_bytes)
                        
                        # Log RSSI
                        rssi_log.append(rssi)
                        
                        packet_count += 1
                        last_packet_time = time.time()
                        
                        if packet_count % 50 == 0:
                            duration = packet_count * 7.5 / 1000.0 # 7.5ms per packet
                            print(f"\rRec: {duration:.1f}s | Pkts: {packet_count} | RSSI: {rssi} dBm", end="")
                            
                    else:
                        # Not magic, discard sync byte
                        buffer = buffer[idx+1:]
                else:
                    break # Wait for next byte
            except Exception as e:
                print(f"Parse Error: {e}")
                buffer = buffer[1:] # Skip 1 byte
        
        # Check for silence timeout (Auto-Save)
        if len(audio_data) > 0 and (time.time() - last_packet_time > 1.0):
            print("\nSilence detected (End of transmission). Saving file...")
            
            # Save Logic
            save_timestamp = int(start_time)
            save_wav(f"capture_{save_timestamp}.wav", audio_data)
            save_csv(f"capture_{save_timestamp}.csv", rssi_log)
            
            # Reset
            audio_data = bytearray()
            rssi_log = []
            packet_count = 0
            start_time = 0 # Will reset on next packet
            print("Ready! Waiting for next transmission...")
            
        # Update start time for next batch if new
        if len(audio_data) > 0 and start_time == 0:
            start_time = last_packet_time

        time.sleep(0.001)

    ser.close()
    
    # Save residual data if Ctrl+C pressed during recording
    if len(audio_data) > 0:
        print("\nSaving final incomplete recording...")
        save_wav(f"capture_{int(time.time())}_final.wav", audio_data)
        save_csv(f"capture_{int(time.time())}_final.csv", rssi_log)

def save_wav(filename, data):
    if not data: return
    try:
        with wave.open(filename, 'wb') as wf:
            wf.setnchannels(CHANNELS)
            wf.setsampwidth(SAMPLE_WIDTH)
            wf.setframerate(SAMPLE_RATE)
            wf.writeframes(data)
        print(f"Saved audio: {filename}")
    except Exception as e:
        print(f"Error saving WAV: {e}")

def save_csv(filename, log):
    if not log: return
    try:
        with open(filename, 'w') as f:
            f.write("PacketIndex,RSSI\n")
            for i, r in enumerate(log):
                f.write(f"{i},{r}\n")
        print(f"Saved RSSI log: {filename}")
    except Exception as e:
        print(f"Error saving CSV: {e}")

if __name__ == "__main__":
    signal.signal(signal.SIGINT, signal_handler)
    
    ports = list_ports()
    if not ports:
        print("No serial ports found!")
        sys.exit(1)
        
    print("Available ports:")
    for i, p in enumerate(ports):
        print(f"{i}: {p}")
        
    if len(ports) == 1:
        selected_port = ports[0]
    else:
        try:
            idx = int(input("Select port index: "))
            selected_port = ports[idx]
        except:
            print("Invalid selection")
            sys.exit(1)
            
    filename = f"capture_{int(time.time())}.wav"
    record_audio(selected_port, filename)
