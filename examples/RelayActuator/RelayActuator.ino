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
