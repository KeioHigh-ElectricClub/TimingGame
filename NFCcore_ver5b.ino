#include <Arduino.h>
// 2020/03/19更新
// NFCcore_ver5新歓専用仕様に変更

#define SerialSpeed 9600  // 変更必須
#define boothNum 255      // 変更必須
#define RST_PIN 9  // NFC RSTピン(SPI通信時には設定必要)  変更必須
#define SS_PIN 10  // NFC SDAピン(SPI通信時には設定必要)  変更必須
#define defaultCredit 5  // デフォルトのクレジット数(変更可能)
/*新歓専用関数
   void nfc_setup()
     機能説明: Serial.beginとSPI.begin、PCD_Initが一体となった関数。
     　　　　　これさえ起動すればNFCモジュールとの接続が完了!

   bool nfc_reduce_credit()
     機能説明: ・NFCカードがかざされるまで待機する処理
     　　　　  ・読み込み/書き込み失敗時の再タッチ機能に対応
     　　　　  ・クレジット残高の自動引き落とし
            　・クレジット残高不足時のエラー表示
            　・カード情報の自動バックアップ機能に対応
            　・ApplePay等のおサイフケータイをかざした時のデモプレイ機能に対応
            　・筐体コードによる動作制限機能に対応
            　・操作番号の記録機能に対応
     使用方法　>呼び出したら、実行結果がbool型で返ってくるので ==
  trueの場合はゲーム開始の処理に移るようにする。 if (nfc_reduce_credit() ==
  false ) return;  //クレジットがない場合はloop文の先頭に戻る

   void nfc_write_point(byte point)
     機能説明: ・NFCカードがかざされるまで待機する処理
            　・引数で渡された追加ポイント情報をカード内の既存のポイントと合算して記録する機能
            　・読み込み/書き込み失敗時の再タッチ機能に対応
            　・reduce_credit()時にかざされたカードと別のカードの場合がかざされた場合のエラー表示機能
            　・ApplePay等のおサイフケータイをかざした時のデモプレイ機能(タッチレス終了機能)に対応
            　・筐体コードによる動作制限機能(制限解除)に対応
            　・操作番号の記録機能に対応
            　・最終プレイの筐体コードのバックアップ機能に対応
            　・オプションのシリアルメッセージ出力
     使用方法　>呼び出し時に()内にカードに追加したいポイントを変数や実数で入力し、関数を呼び出す
     nfc_write_point(100);
  //カード内のポイント管理領域に自動でポイントが加算される


   void nfc_clear_block12()
   　機能説明: ・NFCカードがかざされるまで待機する処理
  　　　　　    ・読み込み/書き込み失敗時の再タッチ機能に対応
   　　　　　  ・ブロック12の情報を初期値(クレジットのみ５)に設定し、書き込む。
   　　　　　　  尚、初期クレジット値は#defineより変更可能
   　書き込むデータ: byte nfc_defaultData[16] = {5,0,0,0 ,0,0,0,0 ,0,0,0,0
  ,0,0,0,0};

   void nfc_restore_block12()
   　機能説明: ・nfc_reduce_credit()でバックアップされたデータを復元
   　　　　　  ・読み込み/書き込み失敗時の再タッチ機能に対応
   　　　　　  ・ブロック12の[12]~[15]は識別用に255となる
*/

/*関数一覧
   bool nfc_read(nfc_readData, byte ReadBlock)
   bool nfc_write(byte WriteData[], byte WriteBlock)
   void nfc_setKeyB(byte block)
   bool nfc_autha(byte block)
   bool nfc_authb(byte block)
   bool ApplePay()
   void nfc_reset()
*/

/*用語集

   ・UID (Unique
  Identification)　Mifareのカードを識別するためのID個体識別のために利用される
   ・PCD (Proximity Coupling Device)　近接型ICのRead/Writeできる端末を指す
   ・PICC (Proximity IC
  Card・近接型カード)　非接触ICカードの中でも近接型のカードを指す ・Halt 止める
   ・Init 起動
   ・Crypto 暗号
   ・RF　Radio frequency - Wikipedia, the free encyclopedia
  　　　　　http://en.wikipedia.org/wiki/Radio_frequency
        搬送波が出ているだけの状態でも「RFが出てる」と表現することがある。高周波のことを表すだけのこともある。


*/

