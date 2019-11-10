#include <ss_oled.h>
#include <RCSwitch.h>

#define C(x) (char*)(x)
/*
   Main service for the station.

   Check all sensors and store data locally
   Show fresh data on the display
   Upload data by WiFi to the server
*/

#define RECINT 0              // 0 interupt = 2 pin for RC receiver
#define SNDPIN 3              // Pin for RC sender
#define WAIT 600              // Timeout between each request
#define SENSORS 6             // Sensors count

int t, h;
int room, crc;

int sensor_h[SENSORS];
int sensor_t[SENSORS];
long timers[SENSORS];
char szOutBuff[17];

RCSwitch mySwitch = RCSwitch();

void setup() {
  Serial.begin(9600);
  Serial.println("Startup");
  for (int i = 0; i < SENSORS; i++) {
    sensor_h[i] = 0;
    sensor_t[i] = 0;
    timers[i] = 0;
  }

  room = t = h = 0;
  int rc = oledInit(OLED_128x64, 0, 0, -1, -1, 400000L); // use standard I2C bus at 400Khz

  if (rc != OLED_NOT_FOUND) {
    oledFill(0x0, 1);
    oledWriteString(0, 0, 0, C("Loading..."), FONT_NORMAL, 0, 1);
  } else {
    Serial.println("Monitor not found");
    blinkLed(3);
  }

  // setup radio
  mySwitch.enableTransmit(SNDPIN);
  mySwitch.enableReceive(RECINT);
  delay(500);
  oledFill(0x0, 1);
  for (int i = 0; i < SENSORS; i++) {
    sprintf(szOutBuff, "%d - no data", i);
    printLed(i, szOutBuff);
  }
}

void loop() {
  // Check new data from sensors
  receiveData();
  showTime();
  delay(10);
}

void showTime() {
  long ltime = getCurrentTime();
  int seconds = ltime % 60;
  int minutes = (ltime / 60) % 60;
  int hours = (ltime / 3600) % 24;

  sprintf(szOutBuff, "%02d:%02d:%02d", hours, minutes, seconds);
  printLed(7, szOutBuff);

  for (int i = 0 ; i < SENSORS; i++) {
    if (ltime - timers[i] > WAIT) {
      sprintf(szOutBuff, "%d - not found", i);
      printLed(i, szOutBuff);
    }
  }
}

void receiveData() {
  while (mySwitch.available()) {
    switch (mySwitch.getReceivedBitlength()) {
      case 15:
        room = mySwitch.getReceivedValue();
        if (room >= SENSORS || room < 0) {
          room = 0;
        }
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

    if ( crc > 0 && crc == room + h + t ) {
      Serial.print("Received data from room:");
      Serial.print(room);
      printLed(room, C("Update...       "));
      crc = 0;

      Serial.print(" Temperatue:");
      Serial.print((float)t / 10);
      Serial.print(" Humiduty:");
      Serial.println((float)h / 10);

      sensor_t[room] = t;
      sensor_h[room] = h;

      delay(150);
      printLed(room, C("Approve...      "));
      sendApprove(room);

      updateData(room);
      room = t = h = 0;
    }

    mySwitch.resetAvailable();
  }
}

void sendApprove(int room) {
  Serial.println("Approve data from sensor");

  mySwitch.send(room, 15);
  int wait = (WAIT / SENSORS) * room - (getCurrentTime() % WAIT);
  if (wait < 0) {
    wait += WAIT;
  }

  timers[room] = getCurrentTime();

  Serial.print("Next request after: ");
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
  return millis() / 1000;
}

void printLed(int room, char *msg) {
  oledWriteString(0, 0, room, msg, FONT_NORMAL, 0, 1);
}

void updateData(int room) {
  printLed(room, C("                "));
  sprintf(szOutBuff, "%d - %02d.%01dC %02d.%01d%%", room,
          sensor_t[room] / 10, sensor_t[room] - (sensor_t[room] / 10 * 10),
          sensor_h[room] / 10, sensor_h[room] - (sensor_h[room] / 10 * 10));
  printLed(room, szOutBuff);
}
