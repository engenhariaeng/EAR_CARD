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

boolean match = false;          // inicializar correspondência de cartão com falso
boolean programMode = false;  // inicializa o modo de programação para falso
boolean replaceMaster = false;

uint8_t successRead;    // Inteiro variável para manter se tivermos uma leitura bem-sucedida do Reader

byte storedCard[4];   // Armazena uma ID lida da EEPROM
byte readCard[4];   // Armazena a identificação digitalizada do módulo RFID
byte masterCard[4];   // Armazena o ID da placa principal lido na EEPROM

String content = "";
int block = 1;

// Crie a instância MFRC522.
constexpr uint8_t RST_PIN = 9;     // Configurável, veja o layout típico de pinos acima
constexpr uint8_t SS_PIN = 10;     // Configurável, veja o layout típico de pinos acima
MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;
///////////////////////////////////////// Setup ///////////////////////////////////
void setup() {
  //Arduino Pin Configuration
  pinMode(redLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
  pinMode(blueLed, OUTPUT);
  pinMode(wipeB, INPUT_PULLUP);   //Habilite o resistor de pull up
  pinMode(relay, OUTPUT);//Tenha cuidado como o circuito do relé se comporta enquanto reinicia ou liga e desliga o seu Arduino
  digitalWrite(relay, LOW);   // Certifique-se de que a porta esteja trancada
  digitalWrite(redLed, LED_OFF);  // Certifique-se de que o led está desligado
  digitalWrite(greenLed, LED_OFF);  // Certifique-se de que o led está desligado
  digitalWrite(blueLed, LED_OFF); // Certifique-se de que o led está desligado
  //Configuração do Protocolo
  Serial.begin(9600);  // Inicialize as comunicações seriais com o PC
  SPI.begin();           // MFRC522 Hardware usa protocolo SPI
  mfrc522.PCD_Init();    // Inicializar Hardware MFRC522
  //Se você definir Antenna Gain to Max, aumentará a distância de leitura
  // mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_max);

  Serial.println(F("Access Control Example v0.1"));   // Para fins de depuração
  //ShowReaderDetails();  // Mostrar detalhes de PCD - MFRC522 Card Reader detalhes
  //Código de limpeza - Se o botão (wipeB) for pressionado durante a execução da instalação (ligado), a EEPROM
  Serial.println(F("Waiting for a card..."));
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;
}


///////////////////////////////////////// Main Loop ///////////////////////////////////
void loop () {
  if ( findID() ) { // Se não, veja se o cartão está na EEPROM
    Serial.println(F("Welcome, You shall pass"));
    granted(5000);         // MANTENHA O NIVEL AUTO POR 5000 ms
  }
  else {      // Caso contrário, mostre que o ID não era válido
    Serial.println(F("Hello"));
    Serial.println(F("You shall not pass"));
    denied();
  }
}
/////////////////////////////////////////  Access Granted    ///////////////////////////////////
void granted ( uint16_t setDelay) {
  /* digitalWrite(blueLed, LED_OFF);   // Desligue o LED azul
    digitalWrite(redLed, LED_OFF);  // Desligue o LED VERMELHO
    digitalWrite(greenLed, LED_ON);   // Desligue o LED VERDE */
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
  /* digitalWrite(redLed, LED_OFF);
    digitalWrite(greenLed, LED_ON);
    digitalWrite(blueLed, LED_OFF);
    delay(200);
    digitalWrite(redLed, LED_OFF);
    digitalWrite(greenLed, LED_OFF);
    digitalWrite(blueLed, LED_ON);
    delay(200);
    digitalWrite(redLed, LED_ON);
    digitalWrite(greenLed, LED_OFF);
    digitalWrite(blueLed, LED_OFF);
    delay(200); */
}

//////////////////////////////////////// Normal Mode Led  ///////////////////////////////////
void normalModeOn () {
  digitalWrite(blueLed, LED_ON);
  digitalWrite(redLed, LED_OFF);
  digitalWrite(greenLed, LED_OFF);
  digitalWrite(relay, LOW);
}

//////
///////////////////////////////////////// Find ID From EEPROM   ///////////////////////////////////
boolean findID() {
  int block = 1;
  boolean finded = false;
  boolean okay = false;
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    okay = false;
  }

  if ( ! mfrc522.PICC_ReadCardSerial()) {
    okay = false;
  }

  while (finded == false) {
    String content = "";
    byte buffer[18];
    byte len = 18;

    byte status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
      Serial.println(F("Authentication failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
      finded = true;
      okay = false;
    }

    status = mfrc522.MIFARE_Read(block, buffer, &len);
    if (status != MFRC522::STATUS_OK) {
      Serial.println(F("Reading failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
      finded = true;
      okay = false;
    }
    Serial.print(block);
    Serial.print(" block is: ");
    for (uint8_t i = 0; i < 4; i++)
    {
      Serial.print(buffer[i] < 0x10 ? " 0" : " ");
      Serial.print(buffer[i], HEX);
      content.concat(String(buffer[i] < 0x10 ? " 0" : " "));
      content.concat(String(buffer[i], HEX));
    }
    Serial.println("");
    content.toUpperCase();


    if (content.substring(1) == "D0 15 70 25" /*SALA 62*/
       ) //change here the UID of the card/cards that you want to give access 76 D8 AD 1F
    {
      Serial.println("Authorized access");
      Serial.println();
      finded = true;
      okay = true;
      block = 1;
      granted(500);
    } else {
      if (block + 1 == 63) {
        Serial.println("Not authorized access");
        Serial.println();
        delay(100);
        finded = true;
        okay = false;
        block = 1;
        denied();
      } else {
        block = block + 1;
      }
    }
  }
  return okay;
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
