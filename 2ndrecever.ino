#include <SPI.h>              
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
String path1,path2,path3,path4,Timestamp,CO,CO2,HeartRate;
int a,b,c,d;

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
//Mail 
#include <ESP_Mail_Client.h>
#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465
#define AUTHOR_EMAIL "124158048@sastra.ac.in"
#define AUTHOR_PASSWORD "mklsbiizpryrjjmu"
#define RECIPIENT_EMAIL "esp32alert2023@gmail.com"
String alert;
SMTPSession smtp;
int mq2;
int mq135;
int hrate;
void smtpCallback(SMTP_Status status);

void sendMail()
{   Serial.println();
      
    ESP_Mail_Session session;
    
    session.server.host_name = SMTP_HOST;
    session.server.port = SMTP_PORT;
    session.login.email = AUTHOR_EMAIL;
    session.login.password = AUTHOR_PASSWORD;
    session.login.user_domain = "";
    
    SMTP_Message message;
    message.sender.name = "ESP Central Node";
    message.sender.email = AUTHOR_EMAIL;
    message.subject = "Abnormal Worker Data";
    message.addRecipient("Supervisor", RECIPIENT_EMAIL);
    String front="<div><h1>Alert !</h1><p>";
    String back="</p></div>";
    String htmlMsg = front+alert+back;
    message.html.content = htmlMsg.c_str();
    message.html.content = htmlMsg.c_str();
    message.text.charSet = "us-ascii";
    message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

    if (!smtp.connect(&session))
      return;
  
    if (!MailClient.sendMail(&smtp, &message))
      Serial.println("Error sending Email, " + smtp.errorReason());
}

