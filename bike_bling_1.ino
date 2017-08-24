
// Burning Man bike bling
// 2017-08 Paul Williamson
//
// Using a 5V Adafruit Trinket.
//
// A strip of Adafruit DotStar RGB LED is connected to the hardware SPI
// (that's pin 1 and 2 on the Trinket).
//
// The first 31 LEDs run from the seatpost to the head tube, and the
// remaining 36 LEDs run from the head tube to the bottom bracket.
// The strip is duplicated on the other side of the bike. Both strips
// are connected to the same SPI pins, so they should always be
// doing the same thing.
//
// The strips are powered by a USB charger battery pack, nominally
// capable of 4.2 amps. At 20mA per LED element (60mA per pixel if all
// three elements are on), that's 70 pixels, or approximately half of
// the pixels on the bike. But at that rate of discharge, the battery
// won't last long. This code needs to be careful about that.
//
// An active-low button is connected to pin 0, and is used to change
// bling modes.

#include <Adafruit_DotStar.h>
// Because conditional #includes don't work w/Arduino sketches...
//#include <SPI.h>         // COMMENT OUT THIS LINE FOR GEMMA OR TRINKET
#include <avr/power.h> // ENABLE THIS LINE FOR GEMMA OR TRINKET
#include <EEPROM.h>

#define EEPROM_VERSION 1

#define LED_NUM_TOP 31
#define LED_NUM_DOWN 36
#define NUMPIXELS (LED_NUM_TOP+LED_NUM_DOWN) // Number of LEDs in strip
#define LED_TOP_SEAT  0
#define LED_TOP_HEAD  30
#define LED_DOWN_HEAD 31
#define LED_DOWN_BB   66
#define LED_TOP_STEP_FORWARD  1
#define LED_TOP_STEP_BACKWARD (-1)
#define LED_DOWN_STEP_FORWARD (-1)
#define LED_DOWN_STEP_BACKWARD  1

#define LED_TOP_FROM_FRONT(x) (LED_TOP_HEAD+x*LED_TOP_STEP_BACKWARD)
#define LED_TOP_FROM_BACK(x)  (LED_TOP_SEAT+x*LED_TOP_STEP_FORWARD)
#define LED_DOWN_FROM_FRONT(x) (LED_DOWN_HEAD+x*LED_DOWN_STEP_BACKWARD)
#define LED_DOWN_FROM_BACK(x)  (LED_DOWN_BB+x*LED_DOWN_STEP_FORWARD)

// Using the constructor flavor for hardware SPI
Adafruit_DotStar strip = Adafruit_DotStar(NUMPIXELS, DOTSTAR_BRG);

uint8_t bling_mode;

void bling_simple_1() {
  static int pos = 0;
  int width = 5;
  uint32_t color;
  
  if (bling_mode == 0)
    color = 0xFF0000;      // 'On' color (starts red)
  else
    color = 0x00FF00;
    
  int tail = pos - width;
  if (pos < LED_NUM_TOP)
    setPixelColorSafe(LED_TOP_FROM_FRONT(pos), color); // 'On' pixel at pos
  if (tail >= 0 && tail < LED_NUM_TOP)
    setPixelColorSafe(LED_TOP_FROM_FRONT(tail), 0);     // 'Off' pixel at tail
  if (pos < LED_NUM_DOWN)
    setPixelColorSafe(LED_DOWN_FROM_FRONT(pos), color);
  if (tail >= 0 && tail < LED_NUM_DOWN)
    setPixelColorSafe(LED_DOWN_FROM_FRONT(tail), 0);
  
  strip.show();                     // Refresh strip
  delay(20);                        // Pause 20 milliseconds (~50 FPS)

  pos++;
  if (tail > LED_NUM_TOP && tail > LED_NUM_DOWN) {
    pos = 0;
    // maybe change color here!
  }
}

typedef void (*bling_fn_ptr_t)(void);
bling_fn_ptr_t bling_modes[] = {
    bling_simple_1,     // red
    bling_simple_1,     // green?  Just to test button function.
    NULL
    };

void setup() {

#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000L)
  clock_prescale_set(clock_div_1); // Enable 16 MHz on Trinket
#endif

  strip.begin(); // Initialize pins for output
  strip.show();  // Turn all LEDs off ASAP
  
  if (EEPROM[0] != 'b' ||
      EEPROM[1] != 'i' ||
      EEPROM[2] != 'k' ||
      EEPROM[3] != 'e' ||
      EEPROM[4] != EEPROM_VERSION) {
    EEPROM[0] = 'b';
    EEPROM[1] = 'i';
    EEPROM[2] = 'k';
    EEPROM[3] = 'e';
    EEPROM[4] = EEPROM_VERSION;
    EEPROM[5] = 0;
    for (int i=6; i < EEPROM.length(); i++) {
        EEPROM.update(i, 0xFF);
        }
    }
  bling_mode = EEPROM[5];
}

void setPixelColorSafe(int n, uint32_t color) {
  if (n > 0 && n < NUMPIXELS)
    strip.setPixelColor(n, color);
}


void loop() {
    bling_modes[bling_mode]();
    
    if (button_hit()) {
        bling_mode++;
        if (bling_modes[bling_mode] == NULL)
            bling_mode = 0;
        }
}
