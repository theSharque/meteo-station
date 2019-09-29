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

RCSwitch mySwitch = RCSwitch();

void setup() {
  int rc = oledInit(OLED_128x64, 0, 0, -1, -1, 400000L); // use standard I2C bus at 400Khz

  if (rc != OLED_NOT_FOUND) {
    oledFill(0x0, 1);
    oledWriteString(0,0,1,(char *)"Loading...", FONT_SMALL, 1, 1);
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

void blinkLed(int count) {
  for (int i = 0; i < count; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(50);
    digitalWrite(LED_BUILTIN, LOW);
    delay(50);
  }

  delay(200);
}
