#include <Arduino.h>
// void led_begin(byte pin) ←led関係の処理を初期化
// void led_start(double speed) ←　ledに光を流すのを始める
// void led_stop()
// void led_loop() ←ledの現在位置を計算し、前と変化していたらテープを更新する
// int led_getPosition() ←現在の光の位置
// void led_reset()

#define LED_PIN 3
#define BUTTON_PIN 4
#define TONE_PIN 5

void gameSetup() {
  led_begin();
  pinMode(BUTTON_PIN, INPUT_PULLUP);
}

int gameManager() {
  const double speedPlus = 15;
  const double startSpeed = 10;
  const int stopLinePosition = 50;
  const int rangeMax = 4;
  const int rangeMin = -4;

  double speed = startSpeed;

  const byte pointPlus = 10;
  int point = 0;

  byte counter = 1;
  const byte countMax = 10;
  while (counter <= countMax) {
    tone(TONE_PIN, 1000, 500);
    Serial.print(counter);
    Serial.println("回目");
    delay(500);

    Serial.println("Start!");
    led_reset();
    led_start(speed);

    while (led_getPosition() <= NUM_LEDS - 1) {
      led_loop();
      bool isPushed = !digitalRead(BUTTON_PIN);
      if (!isPushed) continue;
      led_stop();
      tone(TONE_PIN, 2000, 100);

      break;
    }
    delay(500);

    int position = led_getPosition();
    int difference = position - stopLinePosition;
    if (difference < rangeMin || rangeMax < difference) {
      Serial.println("ストップ失敗!");
      tone(TONE_PIN, 400, 1000);
      delay(1000);
      break;
    }

    Serial.println("ストップ成功!");
    tone(TONE_PIN, 1000, 100);
    delay(100);
    tone(TONE_PIN, 1000, 500);
    delay(500);

    point += pointPlus * counter;

    counter++;
    speed += speedPlus;
    delay(1000);
  }

  Serial.println("ゲーム終了");
  Serial.println("獲得ポイント: ");
  Serial.println(point);

  return point;
}
