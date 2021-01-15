/* PROJETO ESCOLA V3.0

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

byte storedCard[4];   // Armazena uma ID lida da EEPROM
byte readCard[4];   // Armazena a identificação digitalizada do módulo RFID

int block = "";

// Crie a instância MFRC522.
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
  Serial.println(F("Leitura dos Dados :"));
}

///////////////////////////////////////// Main Loop ///////////////////////////////////
void loop () {


  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  byte block;
  byte len;

  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;

  }

  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;

  }

  Serial.println (F("Cartão Detectado"));

  byte buffer[18];

  block = 8;
  len = 18;

  ///////////// Autenticação ////////////////////////

  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 8, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Authentication failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  ///////////// Leitura ////////////////////////

  status = mfrc522.MIFARE_Read(block, buffer, &len);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Reading failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  String content = "";
  for (uint8_t i = 0; i < 16; i++)
  {
     Serial.print(buffer[i] < 0x10 ? " 0" : " ");
     Serial.print(buffer[i], HEX);
     content.concat(String(buffer[i] < 0x10 ? " 0" : " "));
     content.concat(String(buffer[i], HEX));
  }
  Serial.println();
  Serial.print("Message : ");
  content.toUpperCase();

  Serial.println(F("Finalizado"));
 
  delay(1000);
 
  mfrc522.PICC_HaltA();//PARA
  mfrc522.PCD_StopCrypto1();


  /*do {
    successRead = getID();            // define o sucesso Leia para 1 quando formos ler do leitor, caso contrário, 0
    digitalWrite(blueLed, LED_ON);    // Visualize o cartão mestre precisa ser definido
    delay(100);
    digitalWrite(blueLed, LED_OFF);
    delay(100);
    }
  while (!successRead);
  if (programMode) {
    if ( isMaster() ) { //Quando no modo de programa verificar primeiro Se o cartão mestre for digitalizado novamente para sair do modo de programa
      Serial.println(F("Master Card Scanned"));
      Serial.println(F("Exiting Program Mode"));
      Serial.println(F("-----------------------------"));
      programMode = false;
      return;
    }
    else {
      if ( findID(readCard) ) { // Se o cartão digitalizado for conhecido, exclua-o
        Serial.println(F("I know this PICC, removing..."));
        deleteID(readCard);
        Serial.println("-----------------------------");
        Serial.println(F("Scan a PICC to ADD or REMOVE to EEPROM"));
      }
      else {                    // Se o cartão digitalizado não for conhecido, adicione-o
        Serial.println(F("I do not know this PICC, adding..."));
        writeID(readCard);
        Serial.println(F("-----------------------------"));
        Serial.println(F("Scan a PICC to ADD or REMOVE to EEPROM"));
      }
    }
  }
  else {
    if ( isMaster()) {    // Se o ID do cartão digitalizado corresponder ao ID da placa principal - entre no modo de programa
      programMode = true;
      Serial.println(F("Hello Master - Entered Program Mode"));
      uint8_t count = EEPROM.read(0);   // Leia o primeiro Byte da EEPROM que
      Serial.print(F("I have "));     // armazena o número de IDs na EEPROM
      Serial.print(count);
      Serial.print(F(" record(s) on EEPROM"));
      Serial.println("");
      Serial.println(F("Scan a PICC to ADD or REMOVE to EEPROM"));
      Serial.println(F("Scan Master Card again to Exit Program Mode"));
      Serial.println(F("-----------------------------"));

    }

    else {
      if ( findID(readCard) ) { // Se não, veja se o cartão está na EEPROM
        Serial.println(F("Welcome, You shall pass"));
        granted(500);         // MANTENHA O NIVEL AUTO POR 5000 ms
      }
      else {      // Caso contrário, mostre que o ID não era válido
        Serial.println(F("You shall not pass"));
        denied();
      }
    }
  }*/
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
  /* digitalWrite(relay, LOW);     // NIVEL BAIXO
    delay(50);          // NIVEL BAIXO POR ALGUNS SEGUNDOS
    digitalWrite(relay, HIGH);    // NIVEL ALTO
    delay(50);            // Mantenha o LED verde aceso por um SEGUNDO
    digitalWrite(relay, LOW);     // NIVEL BAIXO
    delay(50);          // NIVEL BAIXO POR ALGUNS SEGUNDOS
    digitalWrite(relay, HIGH);    // NIVEL ALTO
    delay(50);            // Mantenha o LED verde aceso por um SEGUNDO
    digitalWrite(relay, LOW);     // NIVEL BAIXO
    delay(50);          // NIVEL BAIXO POR ALGUNS SEGUNDOS
    digitalWrite(relay, HIGH);    // NIVEL ALTO
    delay(50);            // Mantenha o LED verde aceso por um SEGUNDO*/
}

