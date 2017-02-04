#ifndef MC_CONFIG_H
#define MC_CONFIG_H


/* User configurable variables */
/* ***************************************************************************** */
// Enable DEBUG in sketch to show debug prints.
#define ENABLE_ERROR
#define ENABLE_INFO
//#define ENABLE_DEBUG
//#define ENABLE_TRACE


// Disable serial port completely
//#define MC_DISABLED_SERIAL

//Default serial port and baud rate
#define MC_SERIAL Serial
#define SERIAL_BAUD_RATE 115200

// Enable vcc pin
#define ENABLE_READ_VCC

// When there is no configurations updated, this device will create AP to update configuration via mobile/computer
// SSID will be serial of the ESP8266 device
// Should be minimum of 8 char and maximum of 63 char
#define CONFIGURATION_AP_PASSWORD "mycontroller"

// Define factory reset pin.
// You can configure any pin for factory reset. When you cannot access your device,
// You reset your device settings and ask to produce AP to update new settings
//#define FACTORY_RESET_PIN 0
#define FACTORY_RESET_PIN_STATE 0
#define FACTORY_RESET_TIME 1000*9  //hold the button 9 seconds

// MQTT settings, You can leave this as default. Until you do not need any change
// Default settings
#define FEED_ID "mc" //Maximum allowed length 5 chars. will be used as topic to publich and subscribe
#define MQTT_BROKER_HOSTNAME "cloud.mycontroller.org" //MyController server ip
#define MQTT_BROKER_PORT 1883
//#define MQTT_SSL_ENABLED

#define MC_LIB_VERSION "1.0.0"
#define MAX_OTA_PAYLOAD 200
#define MAX_PAYLOAD 220 //!< The maximum size of a payload
#define MAX_TOPIC_LENGTH 72

#define MQTT_MAX_PACKET_SIZE 512 //This one is not working have to check it


/* NO NEED TO TOUCH FROM HERE */
/* ***************************************************************************** */

// Turn off debug if serial pins is used for other stuff
#ifdef MC_DISABLED_SERIAL
#undef ENABLE_ERROR
#undef ENABLE_INFO
#undef ENABLE_DEBUG
#undef ENABLE_TRACE
#undef MC_SERIAL
#endif

// If serial baud not defined take it as 115200
#ifndef SERIAL_BAUD_RATE
#define SERIAL_BAUD_RATE 115200
#endif




//System configuration
#define SYSTEM_RESET 0xFF
#define SYSTEM_SET 0x01

//EEPROM
#define MAX_EEPROM_SIZE 2048
#define EEPROM_INTERNAL_ADDR_INDEX 0
#define EEPROM_USER_ADDRESS_INDEX 1024

#define EEPROM_INTERNAL_SYSTEM_RESET EEPROM_INTERNAL_ADDR_INDEX //1 byte
#define EEPROM_INTERNAL_ADDR_FEED_ID (EEPROM_INTERNAL_SYSTEM_RESET + 1) //allow 5 chars, 1 char for terminate string '0'
#define EEPROM_INTERNAL_ADDR_NODE_EUI (EEPROM_INTERNAL_ADDR_FEED_ID + 6) //allow 12 chars
#define EEPROM_INTERNAL_ADDR_MQTT_SERVER (EEPROM_INTERNAL_ADDR_NODE_EUI + 13) //allow 50 chars
#define EEPROM_INTERNAL_ADDR_MQTT_USERNAME (EEPROM_INTERNAL_ADDR_MQTT_SERVER + 51) //allow 15 chars
#define EEPROM_INTERNAL_ADDR_MQTT_PASSWORD (EEPROM_INTERNAL_ADDR_MQTT_USERNAME + 16) //allow 15 chars
#define EEPROM_INTERNAL_ADDR_MQTT_PORT (EEPROM_INTERNAL_ADDR_MQTT_PASSWORD + 16) //2 bytes port range
#define EEPROM_INTERNAL_ADDR_FW_CONFIG (EEPROM_INTERNAL_ADDR_MQTT_PORT + 2) //Store firmware config. 39 bytes
#define EEPROM_INTERNAL_ADDR_WIFI_SSID (EEPROM_INTERNAL_ADDR_FW_CONFIG + 39) //ssid 32 chars
#define EEPROM_INTERNAL_ADDR_WIFI_PASSWORD (EEPROM_INTERNAL_ADDR_WIFI_SSID + 33) //password 64 chars
#define EEPROM_INTERNAL_ADDR_WIFI_ENABLE_BSSID (EEPROM_INTERNAL_ADDR_WIFI_PASSWORD + 65) //1 byte
#define EEPROM_INTERNAL_ADDR_WIFI_BSSID (EEPROM_INTERNAL_ADDR_WIFI_ENABLE_BSSID + 1) //bssid length 6 bytes
#define EEPROM_INTERNAL_ADDR_MQTT_MDNS (EEPROM_INTERNAL_ADDR_WIFI_BSSID + 6) //mdns settings, 1 byte

#endif
