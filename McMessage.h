#ifndef MC_MESSAGE_H
#define MC_MESSAGE_H

#include "McConfig.h"

#include <Arduino.h>
#include <string.h>
#include <stdint.h>

//broadcast types
#define BC_NODE                       "NODE_BC"
#define BC_SENSOR                     "SENSOR_BC"
                                      
#define C_PRESENTATION                "C_PRESENTATION"
#define C_SET                         "C_SET"
#define C_REQ                         "C_REQ"
#define C_INTERNAL                    "C_INTERNAL"
#define C_STREAM                      "C_STREAM"
                                      
#define S_DOOR                        "S_DOOR"
#define S_MOTION                      "S_MOTION"
#define S_SMOKE                       "S_SMOKE"
#define S_BINARY                      "S_BINARY"
#define S_LIGHT                       "S_LIGHT"
#define S_DIMMER                      "S_DIMMER"
#define S_COVER                       "S_COVER"
#define S_TEMPERATURE                 "S_TEMPERATURE"
#define S_HUMIDITY                    "S_HUMIDITY" 
#define S_RAIN                        "S_RAIN"
#define S_WEIGHT                      "S_WEIGHT"
#define S_POWER                       "S_POWER"
#define S_HEATER                      "S_HEATER"
#define S_DISTANCE                    "S_DISTANCE"
#define S_LOCK                        "S_LOCK"
#define S_IR                          "S_IR"
#define S_WATER                       "S_WATER"
#define S_CUSTOM                      "S_CUSTOM"
#define S_DUST                        "S_DUST"
#define S_RGB_LIGHT                   "S_RGB"
#define S_INFORMATION                 "S_INFORMATION"
#define S_GAS                         "S_GAS"
#define S_GPS                         "S_GPS"
#define S_ARDUINO_NODE                "S_ARDUINO_NODE"
#define S_TEMP                        "S_TEMP"
#define S_MULTIMETER                  "S_MULTIMETER"
#define S_MEMORY                      "S_MEMORY"
#define S_CPU                         "S_CPU"
#define S_DISK                        "S_DISK"

#define V_TEMP                        "V_TEMP"
#define V_HUM                         "V_HUM"
#define V_STATUS                      "V_STATUS"
#define V_LIGHT                       "V_LIGHT"
#define V_PERCENTAGE                  "V_PERCENTAGE"
#define V_PRESSURE                    "V_PRESSURE"
#define V_FORECAST                    "V_FORCAST"
#define V_RATE                        "V_RAIN"
#define V_RATE                        "V_RATE"
#define V_WEIGHT                      "V_WEIGHT"
#define V_DISTANCE                    "V_DISTANCE"
#define V_IMPEDANCE                   "V_IMPEDANCE"
#define V_ARMED                       "V_ARMED"
#define V_TRIPPED                     "V_TRIPPED"
#define V_WATT                        "V_WATT"
#define V_KWH                         "V_KWH"
#define V_HVAC_FLOW_STATE             "V_HVAC_FLOW_STATE"
#define V_HVAC_SPEED                  "V_HVAC_SPEED"
#define V_VAR1                        "V_VAR1"
#define V_VAR2                        "V_VAR2"
#define V_VAR3                        "V_VAR3"
#define V_VAR4                        "V_VAR4"
#define V_VAR5                        "V_VAR5"
#define V_UP                          "V_UP"
#define V_DOWN                        "V_DOWN"
#define V_STOP                        "V_STOP"
#define V_IR_SEND                     "V_IR_SEND"
#define V_IR_RECEIVE                  "V_IR_RECIVE"
#define V_VOLUME                      "V_VOLUME"
#define V_LEVEL                       "V_LEVEL"
#define V_VOLTAGE                     "V_VOLTAGE"
#define V_CURRENT                     "V_CURRENT"
#define V_RGB                         "V_RGB"
#define V_RGBW                        "V_RGBW"
#define V_ID                          "V_ID"
#define V_UID                         "V_UID"
#define V_SETPOINT_COOL               "V_SETPOINT_COOL"  
#define V_SETPOINT_HEAT               "V_SETPOINT_HEAT"
#define V_HVAC_FLOW_MODE              "V_HVAC_FLOW_MODE"
#define V_TEXT                        "V_TEXT"
#define V_CUSTOM                      "V_CUSTOM"
#define V_POSITION                    "V_POSITION"
#define V_VAR                         "V_VAR"
#define V_VA                          "V_VA"
#define V_POWER_FACTOR                "V_PF"
#define V_USED                        "V_USED"
#define V_FREE                        "V_FREE"
#define V_TOTAL                       "V_TOTAL"
#define V_COUNT                       "V_COUNT"
#define V_RATE                        "V_RATE"


#define I_BATTERY_LEVEL               "I_BATTERY_LEVEL"
#define I_LOG_MESSAGE                 "I_LOG_MESSAGE"
#define I_TIME                        "I_TIME"
#define I_VERSION                     "I_VERSION"
#define I_SKETCH_NAME                 "I_SKETCH_NAME"
#define I_SKETCH_VERSION              "I_SKETCH_VERSION"
#define I_REBOOT                      "I_REBOOT"
#define I_HEARTBEAT                   "I_HEARTBEAT"
#define I_HEARTBEAT_RESPONSE          "I_HEARTBEAT_RESPONSE"
#define I_PRESENTATION                "I_PRESENTATION"
#define I_PING                        "I_PING"
#define I_PONG                        "I_PONG"
#define I_DEBUG                       "I_DEBUG"
#define I_RSSI                        "I_RSSI"
#define I_PROPERTIES                  "I_PROPERTIES"
#define I_ID_RESPONSE                 "I_ID_RESPONSE"
#define I_FACTORY_RESET               "I_FACTORY_RESET"

#define ST_FIRMWARE_CONFIG_REQUEST    "ST_FIRMWARE_CONFIG_REQUEST"
#define ST_FIRMWARE_CONFIG_RESPONSE   "ST_FIRMWARE_CONFIG_RESPONSE"
#define ST_FIRMWARE_REQUEST           "ST_FIRMWARE_REQUEST"
#define ST_FIRMWARE_RESPONSE          "ST_FIRMWARE_RESPONSE"
#define ST_SOUND                      "ST_SOUND"
#define ST_IMAGE                      "ST_IMAGE"


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
  McMessage& update(char *_sensorId, char *_type, char *_subType);
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
