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
    Serial.println(F("Ligado!"));
    delay(5000);
  }

  Serial.println(F("Close!"));

  asm volatile("jmp 0");
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
      return false;
    }

    status = mfrc522.MIFARE_Read(block, buffer, &len);
    if (status != MFRC522::STATUS_OK) {
      return false;
    }
    Serial.println(F("Este é o bloco lido!"));
    for (uint8_t i = 0; i < 4; i++)
    {
      Serial.print(buffer[i] < 0x10 ? " 0" : " ");
      Serial.print(buffer[i], HEX);
      content.concat(String(buffer[i] < 0x10 ? " 0" : " "));
      content.concat(String(buffer[i], HEX));
    }
    Serial.println("");
    content.toUpperCase();

    finded = isMatch(content);

    block = block + 1;

    if (block >= 63) {
      finded = true;
    }
  }
  block = 1;;

  return finded;
}

boolean isMatch(String readTag) {
  if (readTag.substring(1) == "86 05 56 1F" /*70*/ or
      readTag.substring(1) == "76 D3 79 1F" /*HS*/ or
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

///////////////////////////////////////// Access Denied  ///////////////////////////////////
void denied() {
  digitalWrite(greenLed, LED_OFF);  // Certifique-se de que o LED verde esteja apagado
  digitalWrite(blueLed, LED_OFF);   // MCertifique-se de que o LED AZUL esteja apagado
  digitalWrite(redLed, LED_ON);   // Ligue o LED vermelho
  delay(5);
}

///////////////////////////////////////// Cycle Leds (Program Mode) ///////////////////////////////////
void cycleLeds() {
  tone(buzzer, 1500);
  delay(100);
  noTone(buzzer);
  tone(buzzer, 2500);
  delay(100);
  noTone(buzzer);
  tone(buzzer, 3500);
  delay(100);
  noTone(buzzer);
  tone(buzzer, 4500);
  delay(100);
  noTone(buzzer);
  delay(1000);
}

//////////////////////////////////////// Normal Mode Led  ///////////////////////////////////
void normalModeOn () {
  digitalWrite(blueLed, LED_ON);
  digitalWrite(redLed, LED_OFF);
  digitalWrite(greenLed, LED_OFF);
  digitalWrite(relay, LOW);
}

///////////////////////////////////////// Write Success to EEPROM   ///////////////////////////////////
// Pisca o LED verde 3 vezes para indicar uma gravação bem-sucedida na EEPROM
void successWrite() {
  digitalWrite(blueLed, LED_OFF);
  digitalWrite(redLed, LED_OFF);
  digitalWrite(greenLed, LED_OFF);
  delay(200);
  digitalWrite(greenLed, LED_ON);
  delay(200);
  digitalWrite(greenLed, LED_OFF);
  delay(200);
  digitalWrite(greenLed, LED_ON);
  delay(200);
  digitalWrite(greenLed, LED_OFF);
  delay(200);
  digitalWrite(greenLed, LED_ON);
  delay(200);
}

///////////////////////////////////////// Write Failed to EEPROM   ///////////////////////////////////
//Pisca o LED vermelho 3 vezes para indicar uma falha na gravação na EEPROM
void failedWrite() {
  digitalWrite(blueLed, LED_OFF);
  digitalWrite(redLed, LED_OFF);
  digitalWrite(greenLed, LED_OFF);
  delay(200);
  digitalWrite(redLed, LED_ON);
  delay(200);
  digitalWrite(redLed, LED_OFF);
  delay(200);
  digitalWrite(redLed, LED_ON);
  delay(200);
  digitalWrite(redLed, LED_OFF);
  delay(200);
  digitalWrite(redLed, LED_ON);
  delay(200);
}

///////////////////////////////////////// Success Remove UID From EEPROM  ///////////////////////////////////
// Pisca o LED azul 3 vezes para indicar um sucesso excluir para EEPROM
void successDelete() {
  digitalWrite(blueLed, LED_OFF);
  digitalWrite(redLed, LED_OFF);
  digitalWrite(greenLed, LED_OFF);
  delay(200);
  digitalWrite(blueLed, LED_ON);
  delay(200);
  digitalWrite(blueLed, LED_OFF);
  delay(200);
  digitalWrite(blueLed, LED_ON);
  delay(200);
  digitalWrite(blueLed, LED_OFF);
  delay(200);
  digitalWrite(blueLed, LED_ON);
  delay(200);
}

////////////////////// Check readCard IF is masterCard   ///////////////////////////////////

bool monitorWipeButton(uint32_t interval) {
  uint32_t now = (uint32_t)millis();
  while ((uint32_t)millis() - now < interval)  {
    // check on every half a second
    if (((uint32_t)millis() % 500) == 0) {
      if (digitalRead(wipeB) != LOW)
        return false;
    }
  }
  return true;
}
