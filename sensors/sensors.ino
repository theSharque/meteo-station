#include "DHT.h"
#include <RCSwitch.h>

/*
   In this module data takes from DHT sensor and send it to base.
   Than we take a timeout and sleep

   Blinks:
   1 - Read data
   2 - Send data
   3 - Can't read data from DHT
   4 - 
*/

#define ROOM_ID 1             // ID of the room (sensor)

#define DHTPIN 2              // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11         // Type of DHT module
#define RECINT 0              // 0 interupt = 2 pin for RC receiver
#define SNDPIN 3              // Pin for RC sender

DHT dht(DHTPIN, DHTTYPE);
RCSwitch mySwitch = RCSwitch();

int h;
int t;
int wait;

void setup() {
  Serial.begin(9600);
  Serial.print("Sensor start on room");
  Serial.println(ROOM_ID);
  
  mySwitch.enableTransmit(SNDPIN);
  mySwitch.enableReceive(RECINT);
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  if ( dhtRead() ) {
    do {
      delay(200);
      sendData();
    } while (!receiveApprove());
  } else {
    blinkLed(3);
  }
}

bool dhtRead() {
  Serial.println("Start read");
  blinkLed(1);

  dht.begin();
  delay(2000);

  float fh = dht.readHumidity();
  float ft = dht.readTemperature();

  if (isnan(ft) || isnan(fh)) {
    Serial.println("Wrong data");
    h = 0;
    t = 0;

    return false;
  } else {
    h = int(fh * 100);
    t = int(ft * 100);

    Serial.print("Data: h=");
    Serial.print(h);
    Serial.print(" t=");
    Serial.println(t);

    return true;
  }
}

void sendData() {
  Serial.println("Send data to the base");
  blinkLed(2);

  mySwitch.send(ROOM_ID, 4);
  mySwitch.send(h, 4);
  mySwitch.send(t, 4);
  mySwitch.send(h + t, 4);
}

bool receiveApprove() {
  Serial.println("Receive approve");
  for (int i = 0; i < 5; i++ ) {
    if (mySwitch.available()) {
      int room = mySwitch.getReceivedValue();
      wait = mySwitch.getReceivedValue();
      int crc = mySwitch.getReceivedValue();
      if ( crc != room + wait) {
        blinkLed(5);
        Serial.println("Wrong CRC");
        return false;
      }

      Serial.println("Approved");
      return true;
    } else {
      Serial.println("Wait 1 second");
      delay(1000);
    }
  }

  Serial.println("No approve received");
  blinkLed(4);
  return false;
}

void blinkLed(int count) {
  for (int i = 0; i < count; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(50);
    digitalWrite(LED_BUILTIN, LOW);
    delay(50);
  }

  delay(200);
}