void smtpCallback(SMTP_Status status){

  Serial.println(status.info());
   if (status.success()){
    Serial.println("----------------");
    ESP_MAIL_PRINTF("Message sent success: %d\n", status.completedCount());
    ESP_MAIL_PRINTF("Message sent failled: %d\n", status.failedCount());
    Serial.println("----------------\n");
    struct tm dt;

    for (size_t i = 0; i < smtp.sendingResult.size(); i++){
      
      SMTP_Result result = smtp.sendingResult.getItem(i);
      time_t ts = (time_t)result.timestamp;
      localtime_r(&ts, &dt);

      ESP_MAIL_PRINTF("Message No: %d\n", i + 1);
      ESP_MAIL_PRINTF("Status: %s\n", result.completed ? "success" : "failed");
      ESP_MAIL_PRINTF("Date/Time: %d/%d/%d %d:%d:%d\n", dt.tm_year + 1900, dt.tm_mon + 1, dt.tm_mday, dt.tm_hour, dt.tm_min, dt.tm_sec);
      ESP_MAIL_PRINTF("Recipient: %s\n", result.recipients.c_str());
      ESP_MAIL_PRINTF("Subject: %s\n", result.subject.c_str());
    }
    Serial.println("----------------\n");
  }
}
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
  
  delimiter_3 =incoming.length();

  Timestamp=incoming.substring( 0 , delimiter);
 
  CO= incoming.substring(delimiter + 1, delimiter_1);
  CO2= incoming.substring(delimiter_1 + 1, delimiter_2);
  HeartRate=incoming.substring(delimiter_2 +1, delimiter_3 );
  Serial.print("Timestamp : ");
  Serial.println(Timestamp);

  Serial.print("CO : ");
  Serial.println(CO);
  Serial.print("CO2 : ");
  Serial.println(CO2);
  Serial.print("HEART RATE : ");
  Serial.println(HeartRate);
}
void  ToFirebase() {
    
    mq2=CO.toInt();
    mq135=CO2.toInt();
    hrate=HeartRate.toInt();
  
    Serial.println(" ");
    Serial.println(path1);
    Serial.println(path2);
    Serial.println(path3);
    Serial.println(path4);

    Firebase.setInt(fbdo, path1, mq2 );
    Firebase.setInt(fbdo, path2, mq135 );
    Firebase.setInt(fbdo, path3, hrate );
    Firebase.set(fbdo, path4,Timestamp);
    delay(500);
    Serial.println("Succesfully sent data to firebase");

    Serial.printf("Get MQ135 CO  value from Firebase--  %s\n", Firebase.getInt(fbdo, path1) ? String(fbdo.to<int>()).c_str() : fbdo.errorReason().c_str());
     a=fbdo.to<int>();
     Serial.printf("Get MQ12 CO2 value from Firebase--  %s\n", Firebase.getInt(fbdo, path2) ? String(fbdo.to<int>()).c_str() : fbdo.errorReason().c_str());
     b=fbdo.to<int>();
     Serial.printf("Get Heart Rate value from Firebase--  %s\n", Firebase.getInt(fbdo, path3) ? String(fbdo.to<int>()).c_str() : fbdo.errorReason().c_str());
     c=fbdo.to<int>();
     Serial.printf("Get TimeStamp from Firebase--  %s\n", Firebase.get(fbdo, path4) ? String(fbdo.to<String>()).c_str() : fbdo.errorReason().c_str());
     d=fbdo.to<int>();
     Serial.println("Value from firebase : ");
     Serial.println("CO from firebase: ");
     Serial.print(a);
     Serial.println("CO2 from firebase: ");
     Serial.print(b);
     Serial.println("Heart Rate from firebase: ");
     Serial.print(c);
     Serial.println(" Timestamp from firebase : ");
     Serial.print(d);
    Serial.println("Successfully received the data from firebase");
    Serial.println();
    Serial.println("------------------");
    Serial.println(); 
}
void setup() {
  Serial.begin(9600);                   // initialize serial
  while (!Serial);
  setupWiFi();
  setupFirebase();
  smtp.debug(1);
  smtp.callback(smtpCallback);
  alert="ESP Started !";
  sendMail();
  Serial.println("LoRa Duplex");
 
  
  LoRa.setPins(csPin, resetPin, irqPin);// set CS, reset, IRQ pin
 
 while (!LoRa.begin(433E6)) {
    Serial.println("lora not initialized!");
    delay(500);
 }
  Serial.println("LoRa init succeeded.");
}
void loop() {
  
  Serial.println("Trying to receive message ....");
  while( onReceive(LoRa.parsePacket()) ) ;
  
  //Environment Threshold
  if(mq135 <650 && mq2<180) {
    Serial.println("Fresh Air: ");
  } 
  else
  {
    Serial.println("Poor Air: ");
    alert="Environment :    MQ135 : "+String(mq135)+"MQ2 : "+String(mq2);
    //send mail
    sendMail(); 
  } 
  if(hrate >=90 && hrate <=200){
    Serial.println("Heart Rate is in Safe Value");
  }
  else{
      Serial.println("heart rate is Abnormal");
      alert="Health :  Heart Rate = "+String(hrate);
      sendMail();
  }
  Serial.println("Received Message Successful");
  if (millis() - lastSendTime > interval) {
    sendMessage(message);
    
    Serial.println("Sending  : " + message);
    lastSendTime = millis();           
    interval = random(2000) + 1000;    
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
  if (incomingLength != incoming.length()) {   
    Serial.println("error: message length does not match length");
    return true;                             
  }
 
  if (recipient != localAddress && recipient != 0xFF) {
    Serial.println("This message is not for me.");
    return true;                          
  }
  
  Serial.println(">> Message Received : " + incoming);
  
  Serial.println("------------------------------");
  message="Received your message : "+incoming;
  parseMessage();

  path1= "Worker_101/EnvironmentStatus/CO" ;
  path2= "Worker_101/EnvironmentStatus/CO2" ;
  path3= "Worker_101/HealthStatus/HeartPulseRate" ;
  path4="Worker_101/TimeStamp";
  if (Firebase.ready()) 
  {
  ToFirebase();
  }
  return false;
}
