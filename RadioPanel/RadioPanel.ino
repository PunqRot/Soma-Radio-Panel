//
// Internet Radio Panel
//

#include <Adafruit_NeoPixel.h>
#include <ClickEncoder.h>
#include <TimerOne.h>

#define PIX_PIN 3
#define PIN_COUNT 7
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIN_COUNT, PIX_PIN, NEO_GRB + NEO_KHZ800);

// Serial buffer size
#define BUFFSIZE 32

// Rotary encoders for volume and channel
ClickEncoder *station;
ClickEncoder *volume;
int16_t lastVol, currentVol;
int16_t lastCh, currentCh;

// Variables for the button presses.
int clickCount=0;
int holdCount=0;

// Setup interrupts for encoders
void timerIsr() {
  station->service();
  volume->service();
}

void setup() {
  // Neopixel Strip Setup
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  //rainbow(10);

  // For debugging via Serial Monitor
  Serial.begin(9600);
  
  // Create both rotary encoders
  station = new ClickEncoder(A1, A0, 13, 4);
  volume = new ClickEncoder(A2, A3, 12);
  Timer1.initialize(1000);
  Timer1.attachInterrupt(timerIsr); 
  
  // Initial values of globals
  lastCh = -1;
  lastVol = -1;
  
} // void setup()

void loop() {  
  int l;
  char n;
  int dH, dT, dO;
  int s;

  sendStation();
  sendVolume();
  processVolButton();
  processChButton();

  char serialdata[BUFFSIZE+3];
  int lf = 10;
  if(Serial.available()) {
    Serial.readBytesUntil(lf, serialdata, BUFFSIZE);
    n = serialdata[0];
    blankBoard();
    
    switch(n) {
      case 'C':
          l = serialdata[1] - 48;
          Serial.print("Setting Channel to: ");
          Serial.println(l);
          displayChannel(l);
        break;
      
      case 'V':
          dT = serialdata[1] - 48;
          dO = serialdata[2] - 48;
          l = dT*10 + dO;
          Serial.print("Setting volume to: ");
          Serial.println(l);
          displayVolume(l);
        break;
      
      case 'R':
          dH = serialdata[1] - 48;
          dT = serialdata[2] - 48;
          dO = serialdata[3] - 48;
          l = dH*100 + dT*10 + dO;
          Serial.print("Rainbow Speed: ");
          Serial.println(l);
          rainbow(l);
        break;

      case 'r':
          dH = serialdata[1] - 48;
          dT = serialdata[2] - 48;
          dO = serialdata[3] - 48;
          l = dH*100 + dT*10 + dO;
          Serial.print("Cycle Speed: ");
          Serial.println(l);
          rainbowCycle(l);
        break;

      case 'L':
          dH = serialdata[1] - 48;
          dT = serialdata[2] - 48;
          dO = serialdata[3] - 48;
          l = dH*100 + dT*10 + dO;
          Serial.print("Larson Scanner: ");
          Serial.println(l);
          larsonScanner(l);
        break;

      default:
          blankBoard();
        break;
    } // switch
  } // if serial available
 
  
} // void loop()


void sendVolume() {
  currentVol += volume->getValue();
  
  if (currentVol != lastVol) {
    char vector;
    if(currentVol > lastVol) {
      vector = '+';
    } else {
      vector = '-';
    }
    lastVol = currentVol;
    
    // Send the volume vector
  
    // Debug may have to go for Serial communication
    Serial.print("Volume: ");
    Serial.print(vector);
    Serial.print(" ");
    Serial.println(currentVol);
  }
} // void sendVolume()  

void sendStation() {
  currentCh += station->getValue();

  if (currentCh != lastCh) {
    char vector;
    if(currentCh > lastCh) {
      vector = '+';
    } else {
      vector = '-';
    }
    lastCh = currentCh;
    
    // Send the channel vector
  
    // Debug may have to go for Serial communication
    Serial.print("Channel: ");
    Serial.print(vector);
    Serial.print(" ");
    Serial.println(currentCh);

  }
} // void sendVolume()  

