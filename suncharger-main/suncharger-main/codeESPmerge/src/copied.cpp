// #include <Adafruit_NeoPixel.h>
// #include <SPI.h>
// #include <MFRC522.h>
// #include <Arduino.h> 

// #define SS_PIN D8
// #define RST_PIN D3
// #define NUM_LEDS 5

// byte * Wheel(byte WheelPos) {
//     static byte c[3];
  
//     if(WheelPos < 85) {
//         c[0] = WheelPos * 3;
//         c[1] = 255 - WheelPos * 3;
//         c[2] = 0;
//     } else if (WheelPos < 170) {
//         WheelPos -= 85;
//         c[0] = 255 - WheelPos * 3;
//         c[1] = 0;
//         c[2] = WheelPos * 3;
//     } else {
//         WheelPos -= 170;
//         c[0] = 0;
//         c[1] = WheelPos * 3;
//         c[2] = 255 - WheelPos * 3;
//     }

//     return c;
// }

// const byte listeUID[4] = {245,100,55,70};
// //const int buzz = D0;
// const int pinLEDrgb = D4;
// const int relai = 10; 

// bool casier = true;

// Adafruit_NeoPixel pixels(NUM_LEDS, pinLEDrgb, NEO_GRB + NEO_KHZ800);

// MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class

// MFRC522::MIFARE_Key key; 

// // Init array that will store new NUID 
// byte nuidPICC[4];

// /* Print Pretty Hexadecimal */
// void printHex(byte *buffer, byte bufferSize) {
//     for (byte i = 0; i < bufferSize; i++) {
//         Serial.print(buffer[i] < 0x10 ? " 0" : " ");
//         Serial.print(buffer[i], HEX);
//     }
// }

// /* Print Pretty Decimal */
// void printDec(byte *buffer, byte bufferSize) {
//     for (byte i = 0; i < bufferSize; i++) {
//         Serial.print(buffer[i] < 0x10 ? " 0" : " ");
//         Serial.print(buffer[i], DEC);
//     }
// }

// void rainbowCycle(int SpeedDelay) {
//     byte *c;
//     uint16_t i, j;

//     for (j = 0; j < 151*5; j++) { // 5 cycles of all colors on wheel
//         for (i = 0; i < NUM_LEDS; i++) {
//             c = Wheel(((i * 151 / NUM_LEDS) + j) & 150);
//             pixels.setPixelColor(i, pixels.Color(*c, *(c+1), *(c+2)));
//         }
//         pixels.show();
//         delay(SpeedDelay);
//         pixels.clear();
//     }
// }

// void setup_copied() {
//     Serial.begin(115200);
//     SPI.begin(); // Init SPI bus
//     rfid.PCD_Init(); // Init MFRC522 

//     for (byte i = 0; i < 6; i++) {
//         key.keyByte[i] = 0xFF;
//     }
    
//     pinMode(pinLEDrgb, OUTPUT);
//     // pinMode(buzz, OUTPUT);
//     pinMode(relai, OUTPUT);

//     pixels.begin();
//     pixels.show();
//     pixels.clear();

//     /* ANIMATION LED DÉBUT */

//     for (int i = 0; i < NUM_LEDS; i++){
//         pixels.setPixelColor(i, pixels.Color(0, 150, 0));
//         pixels.show();
//         delay(300);
//     }

//     rainbowCycle(5);

//     pixels.clear();
//     pixels.show();

//     delay(200);

//     for (int i = 0; i < NUM_LEDS; i++){
//         pixels.setPixelColor(i, pixels.Color(0, 150, 0));
//         pixels.show();
//     }
//     delay(200);
//     pixels.clear();
//     pixels.show();

//     // tone(buzz,523,50);
//     // delay(50);
//     // tone(buzz, 783, 50);
//     // delay(50);
//     // tone(buzz, 1046, 50);
//     // delay(50);
//     // tone(buzz, 1568, 50);
//     // delay(50);
//     // tone(buzz, 2093, 70);
//     // delay(250);

//     /* ANIMATION LED FIN */
// }

 
// void loop_copied() {

//     //rainbowCycle(5); // Arc-en-cieel

//     // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
//     if ( ! rfid.PICC_IsNewCardPresent()) {return;}

