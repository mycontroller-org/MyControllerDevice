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

#ifndef MC_MESSAGE_H
#define MC_MESSAGE_H

#include "McConfig.h"

#include <Arduino.h>
#include <string.h>
#include <stdint.h>

//broadcast types
#define BC_NODE                       "NODE_BC"           //node id - broadcast
#define BC_SENSOR                     "SENSOR_BC"         //sensor id - broadcast
  
//Command types 
#define C_INTERNAL                    "C_INTERNAL"        //This is a special internal message. See table below for the details
#define C_PRESENTATION                "C_PRESENTATION"    //Sent by a node when they present attached sensors.
#define C_REQ                         "C_REQ"             //Requests a variable value (usually from an actuator destined for controller).
#define C_SET                         "C_SET"             //This message is sent from or to a sensor when a sensor value should be updated
#define C_STREAM                      "C_STREAM"          //Used for OTA firmware updates
  
//Command [C_PRESENTATION] options  
#define S_AIR_QUALITY                 "S_AIR_QUALITY"     //Air quality
#define S_ARDUINO_NODE                "S_ARDUINO_NODE"    //Arduino node device
#define S_BAROMETER                   "S_BARO"            //Barometer
#define S_BINARY                      "S_BINARY"          //Binary device(on/off).
#define S_COVER                       "S_COVER"           //Window covers or shades
#define S_CPU                         "S_CPU"             //Raspberry PI CPU
#define S_CUSTOM                      "S_CUSTOM"          //Use this for custom sensors where no other fits.
#define S_DIMMER                      "S_DIMMER"          //Dimmable device of some kind
#define S_DISK                        "S_DISK"            //Raspberry PI Disk
#define S_DISTANCE                    "S_DISTANCE"        //Distance sensor
#define S_DOOR                        "S_DOOR"            //Door and window sensors.
#define S_DUST                        "S_DUST"            //Dust level sensor
#define S_GAS                         "S_GAS"             //Gas meter
#define S_GPS                         "S_GPS"             //GPS Sensor
#define S_HEATER                      "S_HEATER"          //Heater device
#define S_HUMIDITY                    "S_HUM"             //Humidity sensor
#define S_INFO                        "S_INFO"            //LCD text device
#define S_IR                          "S_IR"              //IR sender/receiver device
#define S_LIGHT_LEVEL                 "S_LIGHT_LEVEL"     //Light level
#define S_LOCK                        "S_LOCK"            //Lock device
#define S_MEMORY                      "S_MEMORY"          //Raspberry PI device memory
#define S_MOTION                      "S_MOTION"          //Motion sensors.
#define S_MULTIMETER                  "S_MULTIMETER"      //Multimeter device
#define S_POWER                       "S_POWER"           //Power measuring device, like power meters
#define S_PWM                         "S_PWM"             //PWM
#define S_RAIN                        "S_RAIN"            //Rain sensor
#define S_RGBW_LIGHT                  "S_RGBW_LIGHT"      //RGBW light
#define S_RGB_LIGHT                   "S_RGB_LIGHT"       //RGB light
#define S_SMOKE                       "S_SMOKE"           //Smoke sensor.
#define S_SOUND                       "S_SOUND"           //Sound
#define S_SPRINKLER                   "S_SPRINKLER"       //Sprinkler
#define S_TEMPERATURE                 "S_TEMP"            //Temperature sensor
#define S_UV                          "S_UV"              //UV
#define S_WATER                       "S_WATER"           //Water meter, volume
#define S_WATER_QUALITY               "S_WATER_QUALITY"   //Water quality
#define S_WEIGHT                      "S_WEIGHT"          //Weight sensor for scales etc.
#define S_WIND                        "S_WIND"            //Wind
  
