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
  const double speedPlus = 8;
  const double startSpeed = 10;
  const int stopLinePosition = 50;
  const int rangeMax = 2;
  const int rangeMin = -2;

  double speed = startSpeed;

  const byte pointPlus = 10;
  int point = 0;

  byte counter = 1;
  const byte countMax = 10;
  while (counter <= countMax) {
    led_reset();
    led_turnon_yellow(stopLinePosition - 1);
    led_turnon_yellow(stopLinePosition);
    led_turnon_yellow(stopLinePosition + 1);

    Serial.print(counter);
    Serial.println("回目");

    player.playMp3Folder(counter);

    delay(1500);

    tone(TONE_PIN, 1000, 500);
    delay(1000);
    tone(TONE_PIN, 1000, 500);
    delay(1000);

    player.playMp3Folder(13);

    Serial.println("Start!");
    delay(1900);

    led_start(speed);

    while (led_getPosition() <= NUM_LEDS - 1) {
      led_loop();
      bool isPushed = !digitalRead(BUTTON_PIN);
      tone(TONE_PIN, 1000, 10);
      if (!isPushed) continue;
      led_stop();
      break;
    }
    delay(500);

    int position = led_getPosition();
    int difference = position - stopLinePosition;
    if (difference < rangeMin || rangeMax < difference) {
      Serial.println("ストップ失敗!");
      player.playMp3Folder(17);
      delay(1500);
      player.playMp3Folder(15);
      delay(1600);
      break;
    }

    Serial.println("ストップ成功!");
    player.playMp3Folder(16);

    point += pointPlus + rangeMax - abs(difference);

    Serial.print("判定点からの差: ");
    Serial.println(difference);
    Serial.print("現在のポイント: ");
    Serial.println(point);

    counter++;
    speed += speedPlus;

    if (counter > 10) {
      player.playMp3Folder(11);
      delay(1000);
    }
    delay(1000);
  }

  Serial.println("ゲーム終了");
  Serial.println("獲得ポイント: ");
  Serial.println(point);

  return point;
}
