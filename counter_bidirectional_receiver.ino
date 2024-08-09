#include <SPI.h>              // include libraries
#include <LoRa.h>
 
const int csPin = 4;          // LoRa radio chip select
const int resetPin = 5;       // LoRa radio reset
const int irqPin = 2;         // change for your board; must be a hardware interrupt pin
 
String incoming,outgoing;              // outgoing message
 
byte msgCount = 0;            // count of outgoing messages
byte localAddress = 0xBB;     // address of this device
byte destination = 0xFF;      // destination to send to
long lastSendTime = 0;        // last send time
int interval = 2000;          // interval between sends

static String message;   // send a message
int delimiter, delimiter_1, delimiter_2, delimiter_3;
String path1,path2,path3,Timestamp,CO,CO2,HeartRate;
int a,b,c;
//Firebase_WiFi 
#include <WiFi.h>

#include <FirebaseESP32.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>
#define WIFI_SSID "Dinu"
#define WIFI_PASSWORD "124158019"
#define API_KEY "AIzaSyCa7-5SkW87KTVDFDW_D8shfoa5gbvEv-c"
#define DATABASE_URL "lorasample-ff379-default-rtdb.asia-southeast1.firebasedatabase.app" 
#define USER_EMAIL "dummysignin2@yahoo.com"
#define USER_PASSWORD "password"
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
unsigned long sendDataPrevMillis = 0;
unsigned long count = 0;

void setupWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
}
void setupFirebase() {
  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
  config.api_key = API_KEY;

  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  config.database_url = DATABASE_URL;
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  Firebase.setDoubleDigits(5);
}
void parseMessage() {
  delimiter =incoming.indexOf(";");
  delimiter_1 =incoming.indexOf(";", delimiter + 1);
  delimiter_2 =incoming.indexOf(";", delimiter_1 +1);
  //delimiter_3 =incoming.indexOf(";", delimiter_2 +1);
  delimiter_3 =incoming.length();

  // Define variables to be executed on the code later by collecting information from the readString as substrings.
  Timestamp=incoming.substring( 0 , delimiter);
  //Temp = message.substring(delimiter + 1, delimiter_1);
  CO= incoming.substring(delimiter + 1, delimiter_1);
  CO2= incoming.substring(delimiter_1 + 1, delimiter_2);
  HeartRate=incoming.substring(delimiter_2 +1, delimiter_3 );
  Serial.print("Timestamp : ");
  Serial.println(Timestamp);
  // Serial.print("Temperature : ");
  // Serial.println(Temp);
  Serial.print("CO : ");
  Serial.println(CO);
  Serial.print("CO2 : ");
  Serial.println(CO2);
  Serial.print("HEART RATE : ");
  Serial.println(HeartRate);
}
void  ToFirebase() {
    //Firebase.setInt(fbdo, main, 5);
    int mq2=CO.toInt();
    int mq135=CO2.toInt();
    int hrate=HeartRate.toInt();
    Serial.println("");
    Serial.println(path1);
    Serial.println(path2);
    Serial.println(path3);
    Firebase.setInt(fbdo, path1, mq2 );
    Firebase.setInt(fbdo, path2, mq135 );
    Firebase.setInt(fbdo, path3, hrate );
    delay(500);
    Serial.println("Succesfully sent data to firebase");
    //Get the data 
    Serial.printf("Get MQ135 CO  value from Firebase--  %s\n", Firebase.getInt(fbdo, path1) ? String(fbdo.to<int>()).c_str() : fbdo.errorReason().c_str());
     a=fbdo.to<int>();
     Serial.printf("Get MQ12 CO2 value from Firebase--  %s\n", Firebase.getInt(fbdo, path2) ? String(fbdo.to<int>()).c_str() : fbdo.errorReason().c_str());
     b=fbdo.to<int>();
     Serial.printf("Get Heart Rate value from Firebase--  %s\n", Firebase.getInt(fbdo, path3) ? String(fbdo.to<int>()).c_str() : fbdo.errorReason().c_str());
     c=fbdo.to<int>();
     Serial.println("Value from firebase : ");
     Serial.println("CO from firebase: ");
     Serial.print(a);
     Serial.println("CO2 from firebase: ");
     Serial.print(b);
     Serial.println("Heart Rate from firebase: ");
     Serial.print(c);
    Serial.println("Successfully received the data from firebase");
    Serial.println();
    Serial.println("------------------");
    Serial.println();
  
}
void setup() {
  Serial.begin(115200);                   // initialize serial
  while (!Serial);
  setupWiFi();
  setupFirebase();

  Serial.println("LoRa Duplex");
 
  // override the default CS, reset, and IRQ pins (optional)
  LoRa.setPins(csPin, resetPin, irqPin);// set CS, reset, IRQ pin
 
 while (!LoRa.begin(433E6)) {
    Serial.println("lora not initialized!");
    delay(500);
 }
  Serial.println("LoRa init succeeded.");
}
void loop() {
  // parse for a packet, and call onReceive with the result:
  Serial.println("Trying to receive message ....");
  while( onReceive(LoRa.parsePacket()) ) ;
  Serial.println("Received Message Successful");
  if (millis() - lastSendTime > interval) {
    sendMessage(message);
    
    Serial.println("Sending  : " + message);
    lastSendTime = millis();            // timestamp the message
    interval = random(2000) + 1000;    // 2-3 seconds
  }
}
 
