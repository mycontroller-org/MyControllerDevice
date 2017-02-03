#include "McMessage.h"
#include <stdio.h>
#include <stdlib.h>


McMessage::McMessage() {
  clear();
}

McMessage::McMessage(char *_sensorId, char *_type, char *_subType) {
  clear();
  setSensorId(_sensorId);
  setType(_type);
  setSubType(_subType);
}

void McMessage::clear() {
  /*
    (void)memset(sensorId, 0u, sizeof(sensorId));
    (void)memset(type, 0u, sizeof(type));
    (void)memset(subType, 0u, sizeof(subType));
    (void)memset(data, 0u, sizeof(data));
    */
    sensorId = NULL;
    type = NULL;
    subType = NULL;
    ack = 0;
    (void)memset(data, 0u, sizeof(data));
}


/* Getters for payload converted to desired form */
void* McMessage::getCustom() const {
  return (void *)data;
}

const char* McMessage::getString() const {
  uint8_t payloadType = miGetPayloadType();
  if (payloadType == P_STRING) {
    return data;
  } else {
    return NULL;
  }
}

// handles single character hex (0 - 15)
char McMessage::i2h(uint8_t i) const {
  uint8_t k = i & 0x0F;
  if (k <= 9)
    return '0' + k;
  else
    return 'A' + k - 10;
}

char* McMessage::getCustomString(char *buffer) const {
  for (uint8_t i = 0; i < miGetLength(); i++)
  {
    buffer[i * 2] = i2h(data[i] >> 4);
    buffer[(i * 2) + 1] = i2h(data[i]);
  }
  buffer[miGetLength() * 2] = '\0';
  return buffer;
}

char* McMessage::getStream(char *buffer) const {
  if (strcmp(type, C_STREAM) == 0 && (buffer != NULL)) {
    return getCustomString(buffer);
  } else {
    return NULL;
  }
}

char* McMessage::getString(char *buffer) const {
  uint8_t payloadType = miGetPayloadType();
  if (buffer != NULL) {
    if (payloadType == P_STRING) {
      strncpy(buffer, data, miGetLength());
      buffer[miGetLength()] = 0;
    } else if (payloadType == P_BYTE) {
      itoa(bValue, buffer, 10);
    } else if (payloadType == P_INT16) {
      itoa(iValue, buffer, 10);
    } else if (payloadType == P_UINT16) {
      utoa(uiValue, buffer, 10);
    } else if (payloadType == P_LONG32) {
      ltoa(lValue, buffer, 10);
    } else if (payloadType == P_ULONG32) {
      ultoa(ulValue, buffer, 10);
    } else if (payloadType == P_FLOAT32) {
      dtostrf(fValue,2,min(fPrecision, 8),buffer);
    } else if (payloadType == P_CUSTOM) {
      return getCustomString(buffer);
    }
    return buffer;
  } else {
    return NULL;
  }
}

bool McMessage::getBool() const {
  return getByte();
}

uint8_t McMessage::getByte() const {
  if (miGetPayloadType() == P_BYTE) {
    return data[0];
  } else if (miGetPayloadType() == P_STRING) {
    return atoi(data);
  } else {
    return 0;
  }
}


float McMessage::getFloat() const {
  if (miGetPayloadType() == P_FLOAT32) {
    return fValue;
  } else if (miGetPayloadType() == P_STRING) {
    return atof(data);
  } else {
    return 0;
  }
}

int32_t McMessage::getLong() const {
  if (miGetPayloadType() == P_LONG32) {
    return lValue;
  } else if (miGetPayloadType() == P_STRING) {
    return atol(data);
  } else {
    return 0;
  }
}

uint32_t McMessage::getULong() const {
  if (miGetPayloadType() == P_ULONG32) {
    return ulValue;
  } else if (miGetPayloadType() == P_STRING) {
    return atol(data);
  } else {
    return 0;
  }
}

int16_t McMessage::getInt() const {
  if (miGetPayloadType() == P_INT16) { 
    return iValue;
  } else if (miGetPayloadType() == P_STRING) {
    return atoi(data);
  } else {
    return 0;
  }
}

uint16_t McMessage::getUInt() const {
  if (miGetPayloadType() == P_UINT16) { 
    return uiValue;
  } else if (miGetPayloadType() == P_STRING) {
    return atoi(data);
  } else {
    return 0;
  }

}

