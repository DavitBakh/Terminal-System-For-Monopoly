#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN 9  // Пин rfid модуля RST
#define SS_PIN 10  // Пин rfid модуля SS

MFRC522 rfid(SS_PIN, RST_PIN);  // Объект rfid модуля
MFRC522::MIFARE_Key key;        // Объект ключа
MFRC522::StatusCode status;     // Объект статуса

void setup() {
  Serial.begin(9600);  // Инициализация Serial
  Serial.setTimeout(5);

  SPI.begin();                               // Инициализация SPI
  rfid.PCD_Init();                           // Инициализация модуля
  rfid.PCD_SetAntennaGain(rfid.RxGain_max);  // Установка усиления антенны
  rfid.PCD_AntennaOff();                     // Перезагружаем антенну
  rfid.PCD_AntennaOn();                      // Включаем антенну

  for (byte i = 0; i < 6; i++) {  // Наполняем ключ
    key.keyByte[i] = 0xFF;        // Ключ по умолчанию 0xFFFFFFFFFFFF
  }
}

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
  if (Serial.available() > 1) {
    // read the incoming byte:
    char key = Serial.read();
    int value = Serial.parseInt();

    uint16_t result;
    switch (key) {
      case 'a':
        result = Add(value);
        Serial.print("Your Balance: ");
        Serial.println(result);
        break;

      case 's':
        result = Sub(value);
        Serial.print("Your Balance: ");
        Serial.println(result);
        break;

      case 'b':
        result = GetBalance();
        Serial.print("Your Balance: ");
        Serial.println(result);
        break;
    }
  }
}

uint16_t GetBalance() {
  while (!rfid.PICC_IsNewCardPresent()) { Serial.println("Not present  "); }  // Если новая метка не поднесена - вернуться в начало loop
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

  return dataBlock[0] * 256 + dataBlock[1];
}

uint16_t Add(uint16_t moneyToAdd) {

  while (!rfid.PICC_IsNewCardPresent()) { Serial.println("Not present  "); }  // Если новая метка не поднесена - вернуться в начало loop
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

  return copy;
}

uint16_t Sub(uint16_t moneyToSub) {
  while (!rfid.PICC_IsNewCardPresent()) { Serial.println("Not present  "); }  // Если новая метка не поднесена - вернуться в начало loop
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

  return copy;
}