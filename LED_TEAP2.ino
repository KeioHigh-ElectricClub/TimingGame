#include <Arduino.h>
#include <FastLED.h>

#define NUM_LEDS 60  // LEDの数（仮）
#define DATA_PIN 6   //ピン番号

unsigned long time;  //光始めてからの時間
double speedMillis;
int position;
bool isFlowing;

CRGB leds[NUM_LEDS];

void led_begin() {
  //セットアップ

  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
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

void led_turnon_yellow(int position) {
  leds[position] = CRGB::Yellow;
  FastLED.show();
}

void led_loop() {
  // if ((millis() - time) / speedMillis != position && isFlowing) {
  int calcPosition = speedMillis * (millis() - time);
  if (calcPosition != position && isFlowing) {
    position = calcPosition;
    if (NUM_LEDS <= position) return;
    if (position >= 2) {
      leds[position - 2] = CRGB(0, 0, 0);
    }
    if (position + 1 < NUM_LEDS) {
      leds[position + 1] = CRGB(255, 0, 0);
    }

    FastLED.show();
  }
}

void led_reset() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
  }
  FastLED.show();
  position = 0;
}

int led_getPosition() { return position; }
