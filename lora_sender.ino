//Incuding arduino default SPI library
#include <SPI.h>
//Incuding LoRa library
#include <LoRa.h>

//define the pins used by the transceiver module
#define NSS 4
#define RST 5
#define DI0 2
//#define BUTTON 25

// //this variable is for update the status of the push button


// this function is called the Interrupt Setvice Routine (ISR).
// When button is pressed, this function will trigger.
// and, update the "button_status" variable to (1 to 0) or (0 to 1)
// void IRAM_ATTR ISR() {
//   Serial.println("interrupt triggered");
//   //every time the interrupt is triggered,
//   // button_status value will inverse (0 to 1) or (1 to 0)
//   button_status = !button_status;
// }

String packet;
float temp;
int co,co2,hrate;
void setup() {
  //initialize Serial Monitor
  Serial.begin(115200);
  //take pin 25 as INPUT with PULL-UP.
  //PULLUP means pin 15 is normally high. When button pressed it gets LOW
//  pinMode(BUTTON, INPUT_PULLUP);
  //Attaching an Interrupt to pin 25 (BUTTON pin) 
  // when button goes (high to low) state, then interrupt will triggered
  //attachInterrupt(BUTTON, ISR, FALLING);
  Serial.println("LoRa Sender");
  //setup LoRa sender
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
  Serial.print("Sending packet: ");
  //If the button_status is 1, Send LoRa packet "on" to receiver LoRa
      temp=random(0,100);
      co=random(100,600);
      co2=random(100,800);
      hrate=random(0,100);
      packet="Temp : "+String(temp)+" *C ; CO :"+String(co)+" PPM; CO2 : "+String(co2)+" PPM ; Heart Rate : "+String(hrate);
      Serial.println("PACKET : ");
      Serial.print(packet);

      LoRa.beginPacket();
      LoRa.print(packet);
      LoRa.endPacket();
      Serial.println("lora sended a message");
      delay(500);
}