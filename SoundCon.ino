#include <SoftwareSerial.h>

#include "DFPlayer_Mini_Mp3.h"
SoftwareSerial mySerial(A1, A0);  // DFplayer用のRX, TXピンの指定

//#define busy_pin A2

//音量は、0~30
//曲番号は1~

void soundcon_setup() {
  mySerial.begin(9600);
  mp3_set_serial(mySerial);
  //  pinMode(busy_pin, INPUT);
}

void soundcon_play(byte volume, byte number) {
  mp3_stop();
  mp3_set_volume(volume);
  mp3_play(number);
}

void soundcon_stop() { mp3_stop(); }

void soundcon_pause(bool status) {
  if (status)
    mp3_pause();  // StatusはStatus＝＝１ということと同義
  else
    mp3_play();
}

void soundcon_loop(bool status) { mp3_single_loop(status); }

void soundcon_single_play(byte volume, byte number) {
  mp3_stop();
  mp3_set_volume(volume);
  mp3_single_play(number);
}
