#include <SPI.h>
#include <MFRC522.h>
#define RELAY 8
#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.
void setup()
{
pinMode (RELAY,OUTPUT);
Serial.begin(9600);   // Initiate a serial communication
SPI.begin();      // Initiate  SPI bus
mfrc522.PCD_Init();   // Initiate MFRC522
Serial.println("Approximate your card to the reader...");
Serial.println();
}
void loop()
{
// Look for new cards
if ( ! mfrc522.PICC_IsNewCardPresent())
{
return;
}
// Select one of the cards
if ( ! mfrc522.PICC_ReadCardSerial())
{
return;
}
//Show UID on serial monitor
Serial.print("UID tag :");
String content= "";
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
Serial.print("RELAY: ");
content.toUpperCase();
if (content.substring(1) == "3B 1F 89 03" or content.substring(1) == "76 D8 AD 1F") //change here the UID of the card/cards that you want to give access 76 D8 AD 1F
{
Serial.println("Authorized access");
Serial.println();
Serial.println("Relay on");
digitalWrite (RELAY,LOW);
delay(1000);
}
else   {
Serial.println(" Access denied");
Serial.println(" RELAY off");
Serial.println(" SmartTonics");
digitalWrite(RELAY,HIGH);
}
}
