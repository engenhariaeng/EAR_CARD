/* PROJETO ESCOLA V2.0

  DEFINIÇÃO DAS LIGAÇÕES
   -----------------------------------------------------------------------------------------
               MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
               Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
   Signal      Pin          Pin           Pin       Pin        Pin              Pin
   -----------------------------------------------------------------------------------------
   RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
   SPI SS      SDA(SS)      10            53        D10        10               10
   SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
   SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
   SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
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

  Serial.println(F("SGA - Ar Condicionado 2.1.1"));
  Serial.println(F("Esperando um cartão para ser lido..."));

  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;
}

void loop () {
  boolean moreTime = false;
  do {
    successRead = WaitingCard();
  } while (successRead != 1);
  Serial.println(F("Verificando.."));

  while (verifyAccess() == true) {
    moreTime = true;
    granted(5000);
  }
  if (moreTime == true) {
    asm volatile("jmp 0");
  }
  Serial.println("Desligado!");
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

  while (finded == false) {
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
  if (readTag.substring(1) == "86 98 5F 1F" /*72*/ or
      readTag.substring(1) == "76 D3 79 1F" /*HS*/ or
      readTag.substring(1) == "86 99 D2 1F" /*Ar GERAL*/ or
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