///////////////////////////////////////// Access Denied  ///////////////////////////////////
void denied() {
  digitalWrite(greenLed, LED_OFF);  // Certifique-se de que o LED verde esteja apagado
  digitalWrite(blueLed, LED_OFF);   // MCertifique-se de que o LED AZUL esteja apagado
  digitalWrite(redLed, LED_ON);   // Ligue o LED vermelho
  delay(1000);
}


///////////////////////////////////////// Get PICC's UID ///////////////////////////////////
uint8_t getID() {
  // Getting ready for Reading PICCs
  if ( ! mfrc522.PICC_IsNewCardPresent()) { //Se um novo PICC colocado no leitor RFID continuar
    return 0;
  }
  if ( ! mfrc522.PICC_ReadCardSerial()) {   //Desde que um PICC colocado obtenha o Serial e continue
    return 0;
  }
  /* Há PICCs Mifare que têm cuidado UID de 4 ou 7 bytes se você usar PICC de 7 bytes
    Acho que devemos assumir todos os PICC como eles têm 4 byte UID
    Até suportarmos PICCs de 7 bytes */
  Serial.println(F("Scanned PICC's UID HEX:"));
  for ( uint8_t i = 0; i < 4; i++) {  //
    readCard[i] = mfrc522.uid.uidByte[i];

    Serial.print(readCard[i]);
  }

  Serial.println("");
  mfrc522.PICC_HaltA(); // PARAR LEITURA
  return 1;
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

//////////////////////////////////////// Read an ID from EEPROM //////////////////////////////
void readID( uint8_t number ) {
  uint8_t start = (number * 4 ) + 2;    // Descobrir a posição inicial
  for ( uint8_t i = 0; i < 4; i++ ) {     // Loop 4 vezes para obter os 4 bytes
    storedCard[i] = EEPROM.read(start + i);   // Atribuir valores lidos da EEPROM para o array
  }
}

///////////////////////////////////////// Add ID to EEPROM   ///////////////////////////////////
void writeID( byte a[] ) {
  if ( !findID( a ) ) {     // Antes de escrevermos para a EEPROM, verifique se já vimos este cartão antes!
    uint8_t num = EEPROM.read(0);     // Obter o numer de espaços usados, a posição 0 armazena o número de cartões de identificação
    uint8_t start = ( num * 4 ) + 6;  // Descubra onde o próximo slot começa
    num++;                // Incrementar o contador por um
    EEPROM.write( 0, num );     // Escreva a nova contagem para o contador
    for ( uint8_t j = 0; j < 4; j++ ) {   // Laço de 4 vezes
      EEPROM.write( start + j, a[j] );  // Escreva os valores da matriz para EEPROM na posição correta
    }
    successWrite();
    Serial.println(F("Succesfully added ID record to EEPROM"));
    tone(buzzer, 500);
    delay(300);
    noTone(buzzer);
    tone(buzzer, 3000);
    delay(300);
    noTone(buzzer);
  }
  else {
    failedWrite();
    Serial.println(F("Failed! There is something wrong with ID or bad EEPROM"));
  }
}

///////////////////////////////////////// Remove ID from EEPROM   ///////////////////////////////////
void deleteID( byte a[] ) {
  if ( !findID( a ) ) {     //Antes de excluirmos da EEPROM, verifique se temos este cartão!
    failedWrite();      // If not
    Serial.println(F("Failed! There is something wrong with ID or bad EEPROM"));
  }
  else {
    uint8_t num = EEPROM.read(0);   // Obter o numer de espaços usados, a posição 0 armazena o número de cartões de identificação
    uint8_t slot;       // Descobrir o número do slot do cartão
    uint8_t start;      // = ( num * 4 ) + 6; // Descubra onde o próximo slot começa
    uint8_t looping;    //O número de vezes que o loop é repetido
    uint8_t j;
    uint8_t count = EEPROM.read(0); // Leia o primeiro Byte da EEPROM que armazena o número de cartões
    slot = findIDSLOT( a );   // Descobrir o número do slot do cartão para excluir
    start = (slot * 4) + 2;
    looping = ((num - slot) * 4);
    num--;      //Decrementar o contador por um
    EEPROM.write( 0, num );   // Escreva a nova contagem para o contador
    for ( j = 0; j < looping; j++ ) {         // Laço os tempos de troca de cartão
      EEPROM.write( start + j, EEPROM.read(start + 4 + j));   // Mude os valores da matriz para 4 lugares antes na EEPROM
    }
    for ( uint8_t k = 0; k < 4; k++ ) {         // Loop de mudança
      EEPROM.write( start + j + k, 0);
    }
    successDelete();
    Serial.println(F("Succesfully removed ID record from EEPROM"));
    tone(buzzer, 3000);
    delay(200);
    noTone(buzzer);
    tone(buzzer, 500);
    delay(300);
    noTone(buzzer);

  }
}

///////////////////////////////////////// Check Bytes   ///////////////////////////////////
boolean checkTwo ( byte a[], byte b[] ) {
  if ( a[0] != 0 )      // Certifique-se de que há algo no array primeiro
    match = true;       //Suponha que eles combinem primeiro
  for ( uint8_t k = 0; k < 4; k++ ) {   //Laço de 4 vezes
    if ( a[k] != b[k] )     // SE a! = B, em seguida, defina match = false, um falhará, todos falharão
      match = false;
  }
  if ( match ) {      // Verifique se a correspondência ainda é verdadeira
    return true;      // Retorno verdadeiro
  }
  else  {
    return false;       //Retorna falso
  }
}

///////////////////////////////////////// Find Slot   ///////////////////////////////////
uint8_t findIDSLOT( byte find[] ) {
  uint8_t count = EEPROM.read(0);       // Leia o primeiro Byte da EEPROM que
  for ( uint8_t i = 1; i <= count; i++ ) {    //Loop uma vez para cada entrada da EEPROM
    readID(i);                // Leia uma ID da EEPROM, ela é armazenada no cartão armazenado [4]
    if ( checkTwo( find, storedCard ) ) {   // Verifique para ver se o cartão armazenado leu de EEPROM
      //é o mesmo que o cartão de identificação find [] passado
      return i;         // O número do slot do cartão
      break;          // Pare de olhar, encontramos
    }
  }
}

///////////////////////////////////////// Find ID From EEPROM   ///////////////////////////////////
boolean findID( byte find[] ) {
  uint8_t count = EEPROM.read(0);     // Leia o primeiro Byte da EEPROM que
  for ( uint8_t i = 1; i <= count; i++ ) {    // Loop uma vez para cada entrada da EEPROM
    readID(i);          // Leia uma ID da EEPROM, ela é armazenada no cartão armazenado [4]
    if ( checkTwo( find, storedCard ) ) {   // Verifique para ver se o cartão armazenado leu de EEPROM
      return true;
      break;  // Stop looking we found it
    }
    else {    // If not, return false
    }
  }
  return false;
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
// Verifique se o ID passado é o cartão mestre de programação
boolean isMaster() {
  Serial.print("UID tag :");
  String content = "";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.println();
  Serial.print("Message : ");
  content.toUpperCase();

  if (content.substring(1) == "3B 1F 89 03" or content.substring(1) == "76 D8 AD 1F") //change here the UID of the card/cards that you want to give access 76 D8 AD 1F
  {
    Serial.println("Authorized access");
    Serial.println();
    delay(1000);
    return true;
  } else   {
    Serial.println(" Access denied");
    return false;
  }

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
