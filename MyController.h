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

#ifndef MY_CONTROLLER_H
#define MY_CONTROLLER_H

#include "McConfig.h"
#include "McMessage.h"
#include <PubSubClient.h>         //https://github.com/knolleary/pubsubclient

#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <ESP8266mDNS.h>          //https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266mDNS
#include <Wire.h>

#include <ESP8266WebServer.h>
#include <ESP8266httpUpdate.h>


#include <EEPROM.h>

#include <SimpleTimer.h>

#define MDNS_ENABLED    0x01
#define MDNS_DISABLED   0x00

bool protocolParse(McMessage &message, char* topic, byte* payload, unsigned int length);
uint8_t protocolH2i(char c);
void receive(McMessage &message) __attribute__((weak));
void mcPresentation();
void presentation() __attribute__((weak));
void before() __attribute__((weak));
void request(char* sensorId, char* subType);
void present(char* subType, char* sensorId, char* name = NULL);
void sendSketchInfo(char* name, char* version);
bool send(McMessage &message);
char* getNodeEui();
void sendRSSI();
void reboot();
void mcDelay(long ms);
void requestTime();
void sendBatteryLevel(float level);
void sendStatistics();
void sendLogMessage(char *logMessage);
void receiveTime(unsigned long) __attribute__((weak));
void updateNodeEui(bool isWrite=false);
void updateFeedId(bool isWrite=false);
void factoryReset();
bool isMqttConnected();
bool isWifiConnected();
void checkFirmwareUpgrade();
void parseBytes(const char* str, char sep, byte* bytes, int maxBytes, int base);
void sendInitialStatus();

void configSetupManager();
void handleInfo();
void handleRoot();
void handleSubmit();
void handleConfig();

int getRSSIasQuality(int RSSI);
bool isSystemConfigured();


// Hardware config
void hwReadConfigBlock(void* buf, void* adr, size_t length);
void hwWriteConfigBlock(void* buf, void* adr, size_t length);
void hwWriteConfig(int adr, uint8_t value);
uint8_t hwReadConfig(int adr);
void hwWriteConfigInteger(uint16_t pos, uint16_t value);
uint16_t hwReadConfigInteger(uint16_t pos);
void mqttMsgReceived(char* topic, byte* payload, unsigned int length) ;

void updateConfigFromEEPROM();
void checkFactoryResetPin();
void checkTasks();

// EEPROM
void saveState(uint16_t pos, uint8_t value);
uint8_t loadState(uint16_t pos);


const char HTTP_HEAD[] PROGMEM            = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/><title>MyController.org - {v}</title>";
const char HTTP_STYLE[] PROGMEM           = "<style>.c{text-align: center;} div,input{padding:5px;font-size:1em;} input{width:95%;} body{text-align: center;font-family:verdana;} button{border:0;border-radius:0.3rem;background-color:#1fa3ec;color:#fff;line-height:2.4rem;font-size:1.2rem;width:100%;} .q{float: right;width: 64px;text-align: right;} .l{background: url(\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAMAAABEpIrGAAAALVBMVEX///8EBwfBwsLw8PAzNjaCg4NTVVUjJiZDRUUUFxdiZGSho6OSk5Pg4eFydHTCjaf3AAAAZElEQVQ4je2NSw7AIAhEBamKn97/uMXEGBvozkWb9C2Zx4xzWykBhFAeYp9gkLyZE0zIMno9n4g19hmdY39scwqVkOXaxph0ZCXQcqxSpgQpONa59wkRDOL93eAXvimwlbPbwwVAegLS1HGfZAAAAABJRU5ErkJggg==\") no-repeat left center;background-size: 1em;} #adc[type=checkbox]:checked + #ad {display: none;}</style>";
const char HTTP_SCRIPT[] PROGMEM          = "<script>function c(l){document.getElementById('s').value=l.innerText||l.textContent;document.getElementById('p').focus();}</script>";
const char HTTP_HEAD_END[] PROGMEM        = "</head><body><div style='text-align:left;display:inline-block;min-width:260px;'>";
const char HTTP_PORTAL_OPTIONS[] PROGMEM  = "<form action=\"/config\" method=\"get\"><button>Configure Device</button></form><br/><form action=\"/info\" method=\"get\"><button>Info</button></form>";
const char HTTP_ITEM[] PROGMEM            = "<div><a href='#p' onclick='c(this)'>{v}</a>&nbsp;<span>[{b}]</span>&nbsp;<span class='q {i}'>{r}%</span></div>";
#ifdef ENABLE_MQTT_MDNS
  const char HTTP_FORM_START[] PROGMEM      = "<form method='get' action='sconfig'><input id='s' name='s' length=32 placeholder='SSID'><br/><input id='p' name='p' length=64 type='password' placeholder='WiFi password'><br/><input id='bs' name='bs' length=20 placeholder='BSSID'><br/><br/><label>Enable auto discover</label><input id='adc' name='adc' type=\"checkbox\" style='zoom:1.4;width:20px;'/><span id='ad'><input id='bkr' name='bkr' length=50 type='text' value='{svr}' placeholder='Server'><br/><input id='port' name='port' length=5 type='text'  value='{port}' placeholder='Port'><br/></span><input id='feed' name='feed' length=5 type='text' placeholder='Feed id'><br/><input id='user' name='user' length=15 type='text' placeholder='Username'><br/><input id='bkrPwd' name='bkrPwd' length=15 type='password' placeholder='Password'>";