//     // Verify if the NUID has been readed
//     if ( ! rfid.PICC_ReadCardSerial()) {return;}

//     Serial.print("PICC type: ");
//     MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
//     Serial.println(rfid.PICC_GetTypeName(piccType));

//     // Check is the PICC of Classic MIFARE type
//     if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&  
//         piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
//         piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
//         Serial.println(F("Your tag is not of type MIFARE Classic."));
//         return;
//     }

//     if (rfid.uid.uidByte[0] != nuidPICC[0] || 
//         rfid.uid.uidByte[1] != nuidPICC[1] || 
//         rfid.uid.uidByte[2] != nuidPICC[2] || 
//         rfid.uid.uidByte[3] != nuidPICC[3] ) {

//         Serial.println("A new card has been detected.");

//         if (casier){
//             // Store NUID into nuidPICC array
//             for (byte i = 0; i < 4; i++) {
//                 nuidPICC[i] = rfid.uid.uidByte[i];
//             }
//         }
      
//         Serial.println(F("The NUID tag is:"));
//         Serial.print(F("In hex: "));
//         printHex(rfid.uid.uidByte, rfid.uid.size);
//         Serial.println();
//         Serial.print(F("In dec: "));
//         printDec(rfid.uid.uidByte, rfid.uid.size);
//         Serial.println();

//         // on allume la LED verte
//         pixels.clear();
//         pixels.setPixelColor(4, pixels.Color(0, 150, 0));
//         pixels.show();

//         if (casier) {
                
//             casier = !casier;

//             pixels.clear();
//             pixels.setPixelColor(4, pixels.Color(0, 150, 0));
//             pixels.show();

//             // tone(buzz,523,50);
//             // delay(50);
//             // tone(buzz, 783, 50);
//             // delay(50);
//             // tone(buzz, 1046, 50);
//             // delay(50);
//             // tone(buzz, 1568, 50);
//             // delay(50);
//             // tone(buzz, 2093, 70);
//             // delay(250);
            
//             digitalWrite(relai, HIGH);
//             delay(2000);
//             digitalWrite(relai, LOW);

//             pixels.clear();
//             pixels.show();
//         } else { // Casiers déjà pris
//             pixels.clear();
//             pixels.setPixelColor(4, pixels.Color(150, 0, 0));
//             pixels.show();
//             delay(1000);
//             // tone(buzz,370,50);
//             // delay(100);
//             // tone(buzz, 370, 300);
//         }
//         pixels.clear();
//         pixels.show();
//     }

//     else {

//       Serial.println(F("Card read previously."));

//       if (not casier){ // Retrait de son téléphone

//             casier = !casier;
//             pixels.clear();
//             pixels.setPixelColor(4, pixels.Color(0, 150, 0));
//             pixels.show();

//             // tone(buzz,523,50);
//             // delay(50);
//             // tone(buzz, 783, 50);
//             // delay(50);
//             // tone(buzz, 1046, 50);
//             // delay(50);
//             // tone(buzz, 1568, 50);
//             // delay(50);
//             // tone(buzz, 2093, 70);
//             // delay(250);
            
//             digitalWrite(relai, HIGH);
//             delay(2000);
//             digitalWrite(relai, LOW);

//             pixels.clear();
//             pixels.show();

//             byte nuidPICC[4] = {0,0,0,0};
//       } else {
//             casier = !casier;
//             pixels.clear();
//             pixels.setPixelColor(4, pixels.Color(0, 150, 0));
//             pixels.show();

//             // tone(buzz,523,50);
//             // delay(50);
//             // tone(buzz, 783, 50);
//             // delay(50);
//             // tone(buzz, 1046, 50);
//             // delay(50);
//             // tone(buzz, 1568, 50);
//             // delay(50);
//             // tone(buzz, 2093, 70);
//             // delay(250);
            
//             digitalWrite(relai, HIGH);
//             delay(2000);
//             digitalWrite(relai, LOW);

//             pixels.clear();
//             pixels.show();
//         }
//     }

//     // Halt PICC
//     rfid.PICC_HaltA();

//     // Stop encryption on PCD
//     rfid.PCD_StopCrypto1();

// }

