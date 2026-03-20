import time
import socket

# --- Network Configuration ---
ESP32_IP = "192.168.1.7"  # Replace with your ESP32's IP
UDP_PORT = 4210
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# --- Timing Configuration ---
UNIT_MS = 150  # Base time unit in milliseconds
DOT_TIME = UNIT_MS
DASH_TIME = UNIT_MS * 3

# --- Morse Dictionary ---
morse_dict = {
    'A': '.-', 'B': '-...', 'C': '-.-.', 'D': '-..', 'E': '.', 'F': '..-.',
    'G': '--.', 'H': '....', 'I': '..', 'J': '.---', 'K': '-.-', 'L': '.-..',
    'M': '--', 'N': '-.', 'O': '---', 'P': '.--.', 'Q': '--.-', 'R': '.-.',
    'S': '...', 'T': '-', 'U': '..-', 'V': '...-', 'W': '.--', 'X': '-..-',
    'Y': '-.--', 'Z': '--..', '0': '-----', '1': '.----', '2': '..---', 
    '3': '...--', '4': '....-', '5': '.....', '6': '-....', '7': '--...', 
    '8': '---..', '9': '----.'
}

def send_udp_signal(duration):
    """Sends the (numerical) time duration of the 'press' to the ESP32 , just like 150 for a dot and 450 for a dash"""
    message = str(duration).encode('utf-8')
    sock.sendto(message, (ESP32_IP, UDP_PORT))

def transmit_sentence(sentence):
    print(f"\n--- Transmitting: '{sentence}' ---")
    sentence = sentence.upper()
    
    for char in sentence:
        if char == ' ':
            # Pause between words (8 units total). 
            print("(Word Space)")
            time.sleep((UNIT_MS * 8) / 1000.0) 
            continue
            
        if char in morse_dict:
            morse_seq = morse_dict[char]
            print(f"Sending '{char}': {morse_seq}")
            
            for i, symbol in enumerate(morse_seq):
                # 1. Simulate holding the key down
                duration = DOT_TIME if symbol == '.' else DASH_TIME
                time.sleep(duration / 1000.0)
                
                # 2. Key released, send the packet to ESP32
                send_udp_signal(duration)
                
                # 3. Pause between symbols in the same letter (1 unit)
                if i < len(morse_seq) - 1:
                    time.sleep(UNIT_MS / 1000.0)
            
            # 4. Pause between distinct letters (5 units)
            time.sleep((UNIT_MS * 5) / 1000.0)

# --- Test Data ---
sample_sentences = [
    "SOS",
    "HELLO ESP32",
    "AI ON THE EDGE"
]

print("Starting automated telegraph transmission...")

for text in sample_sentences:
    transmit_sentence(text)
    time.sleep(2.5) # Wait 2.5 seconds before sending the next sentence

print("\nTransmission complete!")
