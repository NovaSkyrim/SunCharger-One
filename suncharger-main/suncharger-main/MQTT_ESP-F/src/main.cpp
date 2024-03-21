// https://github.com/knolleary/pubsubclient/blob/master/examples/mqtt_esp8266/mqtt_esp8266.ino

#include <Arduino.h> 
#include <ESP8266WiFi.h>   // permet la connexion du module ESP8266 à la WiFi
#include <PubSubClient.h>  // permet d'envoyer et de recevoir des messages MQTT

#include <SPI.h>
#include <MFRC522.h>

#include "Wire.h"
#include "Adafruit_INA219.h"
Adafruit_INA219 ina219;

#define PROJECT_NAME "TEST_MQTT"                          // nom du projet
const char* ssid = "ESME-FABLAB";                         // indiquer le SSID de votre réseau
const char* password = "ESME-FABLAB";                     // indiquer le mdp de votre réseau
const char* mqtt_server = "192.168.1.203";                 // adresse du serveur MQTT auquel vous etes connecté

/** WIFI **/
WiFiClient espClient;
PubSubClient client(espClient);

/* MQTT */
#define TPC_NAME_SIZE   80
char inTopic[TPC_NAME_SIZE];
char outTopic[TPC_NAME_SIZE];

/* RFID */
const byte bonUID[4] = {245,100,55,70};
// byte buzz = 8;
// const int pinLEDVerte = 12; // LED verte
// const int pinLEDRouge = 11; // LED rouge

// setup des pin.s de la carte pour la connexion avec le module RFID
const int pinRST = D3; // pin RST du module RC522
const int pinSS = D8; // pin SS du module RC522
const int pinMOSI = D7; // pin MOSI du module RC522
const int pinMISO = D6; // pin MISO du module RC522
const int pinSCK = D5; // pin SCK du module RC522
const int pinSDA = pinSS; // pin SDA du module RC522
MFRC522 rfid(pinSDA, pinRST);

/* VARIABLES TEMPORAIRES */
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];
unsigned long lastMsg = 0;
int value = 0;

/* Fonction de paramètrage du WiFi */
void setup_wifi() {
    delay(10);
    // Nous affichons le nom du réseau WiFi sur lequel nous souhaitons nous connecter
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    // Configuration du WiFi pour faire une connexion à une borne WiFi
    WiFi.mode(WIFI_STA);

    // Connexion au réseau WiFi "ssid" avec le mot de passe contenu dans "password"
    WiFi.begin(ssid, password);
    
    // Tant que le WiFi n'est pas connecté, on attends!
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");

    // Affichage de l'adresse IP du module
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

/* Fonction appelé lors de la réception de donnée via le MQTT */
void callback(char* topic, byte* payload, unsigned int length) {

    // Afficher le message reçu
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i = 0; i < length; i++) {
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
    while (!client.connected()) {
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
            Serial.println(" try again in 5 seconds");

            // Attente de 5 secondes avant une nouvelle tentative
            delay(5000);
        }
    }
}

void setup() {
    // Paramètrage de la pin BUILTIN_LED en sortie
    pinMode(LED_BUILTIN, OUTPUT);
    
    // Configuration de la communication série à 115200 Mbps
    Serial.begin(115200);
    
    Serial.println();
    // Serial.println();
    // Serial.begin(9600);
    if (! ina219.begin()) {
        Serial.println("Erreur pour trouver le INA219");
        while (1) { delay(10); }
    }
    
    Serial.print("Courant"); 
    Serial.print("\t");

    // Connexion au WiFi
    setup_wifi();

    // Configuration de la connexion au broker MQTT
    client.setServer(mqtt_server, 1883);

    SPI.begin();
    rfid.PCD_Init();
    // pinMode(pinLEDVerte, OUTPUT);
    // pinMode(pinLEDRouge, OUTPUT);
    // pinMode(buzz, OUTPUT);

    // Déclaration de la fonction de récupération des données reçues du broker MQTT
    client.setCallback(callback);
}

