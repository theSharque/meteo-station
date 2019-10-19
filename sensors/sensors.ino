#include <DHT.h>
#include <RCSwitch.h>

/*
   In this module data takes from DHT sensor and send it to base.
   Than we take a timeout and sleep

   Blinks:
   1 - Read data
   2 - Send data
   3 - Can't read data from DHT
   4 - No approve received
*/

#define ROOM_ID 1             // ID of the room (sensor)

#define DHTPIN 4              // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11         // Type of DHT module
#define RECINT 0              // 0 interupt = 2 pin for RC receiver
#define SNDPIN 3              // Pin for RC sender

DHT dht(DHTPIN, DHTTYPE);
RCSwitch mySwitch = RCSwitch();

int h;
int t;
long wait;

void setup() {
  Serial.begin(9600);
  Serial.print("Sensor start on room: ");
  Serial.println(ROOM_ID);

  mySwitch.enableTransmit(SNDPIN);
  mySwitch.enableReceive(RECINT);
  pinMode(LED_BUILTIN, OUTPUT);
  h = t = wait = 0;
}

void loop() {
  if ( dhtRead() ) {
    for (int i = 0; i < 5; i++) {
      sendData();
      delay(200);
      wait = 20;
      if ( receiveApprove() ) {
        break;
      }
    }
  } else {
    blinkLed(3);
  }

  Serial.print("Wait for ");
  Serial.println(wait);
  delay(wait);
}

bool dhtRead() {
  Serial.println("Start read");
  blinkLed(1);

  dht.begin();
  delay(500);

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

  mySwitch.send(ROOM_ID, 15);
  mySwitch.send(h, 16);
  mySwitch.send(t, 17);
  mySwitch.send(ROOM_ID + h + t, 18);
}

bool receiveApprove() {
  int r, c;
  long w;
  r = w = c = 0;

  Serial.println("Receive approve");
  for (int i = 0; i < 10; i++ ) {
    while (mySwitch.available()) {
      switch (mySwitch.getReceivedBitlength()) {
        case 15:
          r = mySwitch.getReceivedValue();
          break;
        case 16:
          w = mySwitch.getReceivedValue();
          break;
        case 17:
          c = mySwitch.getReceivedValue();
          break;
      }

      if ( c == r + w) {
        if (r == ROOM_ID) {
          Serial.println("Approved");
          wait = w * 1000;
          return true;
        }
      }
      mySwitch.resetAvailable();
    }
    delay(100);
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
