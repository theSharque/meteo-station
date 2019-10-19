// #include <ss_oled.h>
#include <RCSwitch.h>

/*
   Main service for the station.

   Check all sensors and store data locally
   Show fresh data on the display
   Upload data by WiFi to the server
*/

#define RECINT 0              // 0 interupt = 2 pin for RC receiver
#define SNDPIN 3              // Pin for RC sender
#define WAIT 600              // Timeout between different sensors

int t, h;
int room, crc;

int sensor_h[10];
int sensor_t[10];

RCSwitch mySwitch = RCSwitch();

void setup() {
  Serial.begin(9600);
  Serial.println("Startup");
  for (int i = 0; i < 10; i++) {
    sensor_h[i] = 0;
    sensor_t[0] = 0;
  }

  room = t = h = 0;

  //  int rc = oledInit(OLED_128x64, 0, 0, -1, -1, 400000L); // use standard I2C bus at 400Khz
  //
  //  if (rc != OLED_NOT_FOUND) {
  //    oledFill(0x0, 1);
  //    oledWriteString(0, 0, 1, (char *)"Loading...", FONT_SMALL, 1, 1);
  //  } else {
  //    Serial.println("Monitor not found");
  //    blinkLed(3);
  //  }

  // setup radio
  mySwitch.enableTransmit(SNDPIN);
  mySwitch.enableReceive(RECINT);
}

void loop() {
  // Check new data from sensors
  receiveData();

  // Show fresh data on the display
}

void receiveData() {
  while (mySwitch.available()) {
    switch (mySwitch.getReceivedBitlength()) {
      case 15:
        room = mySwitch.getReceivedValue();
        break;
      case 16:
        h = mySwitch.getReceivedValue();
        break;
      case 17:
        t = mySwitch.getReceivedValue();
        break;
      case 18:
        crc = mySwitch.getReceivedValue();
        break;
    }

    if ( crc == room + h + t ) {
      Serial.print("Received data from room:");
      Serial.print(room);
      crc = 0;

      Serial.print(" Temperatue:");
      Serial.print((float)t / 100);
      Serial.print(" Humiduty:");
      Serial.println((float)h / 100);

      sensor_t[room] = t;
      sensor_h[room] = h;

      delay(200);
      sendApprove(room);

      room = t = h = 0;
    }

    mySwitch.resetAvailable();
  }
}

void sendApprove(int room) {
  Serial.println("Approve data from sensor");

  mySwitch.send(room, 15);
  int wait = WAIT * room - getCurrentTime();
  if(wait < 0) {
    wait += 3600;
  }

  Serial.print("Next request from: ");
  Serial.println(wait);

  mySwitch.send(wait, 16);
  mySwitch.send(wait + room, 17);
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

long getCurrentTime() {
  return (millis() / 1000) % 3600;
}
