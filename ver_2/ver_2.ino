#include <SPI.h>
#include <Wire.h>
#include <Arduino.h>
#include "Adafruit_BLE.h"
#include <Adafruit_Sensor.h>
#include <Adafruit_GFX.h>
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
#define FACTORYRESET_ENABLE         0
#define MINIMUM_FIRMWARE_VERSION    "0.6.6"
#define MODE_LED_BEHAVIOUR          "MODE"

Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591

Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

/**************************************************************************/
/*!
    Helper Function Definitions
*/
/**************************************************************************/

//Sends data over bluetooth connection
void blePrintData(int *readings, Adafruit_BluefruitLE_SPI ble){

  for (i = 0, i < 4, i++)
  ble,print("AT+BLEUARTTX=");
  ble.println(readings[i]);

}

// Configure the gain and integration timing of the TSL 2591
void configureSensor(void){

  //tsl.setGain(TSL2591_GAIN_LOW);    // 1x gain (bright light)
  tsl.setGain(TSL2591_GAIN_MED);      // 25x gain
  //tsl.setGain(TSL2591_GAIN_HIGH);   // 428x gain (low light)

  // Changing the integration time gives you a longer time over which to sense light
  // longer timelines are slower, but are good in very low light situtations!

  //tsl.setTiming(TSL2591_INTEGRATIONTIME_100MS);  // shortest integration time (bright light)
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
  display.setCursor(0,0);
}

//Prints the sensor readings on the OLED screen.
void printReadings(int *readings){
  display.setCursor(0,0);

  display.println("Full|IR|Visible|LUX");
  for (i = 0, i < 4, i++)
  display.print(readings[i]);
  display.print(" ");
}

//Reads sensor data, returns array with the full spectrum, ir , visible and lux readings
  void readTsl(int *readings){
  uint32_t lum = tsl.getFullLuminosity();
  uint16_t ir, full;
  ir = lum >> 16;
  full = lum & 0xFFFF;
  readings[0] = full;
  readings[1] = ir;
  readings[2] = (full - ir);
  readings[3] = tsl.calculateLux(full, ir);
}
/**************************************************************************/
/*!
    VOID SETUP
*/
/**************************************************************************/

void setup(void) {

  pinMode(button_B, INPUT_PULLUP);
  int readings[] = {0, 0, 0, 0};
  initializeDisplay();

  if ( !ble.begin(VERBOSE_MODE) ) {
    display.println("Couldn't find Bluefruit"));
    display.display();
  }

  display.println("OK!");
  display.display();

  if ( FACTORYRESET_ENABLE ) {
    /* Perform a factory reset to make sure everything is in a known state */
    display.println("Performing a factory reset: ");
    display.display();
    if ( ! ble.factoryReset() ){
      display.println("Couldn't factory reset");
      display.display();
    }
  }
  ble.echo(false);
  ble.verbose(false);

  if (tsl.begin()){
    display.println("Connected to sensor.");
    }
  else {
    Serial.println(F("No sensor found..."));
  }

  configureSensor();

  while (! ble.isConnected()){
    yield();
    delay(500);
  }
}

void loop(){
  if (! digitalRead(button_B)){
    
  }
}
