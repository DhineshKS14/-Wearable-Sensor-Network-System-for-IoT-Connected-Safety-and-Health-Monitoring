#include <SPI.h>
//Incuding LoRa library
#include <LoRa.h>

//define the pins used by the transceiver module
#define NSS 4
#define RST 5
#define DI0 2
#define LED 26

String LoRaData;

void setup() {
  //initialize Serial Monitor
  Serial.begin(115200);
  //set pin  26 as output for led
  pinMode(LED, OUTPUT);
  Serial.println("LoRa Sender");

  //setup LoRa transceiver module
  LoRa.setPins(NSS, RST, DI0);
  
  //Select the frequency accordng to your location
  //433E6 for Asia
  //866E6 for Europe
  //915E6 for North America
  while (!LoRa.begin(433E6)) {
    Serial.println(".");
    delay(500);
  }
  // Change sync word (0xF1) to match the receiver LoRa
  // This code ensure that you don't get LoRa messages
  // from other LoRa transceivers
  // ranges from 0-0xFF
  LoRa.setSyncWord(0xF1);
  Serial.println("LoRa Initializing Successful!");
}

void loop() {
  // LoRa data packet size received from LoRa sender
   int packetSize = LoRa.parsePacket();
   // if the packer size is not 0, then execute this if condition
  if (packetSize) {
    // received a packet
    Serial.print("Received packet: ");

    // receiving the data from LoRa sender
    while (LoRa.available()) { 
      LoRaData = LoRa.readString();
    }
    Serial.println(LoRaData);

  }
}
