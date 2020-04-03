#include <FastLED.h>

#define LED_NUM 240

CRGB leds[LED_NUM];
int tape_position = 0;
double tape_speed = 0;
bool tape_isFlowing = false;
unsigned long tape_timer = 0;
int tape_updateRate = 0;

void led_begin(byte pin) {
  FastLED.addLeds<NEOPIXEL, pin>(leds, LED_NUM);
  FastLED.showColor(CRGB::Black);
}

void led_start(double speed) {
  tape_position = LED_NUM - 1;
  tape_speed = speed;
  tape_isFlowing = true;
  tape_timer = micros();
  tape_updateRate = 1 / speed * 1000 * 1000;
}

void led_stop() { tape_isFlowing = false; }

void led_loop() {
  if (!tape_isFlowing) return;
  if (micros() - tape_timer < tape_updateRate) return;
  tape_position--;
  if (tape_position <= 0) return;

  leds[tape_position] = CRGB::Red;
  leds[tape_position + 1] = CRGB::Black;
  FastLED.show();
}

int led_getPositon() { return tape_position; }

void led_reset() {
  tape_position = 0;
  tape_speed = 0;
  tape_isFlowing = false;
  unsigned long tape_timer = 0;
  int tape_updateRate = 0;
  FastLED.showColor(CRGB::Black);
}