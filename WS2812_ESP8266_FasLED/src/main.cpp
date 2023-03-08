#include <Arduino.h>

 #define FASTLED_ESP8266_NODEMCU_PIN_ORDER
#include <FastLED.h>

#define NUM_LEDS 1
#define PIN 2
// D4
#define COLOR_ORDER GRB

CRGBArray<NUM_LEDS> leds;

CRGBSet partA(leds(6, 9));   // Define custom pixel range with a name.
CRGBSet partB(leds(22, 25)); // Define custom pixel range with a name.

CRGB colorOne(0, 222, 255);  // Define a custom color.
CRGB colorTwo(82, 222, 255); // Define a custom color.

//---------------------------------------------------------------
void setup()
{
  Serial.begin(115200);
  while (!Serial)
    ; // wait for serial attach

  Serial.println();
  Serial.println("Initializing...");
  Serial.flush();

  FastLED.addLeds<WS2812, PIN, COLOR_ORDER>(leds, NUM_LEDS);

  Serial.println();
  Serial.println("Running...");
}

uint8_t gHue = 0;
uint8_t  gHueDelta = 3;
//---------------------------------------------------------------
void loop()
{
  delay(10);
  
  gHue++;
  
  CHSV color = CHSV(gHue, 255, 255);

  Serial.println(gHue);

 leds[0] = color;
 

  //leds[0] = leds[0].Blue;
  //leds[1] = leds[1].Red;
  //leds[2] = leds[2].Green;

  //partA[0]=CRGB::Green;

  // fill entire strip for some animted background color
  //fill_solid(leds, partA, colorOne);
  //fill_solid(leds, partB, CRGB::Red);

  FastLED.show();
}