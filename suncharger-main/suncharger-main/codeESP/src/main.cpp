// https://github.com/knolleary/pubsubclient/blob/master/examples/mqtt_esp8266/mqtt_esp8266.ino

#include <Arduino.h> 
#include <ESP8266WiFi.h>   // permet la connexion du module ESP8266 à la WiFi
#include <PubSubClient.h>  // permet d'envoyer et de recevoir des messages MQTT

#include <SPI.h>
#include <MFRC522.h>
#include <EEPROM.h> //https://github.com/esp8266/Arduino/blob/master/libraries/EEPROM/EEPROM.h

#include "Wire.h"
#include "Adafruit_INA219.h"
Adafruit_INA219 ina219;

#define EEPROM_SIZE 2<<7
// /* FabLab Network */
// #define PROJECT_NAME "TEST_MQTT"                   // nom du projet // à enlever ?
// #define SSID "ESME-FABLAB"                         // indiquer le SSID de votre réseau
// #define PASSWORD "ESME-FABLAB"                     // indiquer le mdp de votre réseau
// #define IP_RASPBERRY "192.168.1.202"               // adresse du serveur MQTT auquel vous etes connecté
// #define PORT_RASPBERRY 5678
// /* Raps Access Point Network */
#define SSID "raspi-webgui"                     // indiquer le SSID de votre réseau
#define PASSWORD "ChangeMe"                     // indiquer le mdp de votre réseau
#define IP_RASPBERRY "10.3.141.1"               // adresse du serveur MQTT auquel vous etes connecté
#define PORT_RASPBERRY 5678
#define NOMBRE_CASIER 4

/** WIFI **/
WiFiClient espClient;
PubSubClient client(espClient);
bool wifi_connected = false;

/* MQTT */
#define TPC_NAME_SIZE 80
char inTopic[TPC_NAME_SIZE];
char outTopic[TPC_NAME_SIZE];

/* RFID */
const byte bonUID[NOMBRE_CASIER][4] = {{245,100,55,70}};

/* setup des pin.s de la carte pour la connexion avec le module RFID */
const int pinRST    = D3; // pin RST du module RC522
const int pinSS     = D8; // pin SS du module RC522
const int pinMOSI   = D7; // pin MOSI du module RC522
const int pinMISO   = D6; // pin MISO du module RC522
const int pinSCK    = D5; // pin SCK du module RC522
const int pinSDA    = pinSS; // pin SDA du module RC522
MFRC522 rfid(pinSDA, pinRST);

/* VARIABLES TEMPORAIRES */
#define MSG_BUFFER_SIZE	50
char msg[MSG_BUFFER_SIZE];
unsigned long lastMsg = 0;
int value = 0;

