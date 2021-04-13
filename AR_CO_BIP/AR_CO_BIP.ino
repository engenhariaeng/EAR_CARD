/* PROJETO ESCOLA V2.0
*/

#include <EEPROM.h>     // Vamos ler e escrever os UIDs do PICC de / para a EEPROM
#include <SPI.h>        // Módulo RC522 usa o protocolo SPI
#include <MFRC522.h>  // Biblioteca para dispositivos Mifare RC522
#define COMMON_ANODE
#ifdef COMMON_ANODE
#define LED_ON LOW
#define LED_OFF HIGH
#else
#define LED_ON HIGH
#define LED_OFF LOW
#endif
const int buzzer = A3;
constexpr uint8_t redLed = 7;   // DEFINIR PINOS DOS LEDS
constexpr uint8_t greenLed = 6;
constexpr uint8_t blueLed = 5;

constexpr uint8_t relay = 3;     // DEFINIR PINO DO RELAY
constexpr uint8_t wipeB = 4;     // DEFINIR PINO DO BOTÃO RESSET

uint8_t successRead;

constexpr uint8_t RST_PIN = 9;
constexpr uint8_t SS_PIN = 10;
MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;
///////////////////////////////////////// Setup ///////////////////////////////////
void setup() {
  pinMode(redLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
  pinMode(blueLed, OUTPUT);
  pinMode(wipeB, INPUT_PULLUP);
  pinMode(relay, OUTPUT);
  digitalWrite(relay, LOW);
  digitalWrite(redLed, LED_OFF);
  digitalWrite(greenLed, LED_OFF);
  digitalWrite(blueLed, LED_OFF);

  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();

  Serial.println(F("SGA - Ar Condicionado 2.1"));
  Serial.println(F("Esperando um cartão para ser lido..."));

  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;
}

void loop () {
  do {
    successRead = WaitingCard();
  } while (successRead != 1);
  Serial.println(F("Verificando.."));

  while (verifyAccess() == true) {
    granted(5000);
  }
  //denied();

}

uint8_t WaitingCard() {
  if ( ! mfrc522.PICC_IsNewCardPresent()) { //Se um novo PICC colocado no leitor RFID continuar
    return 0;
  }
  if ( ! mfrc522.PICC_ReadCardSerial()) {   //Desde que um PICC colocado obtenha o Serial e continue
    return 0;
  }

  return 1;
}


boolean verifyAccess() {
  boolean finded = false;
  byte buffer[18];
  byte len = 18;
  int block = 1;

  while (finded == false) {//ANALISAR ISSO AQUI!
    String content = "";
    MFRC522::StatusCode status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
      Serial.print(F("Erro de autenticação "));
      break;
    }

    mfrc522.MIFARE_Read(block, buffer, &len);

    Serial.println(F("Este é o bloco lido!"));
    Serial.println(block);
    for (uint8_t i = 0; i < 4; i++)
    {
      Serial.print(buffer[i] < 0x10 ? " 0" : " ");
      Serial.print(buffer[i], HEX);
      content.concat(String(buffer[i] < 0x10 ? " 0" : " "));
      content.concat(String(buffer[i], HEX));
    }
    Serial.println("");
    content.toUpperCase();

    finded = isMatch(content);//here, we get true for find and false while not find

    block = block + 1;

  }
  block = 1;

  return finded;
}

boolean isMatch(String readTag) {
  if (readTag.substring(1) == "76 D5 16 1F" /*21*/ or
      readTag.substring(1) == "D0 36 E1 25" /*ES*/ or
      readTag.substring(1) == "76 D7 2C 1F" /*GERAL*/
     ) {
    return true;
  }
  return false;
}




/////////////////////////////////////////  Access Granted    ///////////////////////////////////
void granted ( uint16_t setDelay) {
  digitalWrite(relay, HIGH);     // LIGAR O AR!
  delay(setDelay);          // NIVEL ALTO POR ALGUNS SEGUNDOS
  digitalWrite(relay, LOW);    // NIVEL BAIXO
  delay(5);            // Mantenha o LED verde aceso por um MILESIMO

}
