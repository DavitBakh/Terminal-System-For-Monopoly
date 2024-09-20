#include <SPI.h>
#include <MFRC522.h>
#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define RST_PIN 9  // Пин rfid модуля RST
#define SS_PIN 10  // Пин rfid модуля SS

// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);

MFRC522 rfid(SS_PIN, RST_PIN);  // Объект rfid модуля
MFRC522::MIFARE_Key key;        // Объект ключа
MFRC522::StatusCode status;     // Объект статуса

const byte ROWS = 4;
const byte COLS = 4;

char hexaKeys[ROWS][COLS] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};

byte rowPins[ROWS] = { 9, 8, 7, 6 };
byte colPins[COLS] = { 5, 4, 3, 2 };

Keypad keypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

int zPin = A2;

void setup() {
  Serial.begin(9600);  // Инициализация Serial
  Serial.setTimeout(5);

  lcd.begin();
  lcd.backlight();

  pinMode(zPin, OUTPUT);

  SPI.begin();                               // Инициализация SPI
  rfid.PCD_Init();                           // Инициализация модуля
  rfid.PCD_SetAntennaGain(rfid.RxGain_max);  // Установка усиления антенны
  rfid.PCD_AntennaOff();                     // Перезагружаем антенну
  rfid.PCD_AntennaOn();                      // Включаем антенну

  for (byte i = 0; i < 6; i++) {  // Наполняем ключ
    key.keyByte[i] = 0xFF;        // Ключ по умолчанию 0xFFFFFFFFFFFF
  }
}

char* arr = new char[10];
char command;
short index = 0;

bool flag = false;

void loop() {
  // Занимаемся чем угодно

  static uint32_t rebootTimer = millis();  // Важный костыль против зависания модуля!
  if (millis() - rebootTimer >= 1000) {    // Таймер с периодом 1000 мс
    rebootTimer = millis();                // Обновляем таймер
    digitalWrite(RST_PIN, HIGH);           // Сбрасываем модуль
    delayMicroseconds(2);                  // Ждем 2 мкс
    digitalWrite(RST_PIN, LOW);            // Отпускаем сброс
    rfid.PCD_Init();                       // Инициализируем заного
  }

  // send data only when you receive data:
  char key = keypad.getKey();
  if (key) {

    Serial.println(key);
    if (flag) {
      lcd.clear();
      lcd.setCursor(0, 0);
      flag = false;
    }


    digitalWrite(zPin, 1);
    delay(100);
    digitalWrite(zPin, 0);

    if (key == '*') {
      index--;
      lcd.setCursor(index, 0);
      lcd.print("0");
      lcd.setCursor(index, 0);

      return;
    }

    lcd.print(key);

    if (key != '#') {
      arr[index++] = key;
      return;
    }


    // set flag to clear the display when start new input
    flag = true;

    arr[index] = '\0';
    index = 0;

    bool state = false;
    switch (arr[0]) {
      case 'A':
        Print(Add(atoi(arr + 1), state), state);
        break;

      case 'B':
        Print(Transfer(atoi(arr + 1), state), state);
        break;

      case 'C':
        Print(Sub(atoi(arr + 1), state), state);
        break;

      case 'D':
        Print(GetBalance(state), state);
        break;
    }
  }
}

bool zummer = false;

uint16_t GetBalance(bool& state) {
  while (!rfid.PICC_IsNewCardPresent()) {

    static uint32_t rebootTimer = millis();
    if (millis() - rebootTimer >= 250) {  // Таймер с периодом 1500 мс
      rebootTimer = millis();             // Обновляем таймер
      digitalWrite(zPin, zummer);
      zummer = !zummer;
    }

    Serial.println("Not present  ");

  }  // Если новая метка не поднесена - вернуться в начало loop
  if (!rfid.PICC_ReadCardSerial()) {
    Serial.println("Try read");
    if (!rfid.PICC_ReadCardSerial()) {
      Serial.println("Try read");
      return;
    }
  }

  /* Аутентификация сектора, указываем блок безопасности #7 и ключ B */
  status = rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, 7, &key, &(rfid.uid));
  if (status != MFRC522::STATUS_OK) {  // Если не окэй
    Serial.println("Auth error");      // Выводим ошибку
    return;
  }

  /* Чтение блока, указываем блок данных #6 */
  uint8_t dataBlock[18];             // Буфер
  uint8_t size = sizeof(dataBlock);  // Размер буфера

  status = rfid.MIFARE_Read(6, dataBlock, &size);  // Читаем блок 6
  if (status != MFRC522::STATUS_OK) {              // Если не окэй
    Serial.println("Read error");                  // Выводим ошибку
    return;
  }


  rfid.PICC_HaltA();  // Завершаем работу с меткой
  rfid.PCD_StopCrypto1();

  Zummer();

  state = true;
  return dataBlock[0] * 256 + dataBlock[1];
}

