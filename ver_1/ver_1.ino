#include <SPI.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Adafruit_TSL2591.h"

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2
#define button_B 6
#define uvLed 12

#if (SSD1306_LCDHEIGHT != 32)
#error("Height incorrect.");
#endif

Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591);

//Display basic sensor information on the serial monitor
void printSensorDetails(void){
  sensor_t sensor;
  tsl.getSensor(&sensor);
  Serial.println(F("------------------------------------"));
  Serial.print  (F("Sensor:       ")); Serial.println(sensor.name);
  Serial.print  (F("Driver Ver:   ")); Serial.println(sensor.version);
  Serial.print  (F("Unique ID:    ")); Serial.println(sensor.sensor_id);
  Serial.print  (F("Max Value:    ")); Serial.print(sensor.max_value); Serial.println(F(" lux"));
  Serial.print  (F("Min Value:    ")); Serial.print(sensor.min_value); Serial.println(F(" lux"));
  Serial.print  (F("Resolution:   ")); Serial.print(sensor.resolution, 4); Serial.println(F(" lux"));
  Serial.println(F("------------------------------------"));
  Serial.println(F(""));
  delay(500);
}

//Configure gain and integration time for the TSL2591
void configureSensor(void){

  //tsl.setGain(TSL2591_GAIN_LOW);    // 1x gain (bright light)
  tsl.setGain(TSL2591_GAIN_MED);      // 25x gain
  //tsl.setGain(TSL2591_GAIN_HIGH);   // 428x gain

  // Changing the integration time gives you a longer time over which to sense light
  // longer timelines are slower, but are good in very low light situtations!
  //tsl.setTiming(TSL2591_INTEGRATIONTIME_100MS);  // shortest integration time (bright light)
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_200MS);
  //tsl.setTiming(TSL2591_INTEGRATIONTIME_300MS);
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_400MS);
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_500MS);
   tsl.setTiming(TSL2591_INTEGRATIONTIME_600MS);  // longest integration time (dim light)

  Serial.println(F("------------------------------------"));
  Serial.print  (F("Gain:         "));
  tsl2591Gain_t gain = tsl.getGain();
  switch(gain)
  {
    case TSL2591_GAIN_LOW:
      Serial.println(F("1x (Low)"));
      break;
    case TSL2591_GAIN_MED:
      Serial.println(F("25x (Medium)"));
      break;
    case TSL2591_GAIN_HIGH:
      Serial.println(F("428x (High)"));
      break;
    case TSL2591_GAIN_MAX:
      Serial.println(F("9876x (Max)"));
      break;
  }
  Serial.print  (F("Timing:       "));
  Serial.print((tsl.getTiming() + 1) * 100, DEC);
  Serial.println(F(" ms"));
  Serial.println(F("------------------------------------"));
  Serial.println(F(""));
}

void configureDisplay(void){
  delay(3000);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,1);
  display.println("iGEM Team CMUQ");
  display.display();
  delay(2000);
  display.clearDisplay();
  display.display();
  display.setCursor(0,0);
  display.println("Press button to begin");
  display.display();

}

void printReadings(int *readings){
  display.setCursor(0,0);

  display.println("Full|IR|Visible|LUX");
  for (int i = 0; i < 4; i++){
    display.print((readings[i + 4] + readings[i + 8] + readings[i + 12]) / 3);
    display.print(" ");
  }
}

//Reads sensor data, returns array with the full spectrum, ir , visible and lux readings
void readTsl(int *readings, int readingNo){
  uint32_t lum = tsl.getFullLuminosity();
  uint16_t ir, full;
  ir = lum >> 16;
  full = lum & 0xFFFF;
  if (readingNo == 0){
    readings[0] = full;
    readings[1] = ir;
    readings[2] = (full - ir);
    readings[3] = tsl.calculateLux(full, ir);
  }
  else if (readingNo == 1) {
    readings[4] = full - readings[0];
    readings[5] = ir - readings[1];
    readings[6] = (full - ir) - readings[2];
    readings[7] = tsl.calculateLux(full, ir) - readings[3];
  }
  else if (readingNo == 2){
    readings[8] = full - readings[0];
    readings[9] = ir - readings[1];
    readings[10] = (full - ir) - readings[2];
    readings[11] = tsl.calculateLux(full, ir) - readings[3];
  }
  else {
    readings[12] = full - readings[0];
    readings[13] = ir - readings[1];
    readings[14] = (full - ir) - readings[2];
    readings[15] = tsl.calculateLux(full, ir) - readings[3];
  }
}

void takeReading(int *readings){
  display.clearDisplay();
  display.display();
  display.setCursor(0,0);
  display.println("1. Load empty cuvette");
  display.println("2. Press button");
  display.display();

  delay(50);
  while(digitalRead(button_B)){
    delay(50);
  }

  digitalWrite(uvLed, HIGH);
  delay(100);
  readTsl(&readings[0], 0);
  delay(10);
  yield();
  display.clearDisplay();
  display.display();
  display.setCursor(0,0);
  display.println("1. Insert Sample");
  display.println("2. Press button");
  display.setCursor(0,0);
  display.display();

  while(digitalRead(button_B)){
    delay(50);
    yield();
  }
  display.clearDisplay();
  display.setCursor(0,1);
  display.println("Processing...");
  display.display();
  delay(650);
  readTsl(&readings[0], 1);
  delay(10);
  yield();
  readTsl(&readings[0], 2);
  delay(10);
  yield();readTsl(&readings[0], 3);
  delay(10);
  yield();
  digitalWrite(uvLed, LOW);

}


void setup() {
  Serial.println("Running...");
  pinMode (button_B, INPUT_PULLUP);
  pinMode (uvLed, OUTPUT);
  int readings[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  Serial.begin(9600);
  Serial.println(F("Program Started. /n iGEM Team CMUQ 2018. "));

  if (tsl.begin()){
    Serial.println(F("Connected to sensor."));
    }
  else {
    Serial.println(F("No sensor found..."));
    while(1);
  }
  configureDisplay();
  printSensorDetails();
  configureSensor();

}


void loop() {
  if (! digitalRead(button_B)) {
    display.clearDisplay();
    int readings[] = {0 , 0 , 0 , 0};
    takeReading(&readings[0]);
    display.clearDisplay();
    printReadings(&readings[0]);
    delay(10);
    yield();
    display.display();
  }

}
