/*
 * Copyright 2015-2017 Jeeva Kandasamy (jkandasa@gmail.com)
 * and other contributors as indicated by the @author tags.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
 
 /**
 * @author Jeeva Kandasamy (jkandasa)
 * @since 1.0.0
 */


#include <OneWire.h>
#include <DallasTemperature.h>
#include <MyController.h>
#include <SimpleTimer.h>

#define RELAY_PIN 0

#define SEN_TEMP "temp_sensor"
#define SEN_SWITCH "switch"


// Data wire is plugged into pin 2 on the Arduino
#define ONE_WIRE_BUS 2

// Setup a oneWire instance to communicate with any OneWire devices 
// (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
 
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature tempSensors(&oneWire);


//MyControllerDevice instance
MyController mc;

// Create a Timer object called "timer"! 
SimpleTimer timer;

void sendTemperature(){
  tempSensors.requestTemperatures(); // Send the command to get temperatures
  McMessage _message;
  _message.update(SEN_TEMP, C_SET, V_TEMPERATURE);
  send(_message.set(tempSensors.getTempCByIndex(0), 3));
}

void setup() {
  //set GPIO 0 as output pin
  pinMode(RELAY_PIN, OUTPUT);
  // Start up the temperature sensors library
  tempSensors.begin();
  timer.setInterval(1000L * 15, sendTemperature); //  Send temperature every 15 seconds once
}

void before(){

}

void presentation(){
  sendSketchInfo("Relay and Temperature sensor", "1.0.0");
  present(S_TEMPERATURE, SEN_TEMP, "Temperature sensor");
  present(S_BINARY, SEN_SWITCH, "Switch");
  request(SEN_TEMP, V_TEMPERATURE);
  request(SEN_SWITCH, V_STATUS);
}

void receive(McMessage &message) {
  if(message.isSubTypeOf(V_STATUS)){
    digitalWrite(RELAY_PIN, message.getBool());
  }
}

void loop() {
  //Always run MyControllerDevice object.loop
  mc.loop();
  timer.run(); // SimpleTimer is working
}
