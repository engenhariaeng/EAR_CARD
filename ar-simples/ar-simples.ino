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
  while (WaitingCard() == 1) {
    granted(5000);
  }
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



/////////////////////////////////////////  Access Granted    ///////////////////////////////////
void granted ( uint16_t setDelay) {
  digitalWrite(relay, HIGH);     // LIGAR O AR!
  delay(setDelay);          // NIVEL ALTO POR ALGUNS SEGUNDOS
  digitalWrite(relay, LOW);    // NIVEL BAIXO
  delay(5);            // Mantenha o LED verde aceso por um MILESIMO

}
