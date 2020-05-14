#include <Arduino.h>
#include <DFRobotDFPlayerMini.h>
#include <SoftwareSerial.h>
// #include <MFRC522.h>
DFRobotDFPlayerMini player;
SoftwareSerial serial(A0, A1);

void setup() {
  Serial.begin(9600);
  while (!Serial) {
  };
  gameSetup();
  // if (!nfc_setup()) {
  //   Serial.println("initializing NFC failed");
  // }
  player.begin(serial, false, true);
  player.volume(20);
  soundcon_setup();
  //  ゲーム初期化関数
}
void loop() {
  // nfc_reduce_credit(12);

  tone(5, 1000, 200);
  delay(200);

  byte point = gameManager();

  Serial.println("pls reset");

  player.playMp3Folder(12);
  delay(3000);
  player.playMp3Folder(14);

  while (true) {
  }

  // nfc_write_point(point, 12);
}
