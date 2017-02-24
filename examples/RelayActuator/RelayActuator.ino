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

#include <MyController.h>

MyController mc;

//Sensors
#define SENSOR_RELAY_1 "relay-1"
#define SENSOR_RELAY_2 "relay-2"

//Relay pins
#define PIN_RELAY_1 2
#define PIN_RELAY_2 4

void before(){
  //Your code before this library inialize
}

void presentation(){
  //Send node name and version information
  sendSketchInfo("Relay node", "1.0.0");

  //Send sensor name and type
  present(S_BINARY, SENSOR_RELAY_1, "Relay 01");
  //request current status
  request(SENSOR_RELAY_1, V_STATUS);

  present(S_BINARY, SENSOR_RELAY_2, "Relay 02");
  //request current status
  request(SENSOR_RELAY_2, V_STATUS);
}

void receiveTime(unsigned long mcTimestamp){
  //Do something with received time
}

void receive(McMessage &message) {
  if(message.isSensorOf(SENSOR_RELAY_1) && message.isSubTypeOf(V_STATUS)){
    digitalWrite(PIN_RELAY_1, message.getUInt());
  }else if(message.isSensorOf(SENSOR_RELAY_2) && message.isSubTypeOf(V_STATUS)){
    digitalWrite(PIN_RELAY_2, message.getUInt());
  }
}


void setup() {
  pinMode(PIN_RELAY_1, OUTPUT);
  pinMode(PIN_RELAY_2, OUTPUT);
  if(!isSystemConfigured()){
    //You can check, does this node/device configured. If no you can display/blink some led from here.
  }
}

void loop() {
  mc.loop();
  //You can add your logic here.
}
