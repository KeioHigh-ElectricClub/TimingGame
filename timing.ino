#include <Arduino.h>
#include <MFRC522.h>

void setup() {
  Serial.begin(9600);
  while (!Serial) {
  };
  gameSetup();
  nfc_setup();
  //  ゲーム初期化関数
}
void loop() {
  nfc_reduce_credit();

  byte point = gameManager();

  nfc_write_point(point);
  nfc_reset();
}