void send(float &my_value, const char * my_topic ) {
    // Construction du message à envoyer
    // float current_mA = 0;
    // float voltage_V = 0;
    // float shunt_voltage_mV = 0;
    float value = my_value;
    // current_mA = ina219.getCurrent_mA();
    // voltage_V = ina219.getBusVoltage_V();
    // shunt_voltage_mV = ina219.getShuntVoltage_mV();
    // init_value = value;
    // Serial.println(current_mA);
    // Serial.println(voltage_V);
    // Serial.println(shunt_voltage_mV);
    // Serial.println(value);
    // delay(1000);
    // float value = init_value;
    // value = init_value;
    snprintf (msg, MSG_BUFFER_SIZE,"%f", value);
    Serial.print("Publish message: ");
    Serial.print(msg);
    Serial.print(" topic: ");
    Serial.println(my_topic);
    
    // Construction du topic d'envoi
    snprintf(outTopic, TPC_NAME_SIZE, my_topic);
    // outTopic => /ESME/COMPTEUR/outTopic

    // Envoi de la donnée
    client.publish(outTopic, msg);
}

const char* hoster = mqtt_server;
// const char* hoster = "cold-turtle-14.hooks.n8n.cloud/webhook/innov/id=WDXFGHK";

void loop() {

    
    WiFiClient client_local;

    // Si perte de connexion, reconnexion!
    if (!client.connected()) {
      reconnect();
    }

    // Appel de fonction pour redonner la main au process de communication MQTT
    client.loop();

    // Sous programme de test pour un envoi périodique
    unsigned long now = millis();
    if (now - lastMsg > 2000) {
        // Enregistrement de l'action réalisée
        lastMsg = now;

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
        // current_mA *= 1;
        Serial.println();
        send(current_mA, "ESME/COMPTEUR_AMP");
        send(voltage_V, "ESME/COMPTEUR_VOL");
        send(shunt_voltage_mV, "ESME/COMPTEUR_SmV");
        // // delay(1000);
        // value = current_mA*10;
        // snprintf (msg, MSG_BUFFER_SIZE,"%d", value);
        // Serial.print("Publish message: ");
        // Serial.println(msg);
        
        // // Construction du topic d'envoi
        // snprintf(outTopic, TPC_NAME_SIZE, "ESME/COMPTEUR");
        // // outTopic => /ESME/COMPTEUR/outTopic

        // // Envoi de la donnée
        // client.publish(outTopic, msg);
    }

    int refus = 0; // quand cette variable n'est pas nulle, c'est que le code est refusé
    u8 myRFID[4] = {1, 0, 72, 0};
    for (u8 i = 0; i < 4; i ++) {Serial.print(myRFID[i]);Serial.print(" ");}
    Serial.println();
    // if (rfid.PICC_IsNewCardPresent()) { // on a dédecté un tag
    //     if (rfid.PICC_ReadCardSerial()) { // on a lu avec succès son contenu
    //         for (byte i = 0; i < rfid.uid.size; i++) { // comparaison avec le bon UID
    //             Serial.print(rfid.uid.uidByte[i], HEX);
    //             myRFID[i] = rfid.uid.uidByte[i];
    //             if (rfid.uid.uidByte[i] != bonUID[i]) {
    //                 refus++;
    //                 // myRFID 
    //             }
    //         }
            for (u8 i = 0; i < 4; i ++) {Serial.print(myRFID[i]);Serial.print(" ");}
            Serial.println();

            Serial.printf("\n[Connecting to %s ... ", hoster);
            if (client_local.connect(hoster, 80)) {
                Serial.println("connected]");

                Serial.println("[Sending a request]");
                // Serial.print(String("GET /") + " HTTP/1.1\r\n" +
                //             "Host: " + hoster + "\r\n" +
                //             "Connection: close\r\n" +
                //             "\r\n"
                //             );
                
                Serial.println(String("GET / HTTP/1.1\r\nHost: 192.168.1.203\r\nUser-Agent: PostmanRuntime/7.29.0\r\nAccept: */*\r\nContent-Length: 0\r\nAccept-Encoding: gzip, deflate, br\r\nConnection: keep-alive\r\n\r\n"));
                client_local.print(String("GET / HTTP/1.1\r\nHost: 192.168.1.203\r\nUser-Agent: PostmanRuntime/7.29.0\r\nAccept: */*\r\nContent-Length: 0\r\nAccept-Encoding: gzip, deflate, br\r\nConnection: keep-alive\r\n\r\n")
                //  + " HTTP/1.1\r\n" +
                            // "Host: " + hoster + "/index.php\r\n" +
                            // "Host: 192.168.1.203\r\n" +
                            // "Connection: keep-alive\r\n" +
                            // "User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_13_6) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/68.0.3440.106 Safari/537.36\r\n" +
                            // "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8\r\n" + 
                            // "Accept-Encoding: gzip, deflate, br\r\n" +
                            // "Accept-Language: en-US,en;q=0.9\r\n" +
                            // "\r\n"
                            );

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

        //     if (refus == 0) {// UID accepté
        //         // on allume la LED verte pendant trois secondes
        //         // digitalWrite(pinLEDVerte, HIGH);
        //         // tone(buzz,523,50);
        //         // delay(50);
        //         // tone(buzz, 783, 50);
        //         // delay(50);
        //         // tone(buzz, 1046, 50);
        //         // delay(50);
        //         // tone(buzz, 1568, 50);
        //         // delay(50);
        //         // tone(buzz, 2093, 70);
        //         // delay(250);
        //         // delay(2550);
        //         // digitalWrite(pinLEDVerte, LOW);

        //         Serial.println("got it");
        //     }

        //     else {  // UID refusé
        //         // on allume la LED rouge pendant trois secondes
        //         // digitalWrite(pinLEDRouge, HIGH);
        //         // tone(buzz,370,50);
        //         // delay(100);
        //         // tone(buzz, 370, 300);
        //         // delay(1000);
        //         // delay(1900);
        //         // digitalWrite(pinLEDRouge, LOW);
                
        //         Serial.println("not it");
        //     }
        // }
    // }
    
    // WiFiClient client_local;

    // const char* hoster = "cold-turtle-14.hooks.n8n.cloud/webhook/innov/id=WDXFGHK";
    // const char* hoster = mqtt_server;
    // Serial.printf("\n[Connecting to %s ... ", hoster);
    // if (client_local.connect(hoster, 80)) {
    //     Serial.println("connected]");

    //     Serial.println("[Sending a request]");
    //     client_local.print(String("GET /") + " HTTP/1.1\r\n" +
    //                 "Host: " + hoster + "\r\n" +
    //                 "Connection: close\r\n" +
    //                 "\r\n"
    //                 );

    //     Serial.println("[Response:]");
    //     while (client_local.connected() || client_local.available()) {
    //         if (client_local.available()) {
    //             String line = client_local.readStringUntil('\n');
    //             Serial.println(line);
    //         }
    //     }
    //     client_local.stop();
    //     Serial.println("\n[Disconnected]");
    // } else {
    //     Serial.println("connection failed!]");
    //     client_local.stop();
    // }
    delay(1000);
    // delay(100);
}


// #include <ESP8266WiFi.h>

// const char* ssid = "********";
// const char* password = "********";

// const char* host = "www.example.com";


// void setup()
// {
//   Serial.begin(115200);
//   Serial.println();

//   Serial.printf("Connecting to %s ", ssid);
//   WiFi.begin(ssid, password);
//   while (WiFi.status() != WL_CONNECTED)
//   {
//     delay(500);
//     Serial.print(".");
//   }
//   Serial.println(" connected");
// }


// void loop()
// {
//   WiFiClient client;

//   Serial.printf("\n[Connecting to %s ... ", host);
//   if (client.connect(host, 80))
//   {
//     Serial.println("connected]");

//     Serial.println("[Sending a request]");
//     client.print(String("GET /") + " HTTP/1.1\r\n" +
//                  "Host: " + host + "\r\n" +
//                  "Connection: close\r\n" +
//                  "\r\n"
//                 );

//     Serial.println("[Response:]");
//     while (client.connected() || client.available())
//     {
//       if (client.available())
//       {
//         String line = client.readStringUntil('\n');
//         Serial.println(line);
//       }
//     }
//     client.stop();
//     Serial.println("\n[Disconnected]");
//   }
//   else
//   {
//     Serial.println("connection failed!]");
//     client.stop();
//   }
//   delay(5000);
// }