/* PROJETO SGA V1.0 - Dar acesso as salas da instutuição adcionando o código dala ao cartão do funcionário.

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
const int buzzer = A4;

constexpr uint8_t redLed = 7;   // DEFINIR PINOS DOS LEDS
constexpr uint8_t greenLed = 6;
constexpr uint8_t blueLed = 5;

constexpr uint8_t relay = 3;     // DEFINIR PINO DO RELAY
constexpr uint8_t wipeB = 4;     // DEFINIR PINO DO BOTÃO RESSET

       // inicializar correspondência de cartão com falso
  // inicializa o modo de programação para falso

constexpr uint8_t RST_PIN = 9;     // Configurável, veja o layout típico de pinos acima
constexpr uint8_t SS_PIN = 10;     // Configurável, veja o layout típico de pinos acima
MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;
MFRC522::StatusCode status;


///////////////////////////////////////// Setup ///////////////////////////////////
void setup() {
  //Arduino Pin Configuration
  pinMode(buzzer, OUTPUT);
  pinMode(redLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
  pinMode(blueLed, OUTPUT);
  pinMode(wipeB, INPUT_PULLUP);   //Habilite o resistor de pull up
  pinMode(relay, OUTPUT);//Tenha cuidado como o circuito do relé se comporta enquanto reinicia ou liga e desliga o seu Arduino
  digitalWrite(relay, HIGH);   // Certifique-se de que a porta esteja trancada
  digitalWrite(redLed, LED_OFF);  // Certifique-se de que o led está desligado
  digitalWrite(greenLed, LED_OFF);  // Certifique-se de que o led está desligado
  digitalWrite(blueLed, LED_OFF); // Certifique-se de que o led está desligado
  //Configuração do Protocolo
  Serial.begin(9600);  // Inicialize as comunicações seriais com o PC
  SPI.begin();           // MFRC522 Hardware usa protocolo SPI
  mfrc522.PCD_Init();    // Inicializar Hardware MFRC522
  //Se você definir Antenna Gain to Max, aumentará a distância de leitura
  // mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_max);
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;
  Serial.println(F("Leitura dos Dados :"));
}
///////////////////////////////////////// Main Loop ///////////////////////////////////
void loop () {
  while (WaitingCard() == 1) {
    granted(500);
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
  digitalWrite(blueLed, LED_OFF);   // Desligue o LED azul
  digitalWrite(redLed, LED_OFF);  // Desligue o LED VERMELHO
  digitalWrite(greenLed, LED_ON);   // Desligue o LED VERDE
  tone(buzzer, 4500);
  delay(200);
  noTone(buzzer);
  tone(buzzer, 4500);
  delay(100);
  noTone(buzzer);
  digitalWrite(relay, LOW);     // NIVEL BAIXO
  delay(500);          // NIVEL BAIXO POR ALGUNS SEGUNDOS
  digitalWrite(relay, HIGH);    // NIVEL ALTO
  delay(500);            // Mantenha o LED verde aceso por um SEGUNDO
}