#else
  const char HTTP_FORM_START[] PROGMEM      = "<form method='get' action='sconfig'><input id='s' name='s' length=32 placeholder='SSID'><br/><input id='p' name='p' length=64 type='password' placeholder='WiFi password'><br/><input id='bs' name='bs' length=20 placeholder='BSSID'><br/><br/><input id='bkr' name='bkr' length=50 type='text' value='{svr}' placeholder='Server'><br/><input id='port' name='port' length=5 type='text'  value='{port}' placeholder='Port'><br/><input id='feed' name='feed' length=5 type='text' placeholder='Feed id'><br/><input id='user' name='user' length=15 type='text' placeholder='Username'><br/><input id='bkrPwd' name='bkrPwd' length=15 type='password' placeholder='Password'>";
#endif
const char HTTP_FORM_END[] PROGMEM        = "<br/><br/><button type='submit'>Save</button></form>";
const char HTTP_SAVED[] PROGMEM           = "<div>Configurations Saved<br />Device will reboot and start in normal mode.<br />See you on MyController.org :)</div>";
const char HTTP_END[] PROGMEM             = "</div></body></html>";

//Global variables
#ifdef MQTT_SSL_ENABLED
  extern WiFiClientSecure espClient;
#else
  extern WiFiClient espClient;
#endif
extern PubSubClient mqttClient;
extern uint32_t heartbeat;
extern uint32_t pong;
extern char PAYLOAD_NULL[1];
extern char nodeEui[13];
extern uint8_t mDNSstatus;
extern char feedId[6];
extern ESP8266WebServer _webServer;
extern char _mqttServer[51];
extern uint16_t _mqttPort;
extern char _mqttUser[16];
extern char _mqttPwd[16];
extern bool _fwUpdateRunning;
extern long _fwUpdateMillis;
extern SimpleTimer timer;
extern bool initialStatusSent;

// message buffers
extern McMessage _msg;       // Buffer for incoming messages
extern McMessage _msgTmp;    // Buffer for temporary messages (acks and nonces among others)

class MyController {
private:
  void connectWiFi();
  void loadMqttConfig();
public:
  // Constructors
  MyController();
  
  bool initialize();
  void loop();
  void checkMqtt();
  
  bool init_done = false;

} __attribute__((packed));


/**
* @brief FW config structure, stored in eeprom
*/
typedef struct {
  uint16_t type;                //!< Type of config
  uint16_t version;             //!< Version of config
  uint16_t blocks;              //!< Number of blocks
  char md5sum[33];              //!< MD5SUM
} __attribute__((packed)) FirmwareConfig;

/**
* @brief FW block request structure
*/
typedef struct {
  uint16_t type;                //!< Type of config
  uint16_t version;             //!< Version of config
  uint16_t block;               //!< Block index
} __attribute__((packed)) RequestFWBlock;

/**
* @brief  FW block reply structure
*/
typedef struct {
  uint16_t type;                //!< Type of config
  uint16_t version;             //!< Version of config
  uint16_t block;               //!< Block index
  uint16_t size;                //!< Block size
  uint8_t data[MAX_OTA_PAYLOAD];//!< Block data
} __attribute__((packed)) ReplyFWBlock;


extern FirmwareConfig *_fc;
extern RequestFWBlock fwRequest;

#endif
/** @}*/