#include <MFRC522.h>
#include <SPI.h>

MFRC522::StatusCode status;        //ステータス変数を定義
MFRC522 mfrc522(SS_PIN, RST_PIN);  // RC522と接続

MFRC522::MIFARE_Key KeyA = {
  keyByte : {255, 255, 255, 255, 255, 255}
};  //デフォルト
MFRC522::MIFARE_Key KeyB = {
  keyByte : {255, 255, 255, 255, 255, 255}
};  //デフォルト
byte keyB_sector[16][6] = {
    {255, 255, 255, 255, 255, 255}, {255, 255, 255, 255, 255, 255},
    {255, 255, 255, 255, 255, 255}, {255, 255, 255, 255, 255, 255},
    {255, 255, 255, 255, 255, 255}, {255, 255, 255, 255, 255, 255},
    {255, 255, 255, 255, 255, 255}, {255, 255, 255, 255, 255, 255},
    {255, 255, 255, 255, 255, 255}, {255, 255, 255, 255, 255, 255},
    {255, 255, 255, 255, 255, 255}, {255, 255, 255, 255, 255, 255},
    {255, 255, 255, 255, 255, 255}, {255, 255, 255, 255, 255, 255},
    {255, 255, 255, 255, 255, 255}, {255, 255, 255, 255, 255, 255}};

MFRC522::PICC_Type piccType;
bool applepay = false;
byte randomID = 0;
#define creditReduce_code 0xFE  // 変更不可
#define pointWrite_code 0xFF    // 変更不可

void nfc_setup() {
  Serial.begin(SerialSpeed);
  Serial.println(F("begin Serial communication"));
  while (!Serial)
    ;
  Serial.println(F("begin SPI communication"));
  SPI.begin();
  delay(10);
  Serial.println(F("initialize nfc_PCD"));
  mfrc522.PCD_Init();  // PCD (Proximity Coupling Device)の初期化
}

bool nfc_reduce_credit() {
  nfc_wait_touch();
  if (!ApplePay()) {
    applepay = false;
    byte readData[18];
    byte writeData[16];
    while (nfc_read(readData, 12))
      ;

    if (readData[2] != 0) {
      Serial.println(F("------------------------------"));
      Serial.println();
      Serial.println(F("筐体コード制限を解除してください"));
      Serial.println();
      Serial.println(F("------------------------------"));
      return false;
    }

    for (byte i = 0; i < 12; i++) {
      writeData[i + 4] = readData[i];
    }

    randomID = random(0, 256);
    writeData[0] = readData[0] - 1;
    writeData[1] = readData[1];
    writeData[2] = boothNum;
    writeData[3] = creditReduce_code;
    writeData[6] = randomID;

    if (writeData[0] < 0) {
      //      Serial.println(F("------------------------------"));
      //      Serial.println(F("！クレジットが足りません！"));
      //      Serial.println();
      //      Serial.println(F("カードをはずしてください"));
      //      Serial.println();
      //      Serial.println(F("------------------------------"));
      return false;
    }

    while (nfc_write(writeData, 12))
      ;

    //    Serial.println(F("------------------------------"));
    //    Serial.println();
    //    Serial.println(F("ゲームを開始！"));
    //    Serial.println();
    //    Serial.println(F("------------------------------"));
    return true;
  }
  applepay = true;
  return true;
}

