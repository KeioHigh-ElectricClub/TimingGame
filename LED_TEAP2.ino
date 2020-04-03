#include <FastLED.h>

#define NUM_LEDS 20  // LEDの数（仮）
#define DATA_PIN 6   //ピン番号

unsigned long time;  //光始めてからの時間
int tt;
int m;
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
  tt = 1.0 / speed * 1000;
}

void led_stop() { isFlowing = false; }

void led_loop() {
  if ((millis() - time) / tt != m && isFlowing) {
    m = (millis() - time) / tt;
    if (m >= 2) {
      leds[m - 2] = CRGB(0, 0, 0);
    }
    leds[m + 1] = CRGB(255, 0, 0);
    FastLED.show();
  }
}

void led_reset() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB(0, 0, 0);
  }
  FastLED.show();
  time = 0;
}

int led_getPosition() { return m; }