McMessage& McMessage::setType(char *_type) {
  //type = (char *) malloc(strlen(_type));
  //strcpy(type, _type);
  type = _type;
  return *this;
}

McMessage& McMessage::setSensorId(char *_sensorId) {
  //sensorId = (char *) malloc(strlen(_sensorId));
  //strcpy(sensorId, _sensorId);
  sensorId = _sensorId;
  return *this;
}

McMessage& McMessage::setSubType(char *_subType) {
  //subType = (char *) malloc(strlen(_subType));
  //strcpy(subType, _subType);
  subType = _subType;
  return *this;
}

// Set payload
McMessage& McMessage::set(void* value, uint8_t length) {
  uint8_t payloadLength = value == NULL ? 0 : min(length, MAX_PAYLOAD);
  miSetLength(payloadLength); 
  miSetPayloadType(P_CUSTOM);
  memcpy(data, value, payloadLength);
  return *this;
}

McMessage& McMessage::set(const char* value) {
  uint8_t length = value == NULL ? 0 : min(strlen(value), MAX_PAYLOAD);
  miSetLength(length);
  miSetPayloadType(P_STRING);
  if (length) {    
    strncpy(data, value, length);
  }
  // null terminate string
  data[length] = 0;
  return *this;
}

McMessage& McMessage::set(bool value) {
  miSetLength(1);
  miSetPayloadType(P_BYTE);
  data[0] = value;
  return *this;
}

McMessage& McMessage::set(uint8_t value) {
  miSetLength(1);
  miSetPayloadType(P_BYTE);
  data[0] = value;
  return *this;
}

McMessage& McMessage::set(float value, uint8_t decimals) {
  miSetLength(5); // 32 bit float + persi
  miSetPayloadType(P_FLOAT32);
  fValue=value;
  fPrecision = decimals;
  return *this;
}

McMessage& McMessage::set(uint32_t value) {
  miSetPayloadType(P_ULONG32);
  miSetLength(4);
  ulValue = value;
  return *this;
}

McMessage& McMessage::set(int32_t value) {
  miSetPayloadType(P_LONG32);
  miSetLength(4);
  lValue = value;
  return *this;
}

McMessage& McMessage::set(uint16_t value) {
  miSetPayloadType(P_UINT16);
  miSetLength(2);
  uiValue = value;
  return *this;
}

McMessage& McMessage::set(int16_t value) {
  miSetPayloadType(P_INT16);
  miSetLength(2);
  iValue = value;
  return *this;
}

McMessage& McMessage::update(char *_sensorId, char *_type, char *_subType){
  setSensorId(_sensorId);
  setType(_type);
  setSubType(_subType);
}

bool McMessage::isTypeOf(char *_type) {
  return strcmp(type, _type) == 0;
}

bool McMessage::isTypeOf(char *_type, char *_subType) {
  return strcmp(type, _type) == 0 && strcmp(subType, _subType) == 0;
}

bool McMessage::isSubTypeOf(char *_subType) {
  return strcmp(subType, _subType) == 0;
}

bool McMessage::isSensorOf(char *_sensorId) {
  return strcmp(sensorId, _sensorId) == 0;
}

bool McMessage::isTypeOf(String _type) {
  return isTypeOf(_type.c_str());
}

bool McMessage::isTypeOf(String _type, String _subType) {
  return isTypeOf(_type.c_str(), _subType.c_str());
}

bool McMessage::isSubTypeOf(String _subType) {
  return isSubTypeOf(_subType.c_str());
}

bool McMessage::isSensorOf(String _sensorId) {
  return isSensorOf(_sensorId.c_str());
}

bool McMessage::isValid() {
  return valid;
}

McMessage& McMessage::setAck(uint8_t _ack) {
  ack = _ack;
  return *this;
}

bool McMessage::isAckRequest(){
  return ack == ACK_REQUEST;
}

bool McMessage::isAckResponse(){
  return ack == ACK_RESPONSE;
}

void McMessage::printOnSerial() {
  char *_tmpPayload = "";
  MC_SERIAL.printf("MC[T]: McMessage(SensorId:%s, Type:%s, SubType:%s, Payload:{%s})\n", sensorId, type, subType, getString(_tmpPayload));
}