void nfc_write_point(byte addPoint) {
  if (applepay == false) {
    Serial.println(F("------------------------------"));
    Serial.println(
        F("獲得ポイントをNFCカードに記録します。カードをかざしてください"));

    nfc_wait_touch();

    byte nfc_readData[18];
    byte nfc_writeData[16];
    while (nfc_read(nfc_readData, 12))
      ;

    if (nfc_readData[6] != randomID) {
      Serial.println(F("------------------------------"));
      Serial.println(F("ゲーム開始時にかざしたカードをかざしてください"));
      Serial.println(F("------------------------------"));
      return;
    }

    for (byte i = 0; i < 12; i++) {
      nfc_writeData[i + 4] = nfc_readData[i];
    }

    nfc_writeData[0] = nfc_readData[0];
    nfc_writeData[1] = nfc_readData[1] + addPoint;
    nfc_writeData[2] = 0x00;
    nfc_writeData[3] = pointWrite_code;
    nfc_writeData[6] = boothNum;
    while (nfc_write(nfc_writeData, 12))
      ;

    Serial.println(F("------------------------------"));
    Serial.println(F("ゲームデータの記録が完了しました。"));
    Serial.println();
    Serial.print(F("残りクレジット　”　"));
    Serial.print(nfc_writeData[0]);
    Serial.println(F("　”"));
    Serial.print(F("残りポイント　”　"));
    Serial.print(nfc_writeData[1]);
    Serial.println(F("　”"));
    Serial.println();
    Serial.println(F("------------------------------"));
  }

  mfrc522.PICC_HaltA();       // 接続中の端末との通信を停止
  mfrc522.PCD_StopCrypto1();  // 端末とのPCDの暗号化を停止
  Serial.println(F("------------------------------"));
  Serial.println(F("終了"));
  Serial.println(F("------------------------------"));
  Serial.println();
  Serial.println();
  Serial.println();
}

void nfc_clear_block12() {
  Serial.println(F("------------------------------"));
  Serial.println(F("カード内部を初期化します"));

  nfc_wait_touch();

  byte nfc_defaultData[16];
  for (byte i = 0; i < 16; i++) {
    nfc_defaultData[i] = 0;
  }
  nfc_defaultData[0] = defaultCredit;
  while (nfc_write(nfc_defaultData, 12))
    ;
}

void nfc_restore_block12() {
  Serial.println(F("------------------------------"));
  Serial.println(F("バックアップデータを復元します"));

  nfc_wait_touch();

  byte nfc_backup_readData[18];
  byte nfc_backup_writeData[16];
  while (nfc_read(nfc_backup_readData, 12))
    ;

  for (byte i = 0; i < 16; i++) {
    nfc_backup_writeData[i] = nfc_backup_readData[i + 4];
  }
  for (byte i = 12; i < 16; i++) {
    nfc_backup_writeData[i] = 0xFF;
  }
  while (nfc_write(nfc_defaultData, 12))
    ;
}

void nfc_wait_touch() {
  Serial.println(F("------------------------------"));
  Serial.println();
  Serial.println(F("カードをかざしてください"));
  Serial.println();
  Serial.println(F("------------------------------"));
  while (!nfc_check())
    ;
}

bool nfc_check() {  // Mifareカードの確認（新しいカードが無ければ終了し、loop関数を繰り返す）
  mfrc522.PICC_HaltA();  // 接続中の端末との通信を停止
  mfrc522
      .PCD_StopCrypto1();  // 端末とのPCDの暗号化を停止
                           // //これがないと、前回のカード通信が切れずにNewCardPresentを通過してしまう
  delay(10);               //変更可能(タッチ判定の更新速度)
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial())
    return false;
  Serial.println(F("NFCtag touched"));
  Serial.println();
  return true;
}

bool nfc_read(byte nfc_readData[], byte block) {
  Serial.println(F("NFC begin read data"));
  Serial.print(F(" use sector: "));
  Serial.println(int(block / 4));
  Serial.print(F("     block: "));
  Serial.println(block);

  if (!nfc_auth_a(block)) return false;
  byte size = 18;  // 18固定
  status = (MFRC522::StatusCode)mfrc522.MIFARE_Read(block, nfc_readData, &size);
  if (status != MFRC522::STATUS_OK) {
    Serial.println(F("nfc_read() failed"));
    Serial.println();
    return false;
  }

  Serial.print(F("result: "));
  for (byte i = 0; i < 16; i++) {
    Serial.print(nfc_readData[i]);
    Serial.print(" ");
  }
  Serial.println();

  Serial.println(F("nfc_read() is in success"));
  return true;
}