uint16_t Add(uint16_t moneyToAdd, bool& state) {

  while (!rfid.PICC_IsNewCardPresent()) {

    static uint32_t rebootTimer = millis();
    if (millis() - rebootTimer >= 250) {  // Таймер с периодом 1500 мс
      rebootTimer = millis();             // Обновляем таймер
      digitalWrite(zPin, zummer);
      zummer = !zummer;
    }

    Serial.println("Not present  ");
  }  // Если новая метка не поднесена - вернуться в начало loop
  if (!rfid.PICC_ReadCardSerial()) {
    Serial.println("Try read");
    if (!rfid.PICC_ReadCardSerial()) {
      Serial.println("Try read");
      return;
    }
  }

  /* Аутентификация сектора, указываем блок безопасности #7 и ключ B */
  status = rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, 7, &key, &(rfid.uid));
  if (status != MFRC522::STATUS_OK) {  // Если не окэй
    Serial.println("Auth error");      // Выводим ошибку
    return;
  }

  /* Чтение блока, указываем блок данных #6 */
  uint8_t dataBlock[18];             // Буфер
  uint8_t size = sizeof(dataBlock);  // Размер буфера

  status = rfid.MIFARE_Read(6, dataBlock, &size);  // Читаем блок 6
  if (status != MFRC522::STATUS_OK) {              // Если не окэй
    Serial.println("Read error");                  // Выводим ошибку
    return;
  }

  uint16_t money = dataBlock[0] * 256 + dataBlock[1];

  money += moneyToAdd;
  uint16_t copy = money;

  uint8_t t = money / 256;
  money -= t * 256;

  uint8_t dataToWrite[16] = { t, money, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

  status = rfid.MIFARE_Write(6, dataToWrite, 16);  // Пишем массив в блок 6
  if (status != MFRC522::STATUS_OK) {              // Если не окэй
    Serial.println("Write error");                 // Выводим ошибку
    return;
  }

  rfid.PICC_HaltA();  // Завершаем работу с меткой
  rfid.PCD_StopCrypto1();

  Zummer();

  state = true;
  return copy;
}

uint16_t Sub(uint16_t moneyToSub, bool& state) {
  while (!rfid.PICC_IsNewCardPresent()) {

    static uint32_t rebootTimer = millis();
    if (millis() - rebootTimer >= 250) {  // Таймер с периодом 1500 мс
      rebootTimer = millis();             // Обновляем таймер
      digitalWrite(zPin, zummer);
      zummer = !zummer;
    }

    Serial.println("Not present  ");
  }  // Если новая метка не поднесена - вернуться в начало loop
  if (!rfid.PICC_ReadCardSerial()) {
    Serial.println("Try read");
    if (!rfid.PICC_ReadCardSerial()) {
      Serial.println("Try read");
      return;
    }
  }

  /* Аутентификация сектора, указываем блок безопасности #7 и ключ B */
  status = rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, 7, &key, &(rfid.uid));
  if (status != MFRC522::STATUS_OK) {  // Если не окэй
    Serial.println("Auth error");      // Выводим ошибку
    return;
  }

  /* Чтение блока, указываем блок данных #6 */
  uint8_t dataBlock[18];             // Буфер
  uint8_t size = sizeof(dataBlock);  // Размер буфера

  status = rfid.MIFARE_Read(6, dataBlock, &size);  // Читаем блок 6
  if (status != MFRC522::STATUS_OK) {              // Если не окэй
    Serial.println("Read error");                  // Выводим ошибку
    return;
  }

  uint16_t money = dataBlock[0] * 256 + dataBlock[1];

  if (moneyToSub <= money) {
    money -= moneyToSub;
  } else {
    money = 0;
  }

  uint16_t copy = money;
  uint8_t t = money / 256;
  money -= t * 256;

  uint8_t dataToWrite[16] = { t, money, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

  status = rfid.MIFARE_Write(6, dataToWrite, 16);  // Пишем массив в блок 6
  if (status != MFRC522::STATUS_OK) {              // Если не окэй
    Serial.println("Write error");                 // Выводим ошибку
    return;
  }


  rfid.PICC_HaltA();  // Завершаем работу с меткой
  rfid.PCD_StopCrypto1();

  Zummer();

  state = true;
  return copy;
}

uint16_t Transfer(uint16_t moneyToTransfer, bool& state) {

  Print(Sub(moneyToTransfer, state), state);

  delay(1000);
  state = false;
  return Add(moneyToTransfer, state);
}

void Zummer() {
  delay(250);
  digitalWrite(zPin, 1);
  delay(500);
  digitalWrite(zPin, 0);
}

void Print(const uint16_t& result, const bool& state) {
  if (state) {
    Serial.print("Your Balance: ");
    Serial.println(result);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Your Balance");
    lcd.setCursor(0, 1);
    lcd.print(result, DEC);
    return;
  }

  Serial.print("Error Try again");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Error Try Again!");
  digitalWrite(zPin, 0);
}