#include <Arduino.h>
#include <FastLED.h>

#define NUM_LEDS 20  // LEDの数（仮）
#define DATA_PIN 6   //ピン番号

unsigned long time;  //光始めてからの時間
double speedMillis;
int position;
bool isFlowing;

CRGB leds[NUM_LEDS];

void led_begin(byte pin) {
  //セットアップ

  FastLED.addLeds<NEOPIXEL, pin>(leds, NUM_LEDS);
  FastLED.setBrightness(64);  //輝度を64に設定
  delay(500);
}

void led_start(double speed) {
  leds[0] = CRGB(255, 0, 0);
  leds[1] = CRGB(255, 0, 0);
  FastLED.show();
  time = millis();
  isFlowing = true;
  // speedMillis = 1.0 / speed * 1000;
  speedMillis = speed / 1000.0;
}

void led_stop() { isFlowing = false; }

void led_loop() {
  // if ((millis() - time) / speedMillis != position && isFlowing) {
  if (speedMillis * (millis() - time) != position && isFlowing) {
    position = speedMillis * (millis() - time);
    if (position >= 2) {
      leds[position - 2] = CRGB(0, 0, 0);
    }
    leds[position + 1] = CRGB(255, 0, 0);
    FastLED.show();
  }
}

void led_reset() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB(0, 0, 0);
  }
  FastLED.show();
}

int led_getPosition() { return position; }