/* Fonction de paramètrage du WiFi */
void setup_wifi() {
    delay(10);
    // Nous affichons le nom du réseau WiFi sur lequel nous souhaitons nous connecter
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(SSID);

    // Configuration du WiFi pour faire une connexion à une borne WiFi
    WiFi.mode(WIFI_STA);

    // Connexion au réseau WiFi "SSID" avec le mot de passe contenu dans "password"
    WiFi.begin(SSID, PASSWORD);
    
    // Tant que le WiFi n'est pas connecté, on attends!
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");

    // Affichage de l'adresse IP du module
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

/* Fonction appelé lors de la réception de donnée via le MQTT */
void callback(char* topic, byte* payload, unsigned int length) {

    // Afficher le message reçu
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (unsigned int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();

    //********************************//
    // TRAITEMENT DES DONNEES RECUES
    //********************************//
    
}

/* Fonction de reconnexion au broker MQTT */
void reconnect() {
    // Tant que le client n'est pas connecté...
    // while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        
        // Génération d'un identifiant unique
        String clientId = "ESP8266Client-";
        clientId += String(random(0xffff), HEX);
        
        // Tentative de connexion
        if (client.connect(clientId.c_str())) {

            // Connexion réussie
            Serial.println("connected");

            // Abonnement aux topics au près du broker MQTT
            snprintf(inTopic, TPC_NAME_SIZE, "ESME/#");
            
            // inTopic => /ESME/COMPTEUR/inTopic
            client.subscribe(inTopic);

        } else {

            // Tentative échouée
            Serial.print("failed, rc=");
            Serial.print(client.state());
            // Serial.println(" try again in 5 seconds");

            // Attente de 5 secondes avant une nouvelle tentative
            // delay(5000);
        }
    // }
}

void setup_pins() {
    // Paramètrage de la pin BUILTIN_LED en sortie
    pinMode(LED_BUILTIN, OUTPUT);

    // pinMode(pinLEDVerte, OUTPUT);
    // pinMode(pinLEDRouge, OUTPUT);
    // pinMode(buzz, OUTPUT);
}

void send_MQTT(float &my_value, const char * my_topic ) {
    snprintf(msg, MSG_BUFFER_SIZE,"%f", my_value);
    Serial.print("[MQTT] Publish message: ");
    Serial.print(msg);
    Serial.print(" topic: ");
    Serial.println(my_topic);
    
    // Construction du topic d'envoi
    snprintf(outTopic, TPC_NAME_SIZE, my_topic);
    // outTopic => /ESME/COMPTEUR/my_topic

    // Envoi de la donnée
    client.publish(outTopic, msg);
}
   
bool connect_serveur_HTML(WiFiClient wicli) {
    // WiFiClient client_local;
    return wicli.connect(IP_RASPBERRY, PORT_RASPBERRY);
}
bool healthy_HTML(WiFiClient wicli) {
    return wicli.connected();
}

void disconnecte_HTML(WiFiClient wicli) {
    wicli.stop();
}

void HTML_print_send(WiFiClient wicli, String parameters) {
    
    Serial.println("[Begin of the message]");
    String message = String("GET /webhook/innov?") + parameters + " HTTP/1.1\r\n" +
                    "Host: " + IP_RASPBERRY + "\r\n" +
                    "Connection: close\r\n" +
                    "\r\n";
    Serial.println(message);
    Serial.println("[End of the message]");

    wicli.print(message);
}

String HTML_print_response(WiFiClient wicli) {
    Serial.println("[Response:]");
    String respons = "";
    while (wicli.connected() || wicli.available()) {
        if (wicli.available()) {
            respons += wicli.readStringUntil('\n');// may need to as '\n' at the end
            // Serial.println(line);
        }
    }
    Serial.println(respons);
    return respons;
}

void HTTP_connect_send_and_print(String parameters) {

    WiFiClient client_local;
    Serial.printf("\n[Connecting to %s ... ", IP_RASPBERRY);
    if (client_local.connect(IP_RASPBERRY, PORT_RASPBERRY)) {
        Serial.println("connected]");

        Serial.println("[Sending a request]");
        String message = String("GET /webhook/innov?") + parameters + " HTTP/1.1\r\n" +
                        "Host: " + IP_RASPBERRY + "\r\n" +
                        "Connection: close\r\n" +
                        "\r\n";
        Serial.println(message);
        client_local.print(message);

        Serial.println("[Response:]");
        while (client_local.connected() || client_local.available()) {
            if (client_local.available()) {
                String line = client_local.readStringUntil('\n');
                Serial.println(line);
            }
        }
        client_local.stop();
        Serial.println("\n[Disconnected]");
    } else {
        Serial.println("connection failed!]");
        client_local.stop();
    }
}

void EEPROM_write(int adresse, float param) {
    // Init EEPROM
    EEPROM.begin(EEPROM_SIZE);

    //Write data into eeprom
    int write_address = adresse;
    Serial.print("WRITE");

    Serial.print(" | size = ");
    Serial.print(sizeof(param));

    Serial.print(" | address = ");
    Serial.print(write_address);

    EEPROM.put(write_address, param);
    write_address += sizeof(param);

    Serial.print("..");
    Serial.print(write_address);
    
    Serial.print(" | param = ");
    Serial.print(param);
    Serial.println();

    EEPROM.commit();
    EEPROM.end();
}

#define EEPROM_READ(address, type) ({type tmp; _EEPROM_read(address, sizeof(tmp)) ; })
float _EEPROM_read(int adresse, int sizeofparam) {
    //Init EEPROM
    EEPROM.begin(EEPROM_SIZE);

    //Read data from eeprom
    int read_address = adresse;
    Serial.print("READ ");
    
    Serial.print(" | size = ");
    Serial.print(sizeofparam);

    Serial.print(" | address = ");
    Serial.print(read_address);

    float readParam;
    EEPROM.get(read_address, readParam); //readParam=EEPROM.readFloat(address);
    
    Serial.print("..");
    read_address += sizeof(readParam); //update address value
    Serial.print(read_address);
    
    Serial.print(" | val = ");
    Serial.print(readParam);

    Serial.println();
    EEPROM.end();

    return readParam;
}

bool RFID_read_print_and_recognize() {
    u8 currentRFID[4] = {0, 0, 0, 0};
    Serial.println();
    if (rfid.PICC_IsNewCardPresent()) { // on a dédecté un tag
        if (rfid.PICC_ReadCardSerial()) { // on a lu avec succès son contenu
            for (u8 i = 0; i < 4; i ++) { currentRFID[i] = rfid.uid.uidByte[i]; }
            for (u8 i = 0; i < 4; i ++) { Serial.print(currentRFID[i]); Serial.print(" "); }
            Serial.println();
        }
    }
    return false;
}

void MQTT_communication_info() {
    // Construction du message à envoyer
    float current_mA = 0;
    float voltage_V = 0;
    float shunt_voltage_mV = 0;
    current_mA = ina219.getCurrent_mA();
    voltage_V = ina219.getBusVoltage_V();
    shunt_voltage_mV = ina219.getShuntVoltage_mV();
    // Serial.println(current_mA);
    // Serial.println(voltage_V);
    // Serial.println(shunt_voltage_mV);
    Serial.println();
    send_MQTT(current_mA, "ESME/COMPTEUR_AMP");
    send_MQTT(voltage_V, "ESME/COMPTEUR_VOL");
    send_MQTT(shunt_voltage_mV, "ESME/COMPTEUR_SmV");
}


void setup() {
    // assignation des pins de l'ESP
    setup_pins();

    // Configuration de la communication série à 115200 Mbps
    Serial.begin(115200);
    Serial.println();

    // initialisation de la communication avec l'INA219
    // if (! ina219.begin()) {
    //     Serial.println("Erreur pour trouver le INA219");
    //     while (1) { delay(10); }
    // }

    // Connexion au WiFi
    // setup_wifi();

    // Configuration de la connexion au broker MQTT
    // client.setServer(IP_RASPBERRY, 1883);

    // Initialistaion du SPI (dépendances)
    // SPI.begin();

    // Configuration du RFID
    // rfid.PCD_Init();

    // Déclaration de la fonction de récupération des données reçues du broker MQTT
    // client.setCallback(callback);
    
    // EEPROM_write(0, 1.1);
    // EEPROM_write(4, 2.2);
    // EEPROM_write(8, 3.3);
    // EEPROM_write(12, 4.4);
    // EEPROM_READ(0, float);
    // EEPROM_READ(4, float);
    // EEPROM_READ(8, float);
    // EEPROM_READ(12, float);
}

void loop() {
    
    // Si perte de connexion, reconnexion!
    if (!client.connected()) { reconnect(); }

    // Appel de fonction pour redonner la main au process de communication MQTT
    client.loop();

    // init_connect_serveur_HTML();
    wifi_connected = connect_serveur_HTML(espClient);
    // Sous programme de test pour un envoi périodique
    unsigned long now = millis();
    if (now - lastMsg > 2000) {
        // check_health_HTML();
        wifi_connected = healthy_HTML(espClient);
        // Enregistrement de l'action réalisée
        lastMsg = now;
        MQTT_communication_info();
        // RFID_read_print_and_recognize();
        if (wifi_connected) {
            HTML_print_send(espClient, "id=hello");
            HTML_print_response(espClient);
            HTML_print_send(espClient, "id=WDXFGHKM");
            HTML_print_response(espClient);
            HTML_print_send(espClient, "id=WDXFGHKML");
            HTML_print_response(espClient);
        } else {
            espClient.stop();
            wifi_connected = connect_serveur_HTML(espClient);
        }
        // HTTP_connect_send_and_print("id=hello");
        // WDXFGHKML
        // HTTP_connect_send_and_print("id=WDXFGHKML");
        // WDXFGHKM
        // HTTP_connect_send_and_print("id=WDXFGHKM");
        // float test1 = EEPROM_READ(0, float);
        // float test2 = EEPROM_READ(4, float);
        // float test3 = EEPROM_READ(8, float);
        // float test4 = EEPROM_READ(12, float);
        // Serial.println();
        // EEPROM_write(0, test1+1);
        // EEPROM_write(4, test2+2);
        // EEPROM_write(8, test3+3);
        // EEPROM_write(12,test4+4);
    }
    delay(1000);
}

// #include <Adafruit_NeoPixel.h>

// #define NUM_LEDS 5

// const byte listeUID[4] = {245,100,55,70};
// //const int buzz = D0;
// const int pinLEDrgb = D4;
// const int pinRST = D3;
// const int pinSDA = D8; // pin SDA du module RC522
// const int relai = D10; // pin RX de l'ESP

// bool casier = true;

// Adafruit_NeoPixel pixels(NUM_LEDS, pinLEDrgb, NEO_GRB + NEO_KHZ800);
// MFRC522 rfid(pinSDA, pinRST);

// void setup()
// {
//   Serial.begin(9600);
//   Serial.println("Test1");
//   SPI.begin();
//   rfid.PCD_Init();
//   Serial.println("Test2");
  
//   pinMode(pinLEDrgb, OUTPUT);
//   // pinMode(buzz, OUTPUT);
//   pinMode(relai, OUTPUT);

//   pixels.begin();
//   pixels.show();
//   pixels.clear();

//   /* ANIMATION LED DÉBUT */

//   for(int i = 0;i<NUM_LEDS;i++){
//     pixels.setPixelColor(i, pixels.Color(0, 150, 0));
//     pixels.show();
//     delay(300);
//   }

//   pixels.clear();
//   pixels.show();

//   delay(200);

//   for(int i = 0;i<NUM_LEDS;i++){
//     pixels.setPixelColor(i, pixels.Color(0, 150, 0));
//     pixels.show();
//   }
//   Serial.println("Test2");
//   delay(200);
//   pixels.clear();
//   pixels.show();
//   Serial.println("Test3");

//   // tone(buzz,523,50);
//   // delay(50);
//   // tone(buzz, 783, 50);
//   // delay(50);
//   // tone(buzz, 1046, 50);
//   // delay(50);
//   // tone(buzz, 1568, 50);
//   // delay(50);
//   // tone(buzz, 2093, 70);
//   // delay(250);

//   /* ANIMATION LED FIN */
//   Serial.println("Test4");
// }

// void loop()
// {
//   Serial.println("Test5");
//   int refus = 0; // quand cette variable n'est pas nulle, c'est que le code est refusé

//   if (rfid.PICC_IsNewCardPresent())  // on a dédecté un tag
//   {
//     if (rfid.PICC_ReadCardSerial())  // on a lu avec succès son contenu
//     {
//       for (byte i = 0; i < rfid.uid.size; i++) // comparaison avec le bon UID
//       {
//         Serial.print(rfid.uid.uidByte[i], HEX);
//         if (rfid.uid.uidByte[i] != listeUID[i]) {
//           refus++;
//         }
//       }

//       if (refus == 0) // UID accepté
//       {
//         // on allume la LED verte
//         pixels.clear();
//         pixels.setPixelColor(4, pixels.Color(0, 150, 0));
//         pixels.show();

//         if (casier){
          
//           casier = !casier;

//           pixels.clear();
//           pixels.setPixelColor(4, pixels.Color(0, 150, 0));
//           pixels.show();

//           // tone(buzz,523,50);
//           // delay(50);
//           // tone(buzz, 783, 50);
//           // delay(50);
//           // tone(buzz, 1046, 50);
//           // delay(50);
//           // tone(buzz, 1568, 50);
//           // delay(50);
//           // tone(buzz, 2093, 70);
//           // delay(250);
          
//           digitalWrite(relai, 1);
//           delay(8000);
//           digitalWrite(relai, 0);

//           pixels.clear();
//         }
        
//         else{ // Casier déjà pris
//           pixels.clear();
//           pixels.setPixelColor(4, pixels.Color(150, 0, 0));
//           pixels.show();
//           // tone(buzz,370,50);
//           // delay(100);
//           // tone(buzz, 370, 300);
//         }
//         pixels.clear();
//       }

//       else   {  // UID refusé
//         // on allume la LED rouge
//         pixels.clear();
//         pixels.setPixelColor(4, pixels.Color(150, 0, 0));
//         pixels.show();
//         // tone(buzz,370,50);
//         // delay(100);
//         // tone(buzz, 370, 300);
//         // delay(2900);
//         pixels.clear();
//       }
//     }
//   }
// }


// //bool ouverture_relai(){
// //  
// //  const int n = -1;
// //  for (byte i = 0; i < 4; i++){
// //    
// //    n++;
// //    if (casiers_utilises[i]==0){
// //      
// //      if (n==0){
// //        digitalWrite(relai1, 1);
// //        delay(5000);
// //        digitalWrite(relai1, 0);
// //        return true
// //      }
// //      if (n==1){
// //        digitalWrite(relai2, 1);
// //        delay(5000);
// //        digitalWrite(relai2, 0);
// //        return true
// //      }
// //      if (n==2){
// //        digitalWrite(relai3, 1);
// //        delay(5000);
// //        digitalWrite(relai3, 0);
// //        return true
// //      }
// //      if (n==3){
// //        digitalWrite(relai4, 1);
// //        delay(5000);
// //        digitalWrite(relai4, 0);
// //        return true
// //      }
// //      else {
// //        return false
// //      }
// //    }
// //  }
// //}