//Command [C_SET] and [C_REQ] options 
#define V_ARMED                       "V_ARMED"           //Armed status of a security sensor. 1=Armed, 0=Bypassed
#define V_COUNT                       "V_COUNT"           //Count
#define V_CURRENT                     "V_CURRENT"         //Current level
#define V_CUSTOM                      "V_CUSTOM"          //Custom messages used for controller/inter node specific commands, preferably using S_CUSTOM device type.
#define V_DIRECTION                   "V_DIRECTION"       //Wind direction 0-360 (degrees)
#define V_DISTANCE                    "V_DISTANCE"        //Distance
#define V_DOWN                        "V_DOWN"            //Window covering. Down.
#define V_EC                          "V_EC"              //Water electric conductivity μS/cm (microSiemens/cm)
#define V_FLOW                        "V_FLOW"            //Flow of water (in meter)
#define V_FORECAST                    "V_FORCAST"         //Whether forecast. One of "stable", "sunny", "cloudy", "unstable", "thunderstorm" or "unknown"
#define V_FREE                        "V_FREE"            //Free amount
#define V_GUST                        "V_GUST"            //Gust
#define V_HUMIDITY                    "V_HUM"             //Humidity
#define V_HVAC_FLOW_MODE              "V_HVAC_FLOW_MODE"  //Flow mode for HVAC ("Auto", "ContinuousOn", "PeriodicOn")
#define V_HVAC_FLOW_STATE             "V_HVAC_FLOW_STATE" //Mode of header. One of "Off", "HeatOn", "CoolOn", or "AutoChangeOver"
#define V_HVAC_SETPOINT_COOL          "V_HVAC_SETPOINT_COOL" //HVAC cold setpoint
#define V_HVAC_SETPOINT_HEAT          "V_HVAC_SETPOINT_HEAT" //HVAC/Heater setpoint
#define V_HVAC_SPEED                  "V_HVAC_SPEED"      //HVAC/Heater fan speed ("Min", "Normal", "Max", "Auto")
#define V_ID                          "V_ID"              //Optional unique sensor id (e.g. OneWire DS1820b ids)
#define V_IMPEDANCE                   "V_IMPEDANCE"       //Impedance value
#define V_IR_RECEIVE                  "V_IR_RECIVE"       //This message contains a received IR-command
#define V_IR_RECORD                   "V_IR_RECORD"       //Record IR codes S_IR for playback
#define V_IR_SEND                     "V_IR_SEND"         //Send out an IR-command
#define V_KWH                         "V_KWH"             //Accumulated number of KWH for a power meter
#define V_LEVEL                       "V_LEVEL"           //Used for sending level-value
#define V_LIGHT_LEVEL                 "V_LIGHT_LEVEL"     //Uncalibrated light level. 0-100%. Use V_LEVEL for light level in lux.
#define V_LOCK_STATUS                 "V_LOCK_STATUS"     //Set or get lock status. 1=Locked, 0=Unlocked
#define V_ORP                         "V_ORP"             //Water ORP : redox potential in mV
#define V_PERCENTAGE                  "V_PERCENTAGE"      //Percentage value. 0-100 (%)
#define V_PH                          "V_PH"              //Water PH
#define V_POSITION                    "V_POSITION"        //GPS position and altitude. Payload: latitude;longitude;altitude(m). E.g. "55.722526;13.017972;18"
#define V_POWER_FACTOR                "V_POWER_FACTOR"    //Ratio of real power to apparent power: floating point value in the range [-1,..,1]
#define V_PRESSURE                    "V_PRESSURE"        //Atmospheric Pressure or any other
#define V_RAIN                        "V_RAIN"            //Amount of rain
#define V_RAINRATE                    "V_RAINRATE"        //Rate of rain
#define V_RATE                        "V_RATE"            //other rate
#define V_RGB                         "V_RGB"             //RGB value transmitted as ASCII hex string (I.e "#ff0000" for red)
#define V_RGBW                        "V_RGBW"            //RGBW value transmitted as ASCII hex string (I.e "#ff0000ff" for red + full white)
#define V_SCENE_OFF                   "V_SCENE_OFF"       //Turn OFF a scene
#define V_SCENE_ON                    "V_SCENE_ON"        //Turn ON a scene
#define V_STATUS                      "V_STATUS"          //Binary status. 0=off 1=on
#define V_STOP                        "V_STOP"            //Window covering. Stop.
#define V_TEMPERATURE                 "V_TEMP"            //Temperature
#define V_TEXT                        "V_TEXT"            //Text message to display on LCD or controller device
#define V_TOTAL                       "V_TOTAL"           //Total amount
#define V_TRIPPED                     "V_TRIPPED"         //Tripped status of a security sensor. 1=Tripped, 0=Untripped
#define V_UP                          "V_UP"              //Window covering. Up.
#define V_USED                        "V_USED"            //Used amount
#define V_UV                          "V_UV"              //UV light level
#define V_VA                          "V_VA"              //Apparent power: volt-ampere (VA)
#define V_VAR                         "V_VAR"             //Reactive power: volt-ampere reactive (var)
#define V_VAR1                        "V_VAR1"            //Custom value
#define V_VAR2                        "V_VAR2"            //Custom value
#define V_VAR3                        "V_VAR3"            //Custom value
#define V_VAR4                        "V_VAR4"            //Custom value
#define V_VAR5                        "V_VAR5"            //Custom value
#define V_VOLTAGE                     "V_VOLTAGE"         //Voltage level
#define V_VOLUME                      "V_VOLUME"          //Water volume
#define V_WATT                        "V_WATT"            //Watt value for power meters
#define V_WEIGHT                      "V_WEIGHT"          //Weight (for scales etc)
#define V_WIND                        "V_WIND"            //Windspeed

