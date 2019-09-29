#include <ss_oled.h>
#include <RCSwitch.h>

/*
   Main service for the station.

   Check all sensors and store data locally
   Show fresh data on the display
   Upload data by WiFi to the server
*/

#define RECINT 0              // 0 interupt = 2 pin for RC receiver
#define SNDPIN 3              // Pin for RC sender
#define WAIT 2000             // Timeout between requests

int sensor_h[10];
int sensor_t[10];

RCSwitch mySwitch = RCSwitch();

void setup() {
  Serial.println("Startup");
  for (int i = 0; i < 10; i++) {
    sensor_h[i] = 0;
    sensor_t[0] = 0;
  }

  int rc = oledInit(OLED_128x64, 0, 0, -1, -1, 400000L); // use standard I2C bus at 400Khz

  if (rc != OLED_NOT_FOUND) {
    oledFill(0x0, 1);
    oledWriteString(0, 0, 1, (char *)"Loading...", FONT_SMALL, 1, 1);
  } else {
    Serial.println("Monitor not found");
    blinkLed(3);
  }

  // setup radio
  mySwitch.enableTransmit(SNDPIN);
  mySwitch.enableReceive(RECINT);
}

void loop() {
  // Check new data from sensors
  // Read data from sensors
  // Show fresh data on the display
}

void receiveData() {
  Serial.println("Check for data");
  if (mySwitch.available()) {
    Serial.println("Load data...");
    int room = mySwitch.getReceivedValue();
    int h = mySwitch.getReceivedValue();
    int t = mySwitch.getReceivedValue();
    int crc = mySwitch.getReceivedValue();

    if ( crc == room + h + t) {
      Serial.print("Data received: room=");
      Serial.print(room);
      Serial.print(" humiduty=");
      Serial.print(h);
      Serial.print(" temperature=");
      Serial.println(t);

      sensor_t[room] = t;
      sensor_h[room] = h;

      sendApprove(room);
    } else {
      Serial.println("Wrong CRC");
    }
  } else {
    Serial.println("No new data");
  }
}

void sendApprove(int room) {
  Serial.println("Send data to sensor");

  mySwitch.send(room, 4);
  mySwitch.send(WAIT, 4);
  mySwitch.send(WAIT + room, 4);
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
