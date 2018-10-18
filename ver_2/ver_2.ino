#include <SPI.h>
#include <Wire.h>
#include <Arduino.h>
#include "Adafruit_BLE.h"
#include <Adafruit_GFX.h>
#include <Adafruit_Sensor.h>

#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"
#include <Adafruit_SSD1306.h>
#include "Adafruit_TSL2591.h"

#include "BluefruitConfig.h"

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2
#define button_B 6
#define uvLed 12

#define FACTORYRESET_ENABLE         0
#define MINIMUM_FIRMWARE_VERSION    "0.6.6"
#define MODE_LED_BEHAVIOUR          "MODE"

Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591);

Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

/**************************************************************************/
/*!
    Helper Function Definitions
*/
/**************************************************************************/

//Sends data over bluetooth connection
void blePrintData(float *readings, Adafruit_BluefruitLE_SPI ble){

    ble.print("AT+BLEUARTTX=");
    ble.print("{");
    ble.print(readings[1]);
    ble.print(", ");
    ble.print(readings[2]);
    ble.print(", ");
    ble.print(readings[3]);
    ble.print(", ");
    ble.print(readings[4]);
    ble.print("}");
    ble.println(" \n");

}

// Configure the gain and integration timing of the TSL 2591
void configureSensor(void){

  //tsl.setGain(TSL2591_GAIN_LOW);    // 1x gain (bright light)
  tsl.setGain(TSL2591_GAIN_MED);      // 25x gain
  //tsl.setGain(TSL2591_GAIN_HIGH);   // 428x gain (low light)

  // Changing the integration time gives you a longer time over which to sense light
  // longer timelines are slower, but are good in very low light situtations!

  // tsl.setTiming(TSL2591_INTEGRATIONTIME_100MS);  // shortest integration time (bright light)
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_200MS);
  tsl.setTiming(TSL2591_INTEGRATIONTIME_300MS);
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_400MS);
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_500MS);
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_600MS);  // longest integration time (dim light)
}

//Displays the splashscreen for the display
void initializeDisplay(void){
  delay(3000);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Enables the built in HV supply of the display
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(22,12);
  display.println("iGEM Team CMUQ");
  display.display();
  delay(2000);
  display.clearDisplay();
  display.display();
  display.setCursor(0,0);
  display.println("Press button to begin");
  display.display();


}


//Prints the sensor readings on the OLED screen.
void printReadings(float *readings){
  display.clearDisplay();
  display.display();
  display.setCursor(0,0);
  float avg = 0;

  display.print("LUX: ");
  for (byte i = 1; i < 5; i++) {
    avg = avg + readings[i];
  }
  avg = avg / 5.0;
  display.print(avg);
  display.display();
}

//Reads sensor data, returns array with the full spectrum, ir , visible and lux readings
void readTsl(float *readings, int readingNo){
  uint32_t lum = tsl.getFullLuminosity();
  uint16_t ir, full;
  ir = lum >> 0x10;
  full = lum & 0xFFFF;

  switch(readingNo){
    case 0:
      readings[0] = tsl.calculateLux(full, ir);
      break;

    default:
      readings[readingNo] = tsl.calculateLux(full, ir) - readings[0];
      if (readings[readingNo] < 0){
        readings[readingNo] = 0;
      }
      break;
  }
}

void takeReading(float *readings){
  display.clearDisplay();
  display.display();
  display.setCursor(0,0);
  display.println("1. Load empty cuvette");
  display.println("2. Press button");
  display.display();

  delay(100);
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
  display.println("3. Insert Sample");
  display.println("4. Press Button");
  display.setCursor(0,0);
  display.display();

  while(digitalRead(button_B)){
    delay(50);
    yield();
  }
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("Processing...");
  display.display();
  delay(650);
  for (int i = 1; i < 5; i++){
    readTsl(&readings[0], i);
    delay(10);
  }

  digitalWrite(uvLed, LOW);
}

/**************************************************************************/
/*!
    VOID SETUP
*/
/**************************************************************************/

void setup(void) {
  Serial.begin(9600);
  pinMode (button_B, INPUT_PULLUP);
  pinMode (uvLed, OUTPUT);
  float readings[] = {0, 0, 0, 0, 0};
  ble.echo(false);
  ble.verbose(false);
  initializeDisplay();
  display.clearDisplay();
  display.display();
  configureSensor();
  /* Initialise the module */
  Serial.print(F("Initialising the Bluefruit LE module: "));
  if ( !ble.begin(VERBOSE_MODE) )
  {
    // error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
    Serial.println("Error");
  }
  Serial.println( F("OK!") );

  if ( FACTORYRESET_ENABLE )
  {
    /* Perform a factory reset to make sure everything is in a known state */
    Serial.println(F("Performing a factory reset: "));
    if ( ! ble.factoryReset() ){
      // error(F("Couldn't factory reset"));
      Serial.println("Error");
    }
  }
  while (! ble.isConnected()) {
      Serial.println("Connecting..");
      delay(500);
  }
}

void loop(){
  if (! digitalRead(button_B)){
    display.clearDisplay();
    display.display();
    float readings[] = {0, 0, 0, 0, 0, 0, 0, 0};
    takeReading(&readings[0]);
    display.clearDisplay();
    printReadings(&readings[0]);
    Serial.println("Here");
    // Serial.println("")
    Serial.println(ble.isConnected());
    if (ble.isConnected()){
      Serial.println("This");
      blePrintData(&readings[0], ble);
    }
    Serial.println("Now Here");
    delay(10);
    yield();
    display.display();
  }
}