void displayVolume(int volume) {
  Serial.print("Setting volume to: ");
  Serial.println(volume);
  
  int volInc = 14; // roughly 1/7th of 100
  int volFull = volume / volInc; // How many at full brightness
  int volRem = volume % volInc;
  int volPart = volRem * 128 / volInc;

  Serial.print("Number full: ");
  Serial.println(volFull);
  Serial.print("Number Rem: ");
  Serial.println(volRem);
  Serial.print("Number part: ");
  Serial.println(volPart);

  int i = 0; // iniitalize the loop but keep the scope outside for the after bit  
  for(i=0 ; i < volFull ; i++) {
    strip.setPixelColor(i, strip.Color(0, 128, 0));
  }
  strip.setPixelColor(i, strip.Color(0, volPart, 0)); 
  strip.show();
} // void displayVolume()

void displayChannel(int channel) {
  strip.setPixelColor(channel-1, strip.Color(128, 0, 0));
  strip.show();
} // void displayChannel()

void processVolButton() {
  static int holdCount=0;
  
  ClickEncoder::Button b = volume->getButton();
  if (b != ClickEncoder::Open) {
    Serial.print("Button: ");
    #define VERBOSECASE(label) case label: Serial.println(#label); break;
    switch (b) {
      VERBOSECASE(ClickEncoder::Pressed);
      //VERBOSECASE(ClickEncoder::Held)
      //VERBOSECASE(ClickEncoder::Released)
      case ClickEncoder::Held:
          if(holdCount > 5000) {
            Serial.println("Initiate Shutdown");
          } else {
            holdCount++;
            Serial.print("Held for: ");
            Serial.println(holdCount);
          }
        break;
      case ClickEncoder::Released:
          holdCount = 0;
        break;      
      //VERBOSECASE(ClickEncoder::Clicked)
      case ClickEncoder::Clicked:
          clickCount++;
          //showPixel(value);
          Serial.println("Clicked");
          Serial.print("Count: ");
          Serial.println(clickCount);
        break;
      case ClickEncoder::DoubleClicked:
          Serial.println("ClickEncoder::DoubleClicked");
          station->setAccelerationEnabled(!station->getAccelerationEnabled());
          Serial.print("  Acceleration is ");
          Serial.println((station->getAccelerationEnabled()) ? "enabled" : "disabled");
        break;
    } // switch (b)
  } // if (b != ClickEncoder::Open)
} // void processVolButton()

void processChButton() {}

void showPixel(int which) {
  uint32_t pixelColor;
  
  // Clear the board
  for(int i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, strip.Color(0,0,0));
  }
  
  switch(clickCount % 3) {
    case 0:
      pixelColor = strip.Color(64,0,0);
      break;
      
    case 1:
      pixelColor = strip.Color(0,64,0);
      break;

    case 2:
      pixelColor = strip.Color(0,0,64);
      break;
      
     default:
      pixelColor = strip.Color(128,128,128);
  }
  
  // Set the pixel
  strip.setPixelColor(which, pixelColor);
  strip.show();

  Serial.print("Setting pixel ");
  Serial.print(which);
  Serial.print(" to ");
  Serial.println(pixelColor);  
} // void showPixel()

//
// NEOPIXEL CODE BITS
//

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) { // 1 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void blankBoard() {
  for(int i=0 ; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, strip.Color(0,0,0));
  }
  strip.show();
}

void larsonScanner(int times) {
  for(int t=0; t<times; t++) {
    for(int i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, strip.Color(255,0,0));
      strip.setPixelColor(i-1, strip.Color(64,0,0));
      strip.setPixelColor(i-2, strip.Color(16,0,0));
      strip.setPixelColor(i-3, strip.Color(0,0,0));
      strip.show();
      if(i<strip.numPixels()-1) {
        delay(100);
      }
      blankBoard();
    }
    for(int i=strip.numPixels()-1; i>-1; i--) {
      strip.setPixelColor(i, strip.Color(255,0,0));
      strip.setPixelColor(i+1, strip.Color(64,0,0));
      strip.setPixelColor(i+2, strip.Color(16,0,0));
      strip.setPixelColor(i+3, strip.Color(0,0,0));
      strip.show();
      if(i>0) {
        delay(100);
      }
      blankBoard();
    }
  }
}
