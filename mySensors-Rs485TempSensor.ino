/**
 * The MySensors Arduino library handles the wireless radio link and protocol
 * between your home built sensors/actuators and HA controller of choice.
 * The sensors forms a self healing radio network with optional repeaters. Each
 * repeater and gateway builds a routing tables in EEPROM which keeps track of the
 * network topology allowing messages to be routed to nodes.
 *
 * Created by Henrik Ekblad <henrik.ekblad@mysensors.org>
 * Copyright (C) 2013-2015 Sensnology AB
 * Full contributor list: https://github.com/mysensors/Arduino/graphs/contributors
 *
 * Documentation: http://www.mysensors.org
 * Support Forum: http://forum.mysensors.org
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 *******************************
 *
 * REVISION HISTORY
 * Version 1.0 - Thomas Kocher
 *
 * DESCRIPTION
 * The sketch reads up to 10 Dallas DS18x20 sensors and transmits the values to the controller via wired RS485 GW.
 */


/**
 * Configure MySensors library
 */
// Enable debug prints to serial monitor
#define MY_DEBUG

/**
 * Configure RS485 transport.
 */
// Enable RS485 transport layer
#define MY_RS485

// Node id defaults to AUTO (tries to fetch id from controller). Specify a number (1-254) if you want to manually set your Node ID
// Must be manuelly set to a unique value on the bus when using RS485 Transport.
#define MY_NODE_ID 47

// Define this to enables DE-pin management on defined pin
#define MY_RS485_DE_PIN 2

// Set RS485 baud rate to use
#define MY_RS485_BAUD_RATE 9600

#define SENSOR_NAME "10Chan Temperature Sensor" // String containing a short Sketch name or NULL if not applicable. Max 25 characters.
#define SENSOR_VERSION "1.0"

/**
 * Include required libraries
 */
#include <MySensors.h>
#include <OneWire.h>
#include <DallasTemperature.h>

/**
 * Global constants
 */
// define pin assignment
#define ONE_WIRE_BUS_PIN A1
#define STATUS_PIN 13
#define STATUS_BLINK_INTERVAL 200


// other constants
#define UPDATE_INTERVAL 60000 // measure temperature every 60secs
#define TEMPERATURE_PRECISION 9
#define MAX_SENSORS 10

/**
 * Global variables
 */
MyMessage message[MAX_SENSORS];

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS_PIN);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

// arrays to hold device addresses
DeviceAddress address[MAX_SENSORS];

// number of temp sensors attached to the one wire bus
int sensorCount = 0;


/**
 * Optional before() method in sketch - for initialisations that needs to take place before radio has been setup (using SPI).
 * HINT: before() is called first, then presentation(), then setup()
 */
void before(void) {
  Serial.print("Initializing "); Serial.print(SENSOR_NAME); Serial.print(" v"); Serial.print(SENSOR_VERSION); Serial.println("...");
  
  pinMode(STATUS_PIN, OUTPUT);
  digitalWrite(STATUS_PIN, LOW);

    // Start up the DallasTemperature library
  sensors.begin();

  // locate devices on the bus
  sensorCount = sensors.getDeviceCount();
  Serial.print("Locating devices...");
  Serial.print("Found "); Serial.print(sensorCount, DEC); Serial.println(" devices.");

  // report parasite power requirements
  Serial.print("Parasite power is: ");
  if (sensors.isParasitePowerMode()) Serial.println("ON");
  else Serial.println("OFF");


  for (int i = 0; i < sensorCount; i++) {
    if (sensors.getAddress(address[i], i)) {
      Serial.print("Device "); Serial.print(i, DEC); Serial.print(" Address: "); printAddress(address[i]); Serial.println();

      // set the resolution to 9 bit per device
      sensors.setResolution(address[i], TEMPERATURE_PRECISION);
      Serial.print("Device "); Serial.print(i, DEC); Serial.print(" Resolution: "); Serial.print(sensors.getResolution(address[i]), DEC); Serial.println();

      message[i].sensor = i;
      message[i].type = V_TEMP;
    } else {
      Serial.print("Unable to find address for Device "); Serial.println(i, DEC);
    }
  }
}


/**
 * Present the sensor to the controller.
 */
void presentation () {
  sendSketchInfo(SENSOR_NAME, SENSOR_VERSION);
  for (int i = 0; i < sensorCount; i++) present(i, S_TEMP);
}


/**
 * function to print a device address
 */
void printAddress(DeviceAddress deviceAddress) {
  for (uint8_t i = 0; i < 8; i++) {
    // zero pad the address if necessary
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}


/**
 * Blink the status led once.
 */
void blinkStatus() {
  digitalWrite(STATUS_PIN, HIGH);
  wait(STATUS_BLINK_INTERVAL);
  digitalWrite(STATUS_PIN, LOW);
}


/**
 * Setup.
 */
void setup(void) {
  // This sensor was initialized in before(). Do nothing.
}


/**
 * Measure temperatures
 */
void loop(void) {
  // call sensors.requestTemperatures() to issue a global temperature
  // request to all devices on the bus
  Serial.print("Requesting temperatures...");
  sensors.requestTemperatures();
  Serial.println("DONE");
  
  for (int i = 0; i < sensorCount; i++) {
    float temp = sensors.getTempC(address[i]);
    Serial.print("Device "); Serial.print(i, DEC); Serial.print(" Address: "); printAddress(address[i]); Serial.print(" Temperature: "); Serial.print(temp); Serial.println("°C");
    blinkStatus();
    send(message[i].set(temp, 3));
  }

  sleep(UPDATE_INTERVAL);
}

