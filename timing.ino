#include <Arduino.h>
#include <MFRC522.h>

void setup() {
  Serial.begin(9600);
  while (!Serial) {
  };
  gameSetup();
  if (!nfc_setup()) {
    Serial.println("initializing NFC failed");
  }
  //  ゲーム初期化関数
}
void loop() {
  nfc_reduce_credit(12);

  byte point = gameManager();

  nfc_write_point(point, 12);
}