//Command [C_INTERNAL] options
#define I_BATTERY_LEVEL               "I_BATTERY_LEVEL"       //Use this to report the battery level (in percent 0-100).
#define I_DEBUG                       "I_DEBUG"               //Debug message
#define I_FACTORY_RESET               "I_FACTORY_RESET"       //Do factory reset of node
#define I_HEARTBEAT_REQUEST           "I_HEARTBEAT"           //Heartbeat request
#define I_HEARTBEAT_RESPONSE          "I_HEARTBEAT_RESPONSE"  //Heartbeat response
#define I_ID_RESPONSE                 "I_ID_RESPONSE"         //Id response back to node. Payload contains node id.
#define I_LOG_MESSAGE                 "I_LOG_MESSAGE"         //Sent by the gateway to the Controller to trace-log a message
#define I_PING                        "I_PING"                //Ping sent to node, payload incremental hop counter
#define I_PONG                        "I_PONG"                //In return to ping, sent back to sender, payload incremental hop counter
#define I_PRESENTATION                "I_PRESENTATION"        //Presentation message
#define I_PROPERTIES                  "I_PROPERTIES"          //Properties from node. will be displayed on Node properties of GUI
#define I_REBOOT                      "I_REBOOT"              //Used by OTA firmware updates. Request for node to reboot.
#define I_RSSI                        "I_RSSI"                //RSSI value of node
#define I_SKETCH_NAME                 "I_SKETCH_NAME"         //Optional sketch name that can be used to identify sensor in the Controller GUI
#define I_SKETCH_VERSION              "I_SKETCH_VERSION"      //Optional sketch version that can be reported to keep track of the version of sensor in the Controller GUI.
#define I_TIME                        "I_TIME"                //Sensors can request the current time from the Controller using this message. The time will be reported as the seconds since 1970
#define I_VERSION                     "I_VERSION"             //Used to request gateway version from controller.

//Command [C_STREAM] options
#define ST_FIRMWARE_CONFIG_REQUEST    "ST_FIRMWARE_CONFIG_REQUEST"  //Request new FW, payload contains current FW details
#define ST_FIRMWARE_CONFIG_RESPONSE   "ST_FIRMWARE_CONFIG_RESPONSE" //New FW details to initiate OTA FW update
#define ST_FIRMWARE_REQUEST           "ST_FIRMWARE_REQUEST"         //Request FW block
#define ST_FIRMWARE_RESPONSE          "ST_FIRMWARE_RESPONSE"        //Response FW block
#define ST_IMAGE                      "ST_IMAGE"                    //Image
#define ST_SOUND                      "ST_SOUND"                    //Sound


//ACK related flags
#define NO_ACK 0
#define ACK_REQUEST 1
#define ACK_RESPONSE 2

/// @brief Type of payload
typedef enum {
  P_STRING          = 0,    //!< Payload type is string
  P_BYTE            = 1,    //!< Payload type is byte
  P_INT16           = 2,    //!< Payload type is INT16
  P_UINT16          = 3,    //!< Payload type is UINT16
  P_LONG32          = 4,    //!< Payload type is INT32
  P_ULONG32         = 5,    //!< Payload type is UINT32
  P_CUSTOM          = 6,    //!< Payload type is binary
  P_FLOAT32         = 7     //!< Payload type is float32
} mycontroller_payload;



#ifndef BIT
#define BIT(n)                  ( 1<<(n) ) //!< Bit indexing macro
#endif
#define BIT_MASK(len)           ( BIT(len)-1 ) //!< Create a bitmask of length 'len'
#define BF_MASK(start, len)     ( BIT_MASK(len)<<(start) ) //!< Create a bitfield mask of length starting at bit 'start'

