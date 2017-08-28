
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

#define LED_NUM_TOP 31
#define LED_NUM_DOWN 36
#define NUMPIXELS (LED_NUM_TOP+LED_NUM_DOWN) // Number of LEDs in strip
#define LED_TOP_SEAT  0
#define LED_TOP_HEAD  30
#define LED_DOWN_HEAD 31
#define LED_DOWN_BB   66
#define LED_TOP_STEP_FORWARD  1
#define LED_TOP_STEP_BACKWARD -1
#define LED_DOWN_STEP_FORWARD -1
#define LED_DOWN_STEP_BACKWARD  1

#define LED_TOP_FROM_FRONT(x) (LED_TOP_HEAD+x*LED_TOP_STEP_BACKWARD)
#define LED_TOP_FROM_BACK(x)  (LED_TOP_SEAT+x*LED_TOP_STEP_FORWARD)
#define LED_DOWN_FROM_FRONT(x) (LED_DOWN_HEAD+x*LED_DOWN_STEP_BACKWARD)
#define LED_DOWN_FROM_BACK(x)  (LED_DOWN_BB+x*LED_DOWN_STEP_FORWARD)

// Using the constructor flavor for hardware SPI
Adafruit_DotStar strip = Adafruit_DotStar(NUMPIXELS, DOTSTAR_BRG);


#define BUTTON_PIN  0

uint8_t bling_mode = 0;


void setup() {

#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000L)
  clock_prescale_set(clock_div_1); // Enable 16 MHz on Trinket
#endif

  strip.begin(); // Initialize pins for output
  strip.show();  // Turn all LEDs off ASAP

  pinMode(BUTTON_PIN, INPUT);
  digitalWrite(BUTTON_PIN, HIGH); // pullup

  
}

void setPixelColorSafe(int n, uint32_t color) {
  if (n > 0 && n < NUMPIXELS)
    strip.setPixelColor(n, color);
}

int pos = 0;
int width = 5;

void bling_mode_simple(void) {
  uint32_t color;      // 'On' color

  if (bling_mode == 0)
    color = 0xFF0000;
  else if (bling_mode == 1)
    color = 0x00FF00;
  else
    color = 0x0000FF;
    
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


void sparkle_bling(void) {
  static uint8_t pixel;
  static uint8_t state = 0;
  
  if (state == 0) {
    pixel = random() % NUMPIXELS;
    strip.setPixelColor(pixel, 0xFFFFFF); // turn on a new pixel
    state = 1;
    strip.show();
    delay(5);
    }
  else if (state == 1) {
    strip.setPixelColor(pixel, 0x000000); // turn off the previous pixel
    state = 0;
    strip.show();
    delay(5);
    }
  else {
    strip.clear();
    state = 0;
    }
}

int prev_pixel(int pixel) {
  pixel--;
  if (pixel < 0)
    pixel = NUMPIXELS-1;
  return(pixel);
}

int next_pixel(int pixel) {
  pixel++;
  if (pixel >= NUMPIXELS)
    pixel = 0;
  return(pixel);
}

void dualchase_bling(void) {
#define DUALCHASE_WIDTH1 7
#define DUALCHASE_WIDTH2 2
  static int front1 = 0, front2 = 0;
  int pixel;
  int i;
  
  strip.clear();
  // front1 leads a chase toward higher pixel numbers
  front1 = next_pixel(front1);
  pixel = front1;
  
  for (i = 0; i < DUALCHASE_WIDTH1; i++) {
    strip.setPixelColor(pixel, 0xFF0000);
    pixel = prev_pixel(pixel);
    }
  
  // front2 leads a chase toward lower pixel numbers
  front2 = prev_pixel(front2);
  front2 = prev_pixel(front2);
  pixel = front2;
  for (i = 0; i < DUALCHASE_WIDTH2; i++) {
    strip.setPixelColor(pixel, 0x00FF00 | strip.getPixelColor(pixel));
    pixel = next_pixel(pixel);
    }
  strip.show();
  delay(20);
}

void candycane_bling(void) {
  static int length = 4;
  static int length_increment = 1;
  static int width = 2;
  static int rotation = 0;
  int pixel;
  
  strip.clear();
  
  for (pixel = 0; pixel < length; pixel++) {
    if (((pixel+rotation) % (2*width)) < width) {
      strip.setPixelColor(pixel, 0x00FF00);
    } else {
      strip.setPixelColor(pixel, 0xFFFFFF);
    }
  }

  strip.show();
  delay(50);
  
  length += length_increment;
  if (length >= NUMPIXELS) {
    length_increment = -1;
    length += length_increment;
  } else if (length < 4) {
    length_increment = 1;
    length = 4;
    width++;
    if (width > 10) {
      width = 2;
    }
  }

  rotation++;
  if (rotation >= width*2) {
    rotation = 0;
  }

}

typedef void blingfunc(void);
blingfunc *bling_modes[] = {
  bling_mode_simple,  // green
  bling_mode_simple,  // red
  bling_mode_simple,  // blue
  sparkle_bling,
  dualchase_bling,
  candycane_bling,
  NULL};

 bool button_hit(void) {
  static unsigned long lastDebounceTime = 0;
  static unsigned long debounceDelay = 50;
  static int buttonState;
  static int lastButtonState;

  
  bool hit = 0;
  
  int reading = digitalRead(BUTTON_PIN);

  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == LOW) {
        hit = 1;
      }
    }
  }

  lastButtonState = reading;
  
  return hit;
 }

void loop(void) {
  bling_modes[bling_mode]();

  if (button_hit()) {
    bling_mode++;
    if (bling_modes[bling_mode] == NULL) {
      bling_mode = 0;
    }
    strip.clear();
  }
  
}