void sendMessage(String outgoing) {
  
  LoRa.beginPacket();                   // start packet
  LoRa.write(destination);              // add destination address
  LoRa.write(localAddress);             // add sender address
  LoRa.write(msgCount);                 // add message ID
  LoRa.write(outgoing.length());        // add payload length

  LoRa.print(outgoing);                 // add payload
  
  LoRa.endPacket();                     // finish packet and send it
  
  msgCount++;                           // increment message ID
}
 
bool onReceive(int packetSize) {
 
  if (packetSize == 0) return true;          // if there's no packet, return
 
  // read packet header bytes:
  int recipient = LoRa.read();          // recipient address
  byte sender = LoRa.read();            // sender address
  byte incomingMsgId = LoRa.read();     // incoming msg ID
  byte incomingLength = LoRa.read();    // incoming msg length
 
  incoming = "";
 
  while (LoRa.available()) {
    incoming += (char)LoRa.read();
  }
  //change it to int and increment it;
  if (incomingLength != incoming.length()) {   // check length for error
    Serial.println("error: message length does not match length");
    return true;                             // skip rest of function
  }
 
  // if the recipient isn't this device or broadcast,
  if (recipient != localAddress && recipient != 0xFF) {
    Serial.println("This message is not for me.");
    return true;                             // skip rest of function
  }
  // int numbermsg = incoming.toInt();
  // numbermsg=numbermsg+1;

  // message=String(numbermsg);
  // if message is for this device, or broadcast, print details:
  //Serial.println("Message Received from: 0x" + String(sender, HEX));
  //Serial.println("Message Sent to: 0x" + String(recipient, HEX));
  // Serial.println("Message ID: " + String(incomingMsgId));
  // Serial.println("Message length: " + String(incomingLength));
  Serial.println(">> Message Received : " + incoming);
  // Serial.println("RSSI: " + String(LoRa.packetRssi()));
  // Serial.println("Snr: " + String(LoRa.packetSnr()));
  Serial.println("------------------------------");
  message="Received your message : "+incoming;
  parseMessage();
  path1="Saran/"+String(Timestamp)+"/EnvironmentStatus/CO" ;
  path2="Saran/"+String(Timestamp)+"/EnvironmentStatus/CO2" ;
  path3="Saran/"+String(Timestamp)+"/HealthStatus/HeartPulseRate" ;
  if (Firebase.ready()) 
  {
  ToFirebase();
  }
  return false;
}