#define BF_PREP(x, start, len)  ( ((x)&BIT_MASK(len)) << (start) ) //!< Prepare a bitmask for insertion or combining
#define BF_GET(y, start, len)   ( ((y)>>(start)) & BIT_MASK(len) ) //!< Extract a bitfield of length 'len' starting at bit 'start' from 'y'
#define BF_SET(y, x, start, len)    ( y= ((y) &~ BF_MASK(start, len)) | BF_PREP(x, start, len) ) //!< Insert a new bitfield value 'x' into 'y'

#define mSetPayloadType(_pt) BF_SET(message_internal, _pt, 0, 3) //!< Set payload type field
#define mGetPayloadType() ((uint8_t)BF_GET(message_internal, 0, 3)) //!< Get payload type field

#define miSetPayloadType(_pt) BF_SET(message_internal, _pt, 0, 3) //!< Internal setter for payload type field
#define miGetPayloadType() (uint8_t)BF_GET(message_internal, 0, 3) //!< Internal getter for payload type field

#define miSetLength(_length) BF_SET(message_internal, _length, 3, 8) //!< Internal setter for length field
#define miGetLength() ((uint16_t)BF_GET(message_internal, 3, 8)) //!< Internal getter for length field

int max (int a, int b);
int min (int a, int b);

class McMessage {
private:
  char* getCustomString(char *buffer) const;

public:
  // Constructors
  McMessage();

  
  McMessage(char *_sensorId, char *_type, char *_subType);
  
  char i2h(uint8_t i) const;

    /**
     * Clear message contents.
     */
    void clear();

  /**
   * If payload is something else than P_STRING you can have the payload value converted
   * into string representation by supplying a buffer with the minimum size of
   * 2*MAX_PAYLOAD+1. This is to be able to fit hex-conversion of a full binary payload.
   */
  char* getStream(char *buffer) const;
  char* getString(char *buffer) const;
  const char* getString() const;
  void* getCustom() const;
  bool getBool() const;
  uint8_t getByte() const;
  float getFloat() const;
  int16_t getInt() const;
  uint16_t getUInt() const;
  int32_t getLong() const;
  uint32_t getULong() const;

  // Setters for building message "on the fly"
  McMessage& setSensorId(char *sensorId);
  McMessage& setType(char *type);
  McMessage& setSubType(char *subType);
  McMessage& update(char *_sensorId, char *_type, char *_subType, int8_t ack=NO_ACK);
  bool isTypeOf(char *type);
  bool isTypeOf(char *type, char *subType);
  bool isSubTypeOf(char *subType);
  bool isSensorOf(char *sensorId);
  bool isTypeOf(String type);
  bool isTypeOf(String type, String subType);
  bool isSubTypeOf(String subType);
  bool isSensorOf(String sensorId);
  bool isValid();
  bool isAckRequest();
  bool isAckResponse();
  void printOnSerial();

  // Setters for payload
  McMessage& set(void* payload, uint8_t length);
  McMessage& set(const char* value);
  McMessage& set(float value, uint8_t decimals);
  McMessage& set(bool value);
  McMessage& set(uint8_t value);
  McMessage& set(uint32_t value);
  McMessage& set(int32_t value);
  McMessage& set(uint16_t value);
  McMessage& set(int16_t value);
  McMessage& setAck(uint8_t ack);

  char* sensorId;
  char* type;
  char* subType;
  uint16_t message_internal;
  uint8_t ack = 0;
  bool valid = false;

  // Each message can transfer a payload. We add one extra byte for string
  // terminator \0 to be "printable" this is not transferred OTA
  // This union is used to simplify the construction of the binary data types transferred.
  union {
    uint8_t bValue;
    uint16_t uiValue;
    int16_t iValue;
    uint32_t ulValue;
    int32_t lValue;
    struct { // Float messages
      float fValue;
      uint8_t fPrecision;   // Number of decimals when serializing
    };
    struct {  // Presentation messages
      uint8_t version;     // Library version
           uint8_t sensorType;   // Sensor type hint for controller, see table above
    };
    char data[MAX_PAYLOAD + 1];
  } __attribute__((packed));
} __attribute__((packed));

#endif
/** @}*/
