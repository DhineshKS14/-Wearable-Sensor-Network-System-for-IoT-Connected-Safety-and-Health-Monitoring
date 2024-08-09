#include <SPI.h>              // include libraries
#include <LoRa.h>
#include <ESP32Time.h>

ESP32Time rtc(3600);  // offset in seconds GMT+1

const int csPin = 4;          // LoRa radio chip select
const int resetPin = 5;       // LoRa radio reset
const int irqPin = 2;         // change for your board; must be a hardware interrupt pin
 
String outgoing;              // outgoing message
 
byte msgCount = 0;            // count of outgoing messages
byte localAddress = 0xBB;     // address of this device
byte destination = 0xFF;      // destination to send to
long lastSendTime = 0;        // last send time
int interval = 5000;          // interval between sends

static String message = "";   // send a message
int MQ135_data;
int MQ2_data;
int heartrate;
void setup() {
  Serial.begin(9600);                   // initialize serial
  while (!Serial);
  digitalWrite(26,0);
  pinMode(26,OUTPUT);
  Serial.println("LoRa Duplex");
 
  // override the default CS, reset, and IRQ pins (optional)
  LoRa.setPins(csPin, resetPin, irqPin);// set CS, reset, IRQ pin
 
 while (!LoRa.begin(433E6)) {
    Serial.println(".");
    delay(500);
 }
  Serial.println("LoRa init succeeded.");
  //set time 
  rtc.setTime(00, 35, 3, 27, 3, 2023); 
}
void loop() {
  MQ135_data = analogRead(34)-3500;//vp co2
  MQ2_data = analogRead(35)-3500;//vn   methane / ethanol
  heartrate=random(80,180);

  message=rtc.getTime("%B %d %Y %H:%M:%S")+";"+String(MQ2_data)+";"+String(MQ135_data)+";"+String(heartrate);
  if (millis() - lastSendTime > interval) {
    sendMessage(message);
    
    Serial.println("Sending  : " + message);
    lastSendTime = millis();            // timestamp the message
    interval = random(2000) + 1000;    // 2-3 seconds
  }
  // BUZZER  
  if(MQ135_data <650 && MQ2_data<180) {
    Serial.println("BUZZER OFF ");
    digitalWrite(26,0);
  } 
  else
  {
    Serial.println("Poor Air: ");
    digitalWrite(26,1);
    delay(5000);
  }
  digitalWrite(26,0);
  // parse for a packet, and call onReceive with the result:
  onReceive(LoRa.parsePacket());
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
 
void onReceive(int packetSize) {
  if (packetSize == 0) return;          // if there's no packet, return
 
  // read packet header bytes:
  int recipient = LoRa.read();          // recipient address
  byte sender = LoRa.read();            // sender address
  byte incomingMsgId = LoRa.read();     // incoming msg ID
  byte incomingLength = LoRa.read();    // incoming msg length
 
  String incoming = "";
 
  while (LoRa.available()) {
    incoming += (char)LoRa.read();
  }
  //change it to int and increment it;
  if (incomingLength != incoming.length()) {   // check length for error
    Serial.println("error: message length does not match length");
    return;                             // skip rest of function
  }
 
  // if the recipient isn't this device or broadcast,
  if (recipient != localAddress && recipient != 0xFF) {
    Serial.println("This message is not for me.");
    return;                             // skip rest of function
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
}