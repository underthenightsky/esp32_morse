#include <WiFi.h>
#include <WiFiUdp.h>

// --- Network Configuration ---
const char* ssid = "Airtel_Daphne";
const char* password = "JinxingPrimrose";
const int udpPort = 4210;
WiFiUDP udp;

// --- Timing Thresholds (in milliseconds) ---
// Based on our Python script's 150ms unit
const int DOT_DASH_THRESHOLD = 250; 
const int LETTER_GAP_THRESHOLD = 800; // Time silence to denote a new letter
const int WORD_GAP_THRESHOLD = 1500;  // Time silence to denote a new word
const int SENTENCE_GAP_THRESHOLD=2500;
// --- State Variables ---
unsigned long lastSignalTime = 0;
String currentMorseSequence = "";
bool letterPending = false;
bool spacePending = false;
bool endLinePending = false;
// --- Morse Code Dictionary ---
struct MorseChar {
  String morse;
  char ascii;
};

const int DICT_SIZE = 36;
MorseChar morseDict[DICT_SIZE] = {
  {".-", 'A'}, {"-...", 'B'}, {"-.-.", 'C'}, {"-..", 'D'}, {".", 'E'},
  {"..-.", 'F'}, {"--.", 'G'}, {"....", 'H'}, {"..", 'I'}, {".---", 'J'},
  {"-.-", 'K'}, {".-..", 'L'}, {"--", 'M'}, {"-.", 'N'}, {"---", 'O'},
  {".--.", 'P'}, {"--.-", 'Q'}, {".-.", 'R'}, {"...", 'S'}, {"-", 'T'},
  {"..-", 'U'}, {"...-", 'V'}, {".--", 'W'}, {"-..-", 'X'}, {"-.--", 'Y'},
  {"--..", 'Z'}, {"-----", '0'}, {".----", '1'}, {"..---", '2'}, {"...--", '3'},
  {"....-", '4'}, {".....", '5'}, {"-....", '6'}, {"--...", '7'}, {"---..", '8'},
  {"----.", '9'}
};

char decodeMorse(String sequence) {
  for (int i = 0; i < DICT_SIZE; i++) {
    if (morseDict[i].morse == sequence) {
      return morseDict[i].ascii;
    }
  }
  return '?'; // Return a question mark if the sequence isn't recognized
}

void setup() {
  Serial.begin(300);
  WiFi.mode(WIFI_STA); //Essential
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nConnected!");
  Serial.print("ESP32 IP Address: ");
  Serial.println(WiFi.localIP());

  // Start UDP
  udp.begin(udpPort);
  Serial.println("\nListening for Morse Code...\n");
}

void loop() {
  // 1. Check for incoming UDP packets
  int packetSize = udp.parsePacket();
  
  if (packetSize) {
    char packetBuffer[255];
    int len = udp.read(packetBuffer, 255);
    if (len > 0) packetBuffer[len] = 0;
    
    int duration = atoi(packetBuffer);//get the number stored in the UDP Packet
    //this number duration helps specify whether its a dash or a dot
    //no packets transferred for gaps between letters and words

    // Classify as dot or dash based on the gap
    if (duration < DOT_DASH_THRESHOLD) {
      currentMorseSequence += ".";
    } else {
      currentMorseSequence += "-";
    }
    
    // Update our timers and states
    lastSignalTime = millis();
    letterPending = true;
    spacePending = false; // We just got a signal, so we are inside a word
    endLinePending = false;
  }

  // 2. Check for silences (Timeouts)
  unsigned long timeSinceLastSignal = millis() - lastSignalTime;

  // Has enough silence passed to consider the letter complete?
  if (letterPending && (timeSinceLastSignal > LETTER_GAP_THRESHOLD)) {
    char decodedChar = decodeMorse(currentMorseSequence);
    Serial.print(decodedChar); // Print the translated English letter
    
    // Reset for the next letter
    currentMorseSequence = "";
    letterPending = false;
    spacePending = true; // Now we wait to see if this silence becomes a word gap
  }
  // Has enough silence passed to consider the sentence complete?
  if (endLinePending && (timeSinceLastSignal > SENTENCE_GAP_THRESHOLD)) {
    Serial.println(" "); // Print a space between words
    endLinePending = false; // Stop printing spaces
  }
  // Has enough silence passed to consider the word complete?
  if (spacePending && (timeSinceLastSignal > WORD_GAP_THRESHOLD)) {
    Serial.print(" "); // Print a space between words
    spacePending = false; // Stop printing spaces
    endLinePending = true;
  }
  
}