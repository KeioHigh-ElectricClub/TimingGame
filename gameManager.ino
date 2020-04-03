
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
  led_begin(LED_PIN);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
}

int gameManager() {
  const double speedPlus = 10;
  const double startSpeed = 8;
  const int stopLinePosition = 20;
  const byte rangeMax = 2;
  const byte rangeMin = -2;

  double speed = startSpeed;

  const byte pointPlus = 10;
  int point = 0;

  byte counter = 1;

  while (true) {
    tone(TONE_PIN, 1000, 500);
    delay(500);

    led_start(speed);

    while (led_getPosition() > 0) {
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
      tone(TONE_PIN, 400, 1000);
      delay(1000);
      break;
    }

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
  Serial.printn("獲得ポイント: ");
  Serial.println(point);

  return point;
}
