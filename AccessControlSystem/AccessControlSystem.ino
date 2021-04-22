/* ACCESS CONTROL EAR
   GUSTAVO SILVA
   09/04/2021
   V1.0 - current
*/
//=======================Definitions===================================
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

constexpr uint8_t redLed = 7; constexpr uint8_t greenLed = 6; constexpr uint8_t blueLed = 5; // DEFINIR PINOS DOS LEDS

constexpr uint8_t relay = 3;     // DEFINIR PINO DO RELAY
constexpr uint8_t wipeB = 4;     // DEFINIR PINO DO BOTÃO RESSET

constexpr uint8_t RST_PIN = 9;     // Configurável, veja o layout típico de pinos acima
constexpr uint8_t SS_PIN = 10;     // Configurável, veja o layout típico de pinos acima
MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;//create a MIFARE_Key struct named 'key', which will hold the card information

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
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;//keyByte is defined in the "MIFARE_Key" 'struct' definition in the .h file of the library
  }
  Serial.println(F("Insira a tag-ação que deseja executar"));
  delay(1000);
}

//=======================Global Variables===============================
int successRead;

String currentAction = "";

String currentStringTag = "";
byte currentByteTag[4];

String actionTagDelete = "C3 98 53 83";
String actionTagAdd = "9C B6 B4 C3";

byte cleanTag[4] = {0, 0, 0, 0};

byte roomTag[4];
byte userTag[4];

boolean waitUserTag = false;
boolean gettingTags = true;

void loop() {
  do {
    successRead = WaitingCard();
  } while (successRead != 1);
  if (isActionMode()) {
    if (waitUserTag) {
      if (currentAction == "ADD") {
        Serial.println("Inserindo tag...");
        delay(2000);
      } else {
        Serial.println("Removendo tag...");
        delay(2000);
      }
      resetProcess();
    } else if (isRoomTag()) {
      Serial.println("");
      Serial.print("Agora, insira uma tag de ação para iniciar a ação ");
      Serial.println(currentAction);
      Serial.println("Caso queira trocar a sala, basta inserir outra.");
      delay(1000);
    } else {
      if (isCancel()) {
        Serial.println("Ação cancelada.");
        resetProcess();
      } else {
        Serial.println("Agora, insira o cartão individual para realizar a ação.");
        delay(1000);
        waitUserTag = true;
      }
    }
  } else {
    if (verifyAction()) {
      delay(800);
      Serial.println("");
      Serial.print("Você escolheu a função ");
      Serial.print(currentAction);
      Serial.println(" . Agora, preciso que insira o cartão da sala que deseja realizar esta ação.");
      Serial.println("Caso queira cancelar, insirar um cartão de ação antes de qualquer cartão individual.");
    } else {
      Serial.println("Neste momento, é necessário que você insira um cartão de ação.");
    }
    delay(1000);
  }
}

//=====================================CHECK FOR READING===============================
uint8_t WaitingCard() {
  if ( ! mfrc522.PICC_IsNewCardPresent()) { //Se um novo PICC colocado no leitor RFID continuar
    return 0;
  }
  if ( ! mfrc522.PICC_ReadCardSerial()) {   //Desde que um PICC colocado obtenha o Serial e continue
    return 0;
  }

  return 1;
}
//==================================TAKES UID CARD TAG=============================
void saveCurrentUid() {
  String content = "";
  for ( uint8_t i = 0; i < 4; i++) {
    currentByteTag[i] = mfrc522.uid.uidByte[i];
    content.concat(String(currentByteTag[i] < 0x10 ? " 0" : " "));
    content.concat(String(currentByteTag[i], HEX));
  }
  content.toUpperCase();
  currentStringTag = content;
}

//======================CHECK WHAT ACTION DO YOU WANT=====================
boolean verifyAction() {
  saveCurrentUid();
  Serial.println(currentStringTag.substring(1));
  if (currentStringTag.substring(1) == actionTagAdd) {
    if (currentAction == "") {
      currentAction = "ADD";
    }
    return true;
  } else if (currentStringTag.substring(1) == actionTagDelete) {
    if (currentAction == "") {
      currentAction = "DELETE";
    }
    return true;
  } else {
    return false;
  }
}

boolean isActionMode() {
  if (currentAction != "") {
    return true;
  }
  return false;
}

//==============================GET ROOM TAG===========================
boolean isRoomTag() {
  saveCurrentUid();
  if (verifyAction() == true) {
    gettingTags = false;
    return false;
  } else {
    for (int i = 0; i < 4; i++) {
      roomTag[i] = currentByteTag[i];
    }
    return true;
  }
}

boolean isCancel() {
  boolean cancel = true;

  for (int i = 0; i < 4; i++) {
    if (roomTag[i] != 0) {
      cancel = false;
    }
  }
  return cancel;
}


void resetProcess() {
  gettingTags = true;
  currentAction = "";
  currentStringTag = "";
  waitUserTag = false;

  for (int i = 0; i < 4; i++) {
    currentByteTag[i] = 0;
    roomTag[i] = 0;
    userTag[i] = 0;
  }
  delay(1000);
  Serial.println("");
  Serial.println(F("Insira a tag-ação que deseja executar"));
}
/*
  boolean isUserTag() {
  saveCurrentUid();
  boolean isUserTag = false;
  if (verifyAction() == true) {
    gettingTags = false;
  } else {
    for (int i = 0; i < 4; i++) {
      if (currentByteTag[i] != roomTag[i]) {
        isUserTag = true;
      }
    }
  }

  if (isUserTag) {
    for (int i = 0; i < 4; i++) {
      userTag[i] = currentByteTag[i];
    }
  }
  return isUserTag;
  }
*/
