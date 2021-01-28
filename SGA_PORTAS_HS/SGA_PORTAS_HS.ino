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

boolean match = false;          // inicializar correspondência de cartão com falso
boolean programMode = false;  // inicializa o modo de programação para falso
boolean replaceMaster = false;

uint8_t successRead;    // Inteiro variável para manter se tivermos uma leitura bem-sucedida do Reader

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
  do {
    successRead = WaitingCard();
  } while (successRead != 1);
  Serial.println(F("Verificando.."));
  
  while (verifyAccess() == true) {
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

    if (block >= 63) {
      Serial.print(F("Eu li todos "));
      break;
    }
  }
  block = 1;

  return finded;
}

boolean isMatch(String readTag) {
  if (readTag.substring(1) == "A0 A5 43 25" /*71*/) {
    return true;
  }
  return false;
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

///////////////////////////////////////// Access Denied  ///////////////////////////////////
void denied() {
  digitalWrite(greenLed, LED_OFF);  // Certifique-se de que o LED verde esteja apagado
  digitalWrite(blueLed, LED_OFF);   // MCertifique-se de que o LED AZUL esteja apagado
  digitalWrite(redLed, LED_ON);   // Ligue o LED vermelho
  delay(1000);
}

void ShowReaderDetails() {
  // Obtenha a versão do software MFRC522
  byte v = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
  Serial.print(F("MFRC522 Software Version: 0x"));
  Serial.print(v, HEX);
  if (v == 0x91)
    Serial.print(F(" = v1.0"));
  else if (v == 0x92)
    Serial.print(F(" = v2.0"));
  else
    Serial.print(F(" (unknown),probably a chinese clone?"));
  Serial.println("");
  // Quando 0x00 ou 0xFF é retornado, a comunicação provavelmente falhou
  if ((v == 0x00) || (v == 0xFF)) {
    Serial.println(F("WARNING: Communication failure, is the MFRC522 properly connected?"));
    Serial.println(F("SYSTEM HALTED: Check connections."));
    // Visualize o sistema é interrompido
    digitalWrite(greenLed, LED_OFF);  //  Certifique-se de que o LED verde esteja apagado
    digitalWrite(blueLed, LED_OFF);   //  Certifique-se de que o LED AZUL esteja apagado
    digitalWrite(redLed, LED_ON);   // LIGAR LED VERMELHO
    while (true); // não vá mais longe
  }
}

///////////////////////////////////////// Cycle Leds (Program Mode) ///////////////////////////////////
void cycleLeds() {
  digitalWrite(redLed, LED_OFF);
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
  delay(200);
  tone(buzzer, 4500);
  delay(100);
  noTone(buzzer);
  tone(buzzer, 4500);
  delay(100);
  noTone(buzzer);
}

//////////////////////////////////////// Normal Mode Led  ///////////////////////////////////
void normalModeOn () {
  digitalWrite(blueLed, LED_ON);
  digitalWrite(redLed, LED_OFF);
  digitalWrite(greenLed, LED_OFF);
  digitalWrite(relay, HIGH);
}


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
