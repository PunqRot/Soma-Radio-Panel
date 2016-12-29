//
// Internet Radio Panel
//

#include <Adafruit_NeoPixel.h>
#include <ClickEncoder.h>
#include <TimerOne.h>

#define PIX_PIN 3
#define PIN_COUNT 7
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIN_COUNT, PIX_PIN, NEO_GRB + NEO_KHZ800);
int panelLights[7][3];

// Serial buffer size
#define BUFFSIZE 32
String inputString;
bool stringComplete=0;

// Rotary encoders for volume and channel
ClickEncoder *station;
ClickEncoder *volume;
int16_t lastVol, currentVol;
int16_t lastCh, currentCh;

// Setup interrupts for encoders
void timerIsr() {
  station->service();
  volume->service();
}

//
// SETUP
//
void setup() {
  // Neopixel Strip Setup
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  // For debugging via Serial Monitor
  Serial.begin(9600);
  
  // Create both rotary encoders
  station = new ClickEncoder(A1, A0, 4, 4);
  volume = new ClickEncoder(A2, A3, 12, 4);
  Timer1.initialize(1000);
  Timer1.attachInterrupt(timerIsr); 
  
  // Initial values of globals
  lastCh = -1;
  lastVol = -1;
  
} // void setup()

//
// LOOP
//
void loop() {  

  //
  // Pi I/O - Or.. Pi/O?
  //
  while(Serial.available() > 0) {

    for(int x=0 ; x < 7 ; x++) {
      for(int y=0 ; y < 3 ; y++) {
        int val = Serial.parseInt();
        panelLights[x][y] = val;
        Serial.print("Setting [");
        Serial.print(x);
        Serial.print("][");
        Serial.print(y);
        Serial.print("] to ");
        Serial.print(val, DEC);
        Serial.println();
      }
      strip.setPixelColor(x, strip.Color(panelLights[x][0], panelLights[x][1], panelLights[x][2]));
      strip.show();
    }
    
    if(Serial.read() == '\n') {
      for(int a=0 ; a < strip.numPixels() ; a++) {
        Serial.print("Setting ");
        Serial.print(a);
        Serial.print(" to ");
        Serial.print(panelLights[a][0], DEC);
        Serial.print(" ");
        Serial.print(panelLights[a][1], DEC);
        Serial.print(" ");
        Serial.print(panelLights[a][2], DEC);
        Serial.print(" ");
        //Serial.print(strip.Color(panelLights[a][0], panelLights[a][1], panelLights[a][2]), HEX);
        Serial.println();
        strip.setPixelColor(a, strip.Color(panelLights[a][0], panelLights[a][1], panelLights[a][2]));  
      }
      strip.show();
    } // if carriage return
  } // if serial available

  //
  // Transmit Volume  
  //
  currentVol += volume->getValue();
  if (currentVol != lastVol) {
    char volVector;
    if(currentVol > lastVol) {
      volVector = 'V';
    } else {
      volVector = 'v';
    }
    lastVol = currentVol;    
    Serial.print(volVector);
  }

  //
  // Send Station
  //
  currentCh += station->getValue();
  if (currentCh != lastCh) {
    char vector;
    if(currentCh > lastCh) {
      vector = 'C';
    } else {
      vector = 'c';
    }
    lastCh = currentCh;    
    Serial.print(vector);
  } // if channel change
  

} // Loop

//
// Process input
//
void serialEvent() {
  while (Serial.available()) {
    int inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n') {
      stringComplete = true;
    }
  }
}