bool nfc_write(byte WriteData[],
               byte block) {  //書き込む配列、書き込み先ブロック
  // loopに必ず入れること
  //書き込む情報を1バイト*16個保存するための変数（配列）

  // NFC_WRITE()前にdataBlock[16] = { , , , ...}の形式で書き込む内容を定義
  //呼び出しはNFC_WRITE(配列名, ブロック番号);
  //呼び出し時の引数の配列名に[]はいらないので注意

  Serial.println(F("NFC begin write data"));
  Serial.print(F(" use sector: "));
  Serial.println(int(block / 4));
  Serial.print(F("     block: "));
  Serial.println(block);

  if (!nfc_auth_b(block)) return false;
  byte size = 16;  // 16固定
  Serial.print(F(" write data "));
  for (byte i = 0; i < 16; i++) {
    Serial.print(WriteData[i]);
    Serial.print(" ");
  }
  Serial.println();
  status = (MFRC522::StatusCode)mfrc522.MIFARE_Write(block, WriteData, size);
  if (status != MFRC522::STATUS_OK) {
    Serial.println(F("nfc_write() failed"));
    Serial.println();
    return false;
  }
  Serial.println(F("nfc_write() is in success"));
  Serial.println();
  return true;
}

void nfc_setKeyB(byte sector) {
  //  Serial.print(F("NFC set keyB data for sector: "));
  //  Serial.println(sector);
  //  Serial.print(F(" use KeyB: "));
  for (byte i = 0; i < 6; i++) {
    KeyB.keyByte[i] = keyB_sector[sector][i];
    //    Serial.print(KeyB.keyByte[i]);
    //    Serial.print(" ");
  }
  //  Serial.println();
}

bool nfc_auth_a(byte block) {
  byte sector = int(block / 4);
  byte trailerBlock =
      sector * 4 + 3;  //そのセクターの認証キーが格納されているブロック番号
  //  Serial.println(F("start authenticate typeA"));
  //  Serial.print(F(" sector: "));
  //  Serial.println(sector);
  //  Serial.print(F(" piccType: "));
  //  piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  //  Serial.println(piccType);
  if (piccType != MFRC522::PICC_TYPE_MIFARE_UL) {
    //    Serial.print(F("use KeyA: "));
    //    for (byte i = 0; i < 6; i++) {
    //      Serial.print(KeyA.keyByte[i]);
    //      Serial.print(" ");
    //    }
    //    Serial.println();
    status = (MFRC522::StatusCode)mfrc522.PCD_Authenticate(
        MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &KeyA, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
      Serial.println(F("nfc_auth_a() failed"));
      Serial.println();
      return false;
    } else
      return true;  // MifareULではなく、認証に成功した場合
  }
  return true;  // MifareULの場合
}

bool nfc_auth_b(byte block) {
  byte sector = int(block / 4);
  byte trailerBlock =
      sector * 4 + 3;  //そのセクターの認証キーが格納されているブロック番号
  //  Serial.println(F("start authenticate typeB"));
  //  Serial.print(F(" sector: "));
  //  Serial.println(sector);
  //  Serial.print(F(" piccType: "));
  //  piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  //  Serial.println(piccType);
  if (piccType != MFRC522::PICC_TYPE_MIFARE_UL) {
    nfc_setKeyB(sector);
    status = (MFRC522::StatusCode)mfrc522.PCD_Authenticate(
        MFRC522::PICC_CMD_MF_AUTH_KEY_B, trailerBlock, &KeyB, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
      Serial.println(F("nfc_auth_b() failed"));
      Serial.println();
      return false;
    } else
      return true;  // MifareULではなく、認証に成功した場合
  }
  return true;  // MifareULの場合
}

bool ApplePay() {
  Serial.println(F("check card UID & Apple Pay..."));
  if (String(mfrc522.uid.uidByte[0], HEX) == "8") {
    Serial.println(F("Apple Pay"));
    return true;
  } else {
    Serial.println(F("isn't Apple Pay"));
    return false;
  }
}
