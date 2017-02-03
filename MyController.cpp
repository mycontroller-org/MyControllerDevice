#include "MyController.h"

#ifdef ENABLE_READ_VCC
  //Enable read VCC
  ADC_MODE(ADC_VCC);
#endif

//Global variables
#ifdef MQTT_SSL_ENABLED
  WiFiClientSecure espClient;
#else
  WiFiClient espClient;
#endif
PubSubClient mqttClient(espClient);
uint32_t heartbeat = 0;
uint32_t pong = 0;
char PAYLOAD_NULL[1] = "";
ESP8266WebServer _webServer(80);

FirmwareConfig *_fc = (FirmwareConfig *) malloc(sizeof(FirmwareConfig));
RequestFWBlock fwRequest;

char nodeEui[13] = "node-eui";
uint8_t mDNSstatus = 0x00;
char feedId[6] = FEED_ID;
char _mqttServer[51];
uint16_t _mqttPort = 1883;
char _mqttUser[16];
char _mqttPwd[16];
bool _fwUpdateRunning = false;
long _fwUpdateMillis = 0;
unsigned long _lastMqttLoopRun = millis();
bool _mqttClientInit = false;

char* getNodeEui(){
  return &nodeEui[0];
}

MyController::MyController() {
}

#ifdef ENABLE_DEBUG
  long milliOld = millis();
#endif

void MyController::loop() {
  if(!init_done){
    initialize();
  }
  #ifdef ENABLE_TRACE
    if((millis() - milliOld) >= 5000){
      MC_SERIAL.printf("MC[T]: FreeHeap:%d\n", ESP.getFreeHeap());
      milliOld = millis();
    }    
  #endif
  if (!mqttClient.connected()) {
  #ifdef ENABLE_INFO
    MC_SERIAL.printf("MC[I]: MQTT failed! Retryting to connect...\n");
  #endif
    checkMQTT();
  }else{
    mqttClient.loop();
    _lastMqttLoopRun = millis();
  }
  checkTasks();
}

bool discoverMyControllerServer(){
  
}

bool MyController::initialize() {
  if(init_done){
  #ifdef ENABLE_INFO
    MC_SERIAL.printf("MC[I]: Already initialized!\n");
  #endif
    return false;
  }
  //Initialize serial port
  #ifndef MC_DISABLED_SERIAL
    MC_SERIAL.begin(SERIAL_BAUD_RATE, SERIAL_8N1, SERIAL_FULL, 1);
  #endif
  #ifdef ENABLE_INFO
    MC_SERIAL.printf("\nMC[I]: Booting ESP8266 device...\n");
  #endif
  #ifdef NODE_EUI
    strcpy(nodeEui, NODE_EUI);
  #else
    strcpy(nodeEui, WiFi.hostname().c_str());
  #endif
  updateConfigFromEEPROM();
  init_done = true;
  WiFi.mode(WIFI_STA);
  checkMQTT();
  if(mqttClient.connected()){
    before();
    sendRSSI();
    sendStatistics();
    mcPresentation();
  }
  #ifdef ENABLE_INFO
    MC_SERIAL.printf("MC[I]: Initialization done...\n");
  #endif
}

void mcDelay(long ms){
  long start = 0;
  while(start < ms){
    delay(50);
    checkTasks();
    start += 50;
  }
}

void checkTasks(){
  #ifdef FACTORY_RESET_PIN
    checkFactoryResetPin();
  #endif
  checkFirmwareUpgrade();
}

void checkFactoryResetPin(){
  #ifdef FACTORY_RESET_PIN
    pinMode(FACTORY_RESET_PIN, INPUT_PULLUP);
    bool doReset = false;
    long refMillis = millis();
    while(digitalRead(FACTORY_RESET_PIN) == FACTORY_RESET_PIN_STATE){
      delay(100);
      if((millis()-refMillis) >= FACTORY_RESET_TIME){
        #ifdef ENABLE_INFO
          MC_SERIAL.printf("\nMC[I]: Factory reset pin triggered by user\n");
        #endif
        factoryReset();
      }
    }
  #endif
}

bool isSystemConfigured(){
  return hwReadConfig(EEPROM_INTERNAL_SYSTEM_RESET) == 0x01;
}

void checkFirmwareUpgrade(){
  if(_fwUpdateRunning && (millis() - _fwUpdateMillis) > 1000*60*3){//Timeout 3 minutes
    _fwUpdateRunning = false;
    Update.end();
    #ifdef ENABLE_INFO
      MC_SERIAL.printf("MC[I]: Firmware upgrade timeout...\n");
    #endif
  }
}

void updateConfigFromEEPROM(){
  if(!isSystemConfigured()){
    //Update default settings for firmware config
    _fc->type = 65535;
    _fc->version = 65535;
    _fc->blocks = 65535;
    char arr[33] = "---------md5sum-not-set---------";
    memcpy(_fc->md5sum, arr, sizeof(arr));
    hwWriteConfigBlock((void *)_fc, (void *)EEPROM_INTERNAL_ADDR_FW_CONFIG, sizeof(FirmwareConfig));

    //Update default settings for node EUI
    updateNodeEui(true);
    //Load default settings
    configSetupManager();
    return;
  }
  //Update MQTT mDNS status
  mDNSstatus = hwReadConfig(EEPROM_INTERNAL_ADDR_MQTT_MDNS);
  #ifdef ENABLE_TRACE
    MC_SERIAL.printf("MC[T]: MQTT mDNS status:[%d]\n", mDNSstatus);
  #endif
  //Read node EUI
  updateNodeEui();
  //Read Feed id
  updateFeedId();
}

//Call connect WiFi
void MyController::connectWiFi(){
    char _ssid[33];
    char _pwd[65];
    uint8_t _bssid_eeprom[6];
    hwReadConfigBlock((void *)_ssid, (void *)EEPROM_INTERNAL_ADDR_WIFI_SSID, 33);
    hwReadConfigBlock((void *)_pwd, (void *)EEPROM_INTERNAL_ADDR_WIFI_PASSWORD, 65);
    hwReadConfigBlock((void *)_bssid_eeprom, (void *)EEPROM_INTERNAL_ADDR_WIFI_BSSID, 6);
    uint8_t _bssid_set = hwReadConfig(EEPROM_INTERNAL_ADDR_WIFI_ENABLE_BSSID);
    uint8_t *_bssid = NULL;

    if(_bssid_set == 0x01){
      _bssid = _bssid_eeprom;
    }else{
      int n = WiFi.scanNetworks();
      int quality = 0;
      int index = -1;
      #ifdef ENABLE_DEBUG
        MC_SERIAL.printf("MC[D]: Searching BSSID for ssid:[%s]\n", _ssid);
      #endif
      for (int i = 0; i < n; i++) {
        if(strcmp(WiFi.SSID(i).c_str(), _ssid) == 0){
          #ifdef ENABLE_DEBUG
            MC_SERIAL.printf("MC[D]: Found BSSID[%s] for ssid:[%s], RSSI:[%d -dBm, %d %]\n", WiFi.BSSIDstr(i).c_str(), _ssid, WiFi.RSSI(i), getRSSIasQuality(WiFi.RSSI(i)));
          #endif
          if(quality < getRSSIasQuality(WiFi.RSSI(i))){
            quality = getRSSIasQuality(WiFi.RSSI(i));
            index = i;
          }
        }
      }
      if(index != -1){
        _bssid = WiFi.BSSID(index);
        #ifdef ENABLE_DEBUG
          MC_SERIAL.printf("MC[D]: Selected bssid:[%s]\n", WiFi.BSSIDstr(index).c_str());
        #endif
      }
    }

    #ifdef ENABLE_INFO
      MC_SERIAL.printf("MC[I]: Connecting to WiFi with ");
      MC_SERIAL.printf("ssid[%s]", _ssid);
    #endif
    if(strlen(_pwd) != 0){
      #ifdef ENABLE_INFO
        MC_SERIAL.printf(" and with password...");
      #endif
      WiFi.begin(_ssid, _pwd, 0, _bssid);
    }else{
      #ifdef ENABLE_INFO
        MC_SERIAL.printf("and no password.");
      #endif
      WiFi.begin(_ssid, NULL, 0, _bssid);
    }    
    //wait 5~10 seconds
    long refranceMillis = millis();
    while (WiFi.status() != WL_CONNECTED){
      mcDelay(500);
      #ifdef ENABLE_INFO
        MC_SERIAL.printf(".");
      #endif
      if((millis() - refranceMillis) > 10000){
        break;
      }
    }
    #ifdef ENABLE_INFO
      MC_SERIAL.printf("%s\n", WiFi.isConnected() ? "OK" : "FAILED");
      MC_SERIAL.printf("MC[I]: WiFi BSSID:[%s], RSSI:[%d dBm, %d %], IP:[%s], StatusCode:[%d]\n", WiFi.BSSIDstr().c_str(), WiFi.RSSI(), getRSSIasQuality(WiFi.RSSI()), WiFi.localIP().toString().c_str(), WiFi.status());
    #endif

    if(!WiFi.isConnected()){
      return;
    }
    //If MQTT init done, no need to follow MQTT setup
    if(_mqttClientInit){
      return;
    }
    //Load MQTT server details from EEPROM or from mDNS
    if(mDNSstatus == 0x01){
      #ifdef ENABLE_DEBUG
        MC_SERIAL.printf("MC[D]: mDNS query service enabled with hostname as [%s]\n", getNodeEui());
      #endif
      WiFi.hostname(getNodeEui());
      if (!MDNS.begin(getNodeEui())) {
        #ifdef ENABLE_ERROR
          MC_SERIAL.printf("MC[E]: Error setting up MDNS responder!\n");
        #endif
      }else{
        #ifdef ENABLE_INFO
          MC_SERIAL.printf("MC[I]: Sending mDNS query[_mc_mqtt._tcp]...\n");
        #endif
        int noServices = 0;
        for(uint8_t count = 0; count < 5; count++){
          #ifdef ENABLE_DEBUG
            MC_SERIAL.printf("MC[D]: Executing mDNS query service. Attempt %d of 5.\n", count+1);
          #endif
          noServices = MDNS.queryService("mc_mqtt", "tcp"); // Send out query for MQTT tcp services
          if(noServices > 0){
            break;
          }
          mcDelay(1000);
        }
        #ifdef ENABLE_INFO
          MC_SERIAL.printf("MC[I]: Number of services found: %d\n", noServices);
        #endif
        if (noServices == 0) {
          #ifdef ENABLE_ERROR
            MC_SERIAL.printf("MC[E]: There is no MQTT services found!\n");
          #endif
          reboot();
        }
        #ifdef ENABLE_TRACE
          for (int index = 0; index <noServices ; ++index) {
            // Print details for each service found
            MC_SERIAL.printf("MC[T]: %d: mDNS response(Hostname:[%s], IP:[%s], Port:[%d])\n", index + 1, MDNS.hostname(index).c_str(), MDNS.IP(index).toString().c_str(), MDNS.port(index));
          }
        #endif
        //Taking first mDNS service
        MDNS.IP(0).toString().toCharArray(_mqttServer, 51);
        _mqttPort = MDNS.port(0);
      }
    }else{
      _mqttPort = hwReadConfigInteger(EEPROM_INTERNAL_ADDR_MQTT_PORT);
      hwReadConfigBlock((void *)_mqttServer, (void *)EEPROM_INTERNAL_ADDR_MQTT_SERVER, 51);
    }
    hwReadConfigBlock((void *)_mqttUser, (void *)EEPROM_INTERNAL_ADDR_MQTT_USERNAME, 16);
    hwReadConfigBlock((void *)_mqttPwd, (void *)EEPROM_INTERNAL_ADDR_MQTT_PASSWORD, 16);
     //update MQTT client details
    mqttClient.setServer(_mqttServer, _mqttPort);
    mqttClient.setCallback(mqttMsgReceived);
    #ifdef ENABLE_INFO
      MC_SERIAL.printf("MC[I]: Configuration(NodeEUI:[%s], Mqtt{mDNS-status:[%d], Server:[%s], Port:[%d], FeedId:[%s], User:[%s]})\n", nodeEui, mDNSstatus, _mqttServer, _mqttPort, feedId, _mqttUser);
    #endif
    _mqttClientInit = true;
}

//checkMQTT
void MyController::checkMQTT() {
  bool skip = false;
  // Loop until we're connected
  if (!WiFi.isConnected()) {
    #ifdef ENABLE_INFO
      MC_SERIAL.printf("MC[I]: WIFI not connected, trying connection...\n");
    #endif
    connectWiFi();
  }
  #ifdef ENABLE_DEBUG
    MC_SERIAL.printf("MC[D]: WiFi connection status: %s\n", WiFi.isConnected()? "Connected" : "Not connected");
  #endif
  //If WiFi is in connected state check MQTT
  if(WiFi.isConnected() && !mqttClient.connected()){
    #ifdef ENABLE_DEBUG
      MC_SERIAL.printf("MC[D]: Attempting MQTT connection...\n");
    #endif
    #ifdef ENABLE_INFO
      MC_SERIAL.printf("MC[I]: MQTT settings(Broker:[%s], Port:[%d])\n", _mqttServer, _mqttPort);
    #endif
    // Attempt to connect
    if(strlen(_mqttUser) == 0){
      #ifdef ENABLE_INFO
        MC_SERIAL.printf("MC[I]: MQTT authenticating as anonymous\n");
      #endif
      mqttClient.connect(WiFi.hostname().c_str());
    }else{
      #ifdef ENABLE_INFO
        MC_SERIAL.printf("MC[I]: MQTT authenticating as user:[%s]\n", _mqttUser);
      #endif
      mqttClient.connect(WiFi.hostname().c_str(), _mqttUser, _mqttUser);
    }
    if (mqttClient.connected()) {
      #ifdef ENABLE_INFO
        MC_SERIAL.printf("MC[I]: MQTT connected :)\n");
      #endif
      // Once connected, resubscribe
      char _topic[30];
      snprintf_P(_topic, 30, PSTR("in_%s/%s/#"), feedId, getNodeEui());
      mqttClient.subscribe(_topic);
      #ifdef ENABLE_INFO
        MC_SERIAL.printf("MC[I]: MQTT topic subscribed:[%s]\n", _topic);
      #endif
      skip = true;
    } else {
      #ifdef ENABLE_INFO
        MC_SERIAL.printf("MC[I]: MQTT connection failed, rc=%d\n", mqttClient.state());
      #endif
    }
  }
  if(!skip){
    #ifdef ENABLE_DEBUG
      MC_SERIAL.printf("MC[D]: Try again in 3 seconds\n");
    #endif
    // Wait 3 seconds before retrying
    mcDelay(3000);
  }
  delay(10);
}

//MQTT on _msg received
void mqttMsgReceived(char* topic, byte* payload, unsigned int length) {
  McMessage _msg;
  #ifdef ENABLE_DEBUG
    MC_SERIAL.printf("MC[D]: Message arrived on topic[%s]\n", topic);
  #endif
  protocolParse(_msg, topic, payload, length);
  #ifdef ENABLE_TRACE
    _msg.printOnSerial();
  #endif
  if(_msg.isValid()){
    if(_msg.isAckRequest()){
      send(_msg.setAck(ACK_RESPONSE));
      _msg.ack = NO_ACK; //Reset ack status
    }else if(_msg.isAckResponse()){
      //This ack response message. Just ignore for other than SET, REQ
      if(_msg.isTypeOf(C_SET) || _msg.isTypeOf(C_REQ)){
        receive(_msg); //Handover to callback
      }
      return;
    }
    if(_msg.isTypeOf(C_SET) || _msg.isTypeOf(C_REQ)){
      receive(_msg); //Handover to callback
      return;
    }else if(_msg.isTypeOf(C_INTERNAL)){
      if(_msg.isSubTypeOf(I_HEARTBEAT)){
        _msg.update(BC_SENSOR, C_INTERNAL, I_HEARTBEAT_RESPONSE);
        send(_msg.set(heartbeat++));
      }else if(_msg.isSubTypeOf(I_PING)){
        _msg.update(BC_SENSOR, C_INTERNAL, I_PONG);
        send(_msg.set(pong++));
      }else if(_msg.isSubTypeOf(I_PRESENTATION)){
        mcPresentation();
      }else if(_msg.isSubTypeOf(I_REBOOT)){
        reboot();
      }else if(_msg.isSubTypeOf(I_TIME)){
        receiveTime(_msg.getULong());
      }else if(_msg.isSubTypeOf(I_RSSI)){
        sendRSSI();
      }else if(_msg.isSubTypeOf(I_ID_RESPONSE)){
        //Update new ID and reboot
        _msg.getString(nodeEui);
        updateNodeEui(true);
        reboot();
      }else if(_msg.isSubTypeOf(I_FACTORY_RESET)){
        factoryReset();
      }else if(_msg.isSubTypeOf(I_PROPERTIES)){
        sendStatistics();
      }
    }else if(_msg.isTypeOf(C_STREAM)){
      if(_msg.isSubTypeOf(ST_FIRMWARE_CONFIG_RESPONSE)){
        //Check fwUpdateRunning?
        if(_fwUpdateRunning){
          #ifdef ENABLE_INFO
            MC_SERIAL.printf("MC[I]: FW Upgrade already running...\n");
          #endif
          sendLogMessage("FW Upgrade already running...");
          return;
        }

        //Fix to avoid grabage values on 'md5sum', as we have this one as char array
        _msg.data[sizeof(FirmwareConfig)-1] = 0;
        memcpy(_fc, _msg.data, sizeof(FirmwareConfig));
        //Get internal
        FirmwareConfig *_fc_int = (FirmwareConfig *) malloc(sizeof(FirmwareConfig));
        hwReadConfigBlock((void *)_fc_int, (void *)EEPROM_INTERNAL_ADDR_FW_CONFIG, sizeof(FirmwareConfig));
        #ifdef ENABLE_INFO
          MC_SERIAL.printf("MC[I]: FW Config Internal(Type:%d, Version:%d, Blocks:%d, md5sum:[%s])\n", _fc_int->type, _fc_int->version, _fc_int->blocks, _fc_int->md5sum);
          MC_SERIAL.printf("MC[I]: FW Config Response(Type:%d, Version:%d, Blocks:%d, md5sum:[%s])\n", _fc->type, _fc->version, _fc->blocks, _fc->md5sum);
        #endif
        bool fwUpdateState = _fc_int->type == _fc->type 
          && _fc_int->version == _fc->version 
          && _fc_int->blocks == _fc->blocks
          && strcmp(_fc_int->md5sum, _fc->md5sum) == 0;

        free(_fc_int);
        #ifdef ENABLE_TRACE
          MC_SERIAL.printf("MC[T]: FreeHeap:%d\n", ESP.getFreeHeap());
        #endif
        if(fwUpdateState){
          sendLogMessage("Firmware up to date!");
          #ifdef ENABLE_INFO
            MC_SERIAL.printf("MC[I]: Firmware up to date. nothing to do!\n");
          #endif
          return;
        }

        uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
        uint32_t requiredSpace = (_fc->blocks-1) * (uint32_t) MAX_OTA_PAYLOAD;
        if(maxSketchSpace < requiredSpace){
          char *_logMessage = "";
          snprintf_P(_logMessage, MAX_PAYLOAD, "FW update: There is no enough space available! expected:%d, available:%d", requiredSpace, maxSketchSpace);
          sendLogMessage(_logMessage);
          #ifdef ENABLE_INFO
            MC_SERIAL.printf("MC[I]: %s\n", _logMessage);
          #endif
          return;
        }else if(maxSketchSpace < (requiredSpace + MAX_OTA_PAYLOAD)){
          char *_logMessage = "";
          snprintf_P(_logMessage, MAX_PAYLOAD, "FW update is running on edge! expected:~%d, available:%d\n", (requiredSpace + MAX_OTA_PAYLOAD), maxSketchSpace);
          sendLogMessage(_logMessage);
          #ifdef ENABLE_INFO
            MC_SERIAL.printf("MC[I]: %s\n", _logMessage);
          #endif
          //Will continue
        }
        if(!Update.begin(maxSketchSpace)){//start with max available size
          Update.printError(Serial);
        }else if(!Update.setMD5(_fc->md5sum)){
            #ifdef ENABLE_INFO
              MC_SERIAL.printf("MC[I]: Failed to set md5\n");
            #endif
            return;
        }
        fwRequest.type = _fc->type;
        fwRequest.version = _fc->version;
        fwRequest.block = 0;
        _msg.setSubType(ST_FIRMWARE_REQUEST);
        _msg.set(&fwRequest, sizeof(RequestFWBlock));
        send(_msg);
        #ifdef ENABLE_INFO
          MC_SERIAL.printf("MC[I]: Firmware upgrade started...\n");
        #endif
        _fwUpdateMillis = millis();
        _fwUpdateRunning = true;
      }else if(_msg.isSubTypeOf(ST_FIRMWARE_RESPONSE)){
        _fwUpdateMillis = millis();
        ReplyFWBlock *replyFwBlock = (ReplyFWBlock *)_msg.data;
        #ifdef ENABLE_DEBUG
          MC_SERIAL.printf("MC[D]: FW Response(Type:%d, Version:%d, Block:%d of %d, DataLength:%d), FreeHeap:%d\n",
            replyFwBlock->type, replyFwBlock->version, replyFwBlock->block, _fc->blocks, replyFwBlock->size, ESP.getFreeHeap());
        #endif
        size_t result = Update.write(replyFwBlock->data, replyFwBlock->size);
        if(result != replyFwBlock->size){
          #ifdef ENABLE_INFO
            MC_SERIAL.printf("MC[I]: Update failed...Write result:%d\n", result);
            Update.printError(MC_SERIAL);
          #endif
        }else if(replyFwBlock->block >= (_fc->blocks-1)){
          if(Update.end(true)){ //true to set the size to the current progress
            //Update firmware config to EEPROM
            hwWriteConfigBlock((void *)_fc, (void *)EEPROM_INTERNAL_ADDR_FW_CONFIG, sizeof(FirmwareConfig));
            #ifdef ENABLE_INFO
              MC_SERIAL.printf("MC[I]: Firmware update success...\n");
            #endif
            sendLogMessage("Firmware upgrade success...");
            reboot();
          } else {
            #ifdef ENABLE_INFO
              Update.printError(MC_SERIAL);
            #endif
            _fwUpdateRunning = false;
          }
        }else{
          fwRequest.block++;
          _msg.setSubType(ST_FIRMWARE_REQUEST);
          _msg.set(&fwRequest, sizeof(RequestFWBlock));
          send(_msg);         
        }
      }     
    }
  }
}
void requestTime(){
  McMessage _message;
  _message.update(BC_SENSOR, C_INTERNAL, I_TIME);
  send(_message.set(0));
}

void sendBatteryLevel(float level){
  McMessage _message;
  _message.update(BC_SENSOR, C_INTERNAL, I_BATTERY_LEVEL);
  send(_message.set(level, 3));
}

void sendRSSI(){
  McMessage _message;
  _message.update(BC_SENSOR, C_INTERNAL, I_RSSI);
  char _rssi[21];
  snprintf_P(_rssi, 20, "%d dBm", WiFi.RSSI());
  send(_message.set(_rssi));
}

void sendLogMessage(char *logMessage){
  McMessage _message;
  _message.update(BC_SENSOR, C_INTERNAL, I_LOG_MESSAGE);
  send(_message.set(logMessage));
}

void sendStatistics(){
  McMessage _message;
  _message.update(BC_SENSOR, C_INTERNAL, I_PROPERTIES);
  char _payload[MAX_PAYLOAD];
  //Part 1
  #ifdef ENABLE_READ_VCC
    snprintf_P(_payload, MAX_PAYLOAD, "freeHeap=%d;chipId=%d;flashChipSize=%d;flashChipRealSize=%d;flashChipSpeed=%d;cycleCount=%d;localIP=%s;vcc=%d mV",
      ESP.getFreeHeap(), ESP.getChipId(), ESP.getFlashChipSize(), ESP.getFlashChipRealSize(), ESP.getFlashChipSpeed(), ESP.getCycleCount(), WiFi.localIP().toString().c_str(), ESP.getVcc());
  #else
    snprintf_P(_payload, MAX_PAYLOAD, "freeHeap=%d;chipId=%d;flashChipSize=%d;flashChipRealSize=%d;flashChipSpeed=%d;cycleCount=%d;localIP=%s",
      ESP.getFreeHeap(), ESP.getChipId(), ESP.getFlashChipSize(), ESP.getFlashChipRealSize(), ESP.getFlashChipSpeed(), ESP.getCycleCount(), WiFi.localIP().toString().c_str());
  #endif

  send(_message.set(_payload));
  
  //Part 2
  snprintf_P(_payload, MAX_PAYLOAD, "subnetMask=%s;gatewayIP=%s;macAddress=%s;ssid=%s;hostname=%s;bssid=%s;rssi=%d dBm;rssiAsQuality=%d",
    WiFi.subnetMask().toString().c_str(), WiFi.gatewayIP().toString().c_str(), WiFi.macAddress().c_str(), WiFi.SSID().c_str(), WiFi.hostname().c_str(), WiFi.BSSIDstr().c_str(), WiFi.RSSI(), getRSSIasQuality(WiFi.RSSI()));
  send(_message.set(_payload));
}

bool isMqttConnected(){
  return mqttClient.connected();
}

bool isWifiConnected(){
  return WiFi.isConnected();
}

void factoryReset(){
  sendLogMessage("Factory reset triggered...");
  #ifdef ENABLE_INFO
    MC_SERIAL.printf("MC[I]: Factory reset triggered...\n");
  #endif
  hwWriteConfig(EEPROM_INTERNAL_SYSTEM_RESET, 0x00);
  reboot();
}

void reboot(){
  sendLogMessage("Rebooting...");
  #ifdef ENABLE_INFO
    MC_SERIAL.printf("MC[I]: Rebooting...\n");
  #endif
  ESP.restart();
}

void updateNodeEui(bool isWrite){
  if(isWrite){
    hwWriteConfigBlock((void *)nodeEui, (void *)EEPROM_INTERNAL_ADDR_NODE_EUI, 13);
  }else{
    hwReadConfigBlock((void *)nodeEui, (void *)EEPROM_INTERNAL_ADDR_NODE_EUI, 13);
  }
}

void updateFeedId(bool isWrite){
  if(isWrite){
    hwWriteConfigBlock((void *)feedId, (void *)EEPROM_INTERNAL_ADDR_FEED_ID, 6);
  }else{
   hwReadConfigBlock((void *)feedId, (void *)EEPROM_INTERNAL_ADDR_FEED_ID, 6); 
  }
}


void handleRoot(){
  #ifdef ENABLE_TRACE
    MC_SERIAL.printf("MC[T]: on root page\n");
  #endif
  String page = FPSTR(HTTP_HEAD);
  page.replace("{v}", "Options");
  page += FPSTR(HTTP_SCRIPT);
  page += FPSTR(HTTP_STYLE);
  page += FPSTR(HTTP_HEAD_END);
  page += F("<h1>MYCONTROLLER.ORG</h1>");
  page += "<h3>";
  page += WiFi.hostname();
  page += "</h3><hr>";
  page += FPSTR(HTTP_PORTAL_OPTIONS);
  page += FPSTR(HTTP_END);

  _webServer.send(200, "text/html", page);
}

void handleConfig(){
  #ifdef ENABLE_TRACE
    MC_SERIAL.printf("MC[T]: on configuration page\n");
  #endif
  String page = FPSTR(HTTP_HEAD);
  page.replace("{v}", "Config ESP");
  page += FPSTR(HTTP_SCRIPT);
  page += FPSTR(HTTP_STYLE);
  page += FPSTR(HTTP_HEAD_END);

  int n = WiFi.scanNetworks();
  if (n == 0) {
    #ifdef ENABLE_INFO
      MC_SERIAL.printf("MC[I]: No networks found\n");
    #endif
    page += F("No networks found. Refresh to scan again.");
  }else {
    //display networks in page
    for (int i = 0; i < n; i++) {
      int quality = getRSSIasQuality(WiFi.RSSI(i));

      String item = FPSTR(HTTP_ITEM);
      String rssiQ;
      rssiQ += quality;
      item.replace("{v}", WiFi.SSID(i));
      item.replace("{b}", WiFi.BSSIDstr(i));
      item.replace("{r}", rssiQ);
      if (WiFi.encryptionType(i) != ENC_TYPE_NONE) {
        item.replace("{i}", "l");
      } else {
        item.replace("{i}", "");
      }
      //DEBUG_WM(item);
      page += item;
      delay(0);
      }
    }
    page += "<br/>";
    String httpForm = FPSTR(HTTP_FORM_START);
    httpForm.replace("{svr}", MQTT_BROKER_HOSTNAME);
    httpForm.replace("{port}", String(MQTT_BROKER_PORT));
    page += httpForm;
    page += FPSTR(HTTP_FORM_END);
    page += FPSTR(HTTP_END);
    _webServer.send(200, "text/html", page);
}

void handleSubmit(){
  #ifdef ENABLE_TRACE
    MC_SERIAL.printf("MC[T]: on configuration save page\n");
  #endif
  bool status = true;
  //Save ssid
  String _ssid    = _webServer.arg("s").c_str();
  String _pwd     = _webServer.arg("p").c_str();  
  String _bssid   = _webServer.arg("bs").c_str();

  String _adc     = _webServer.arg("adc").c_str();
  String _bkr     = _webServer.arg("bkr").c_str();
  String _port    = _webServer.arg("port").c_str();
  String _feed    = _webServer.arg("feed").c_str();
  String _user    = _webServer.arg("user").c_str();
  String _bkrPwd  = _webServer.arg("bkrPwd").c_str();

  uint8_t _mdns_status = 0x00;
  if(_adc.length() != 0 && _adc == "on"){
    _mdns_status = 0x01;
  }
  
  #ifdef ENABLE_DEBUG
    MC_SERIAL.printf("MC[D]: User input raw {ssid:[%s], bssid:[%s], mDNS-status:[%s, %d], mqtt-broker:[%s], port:[%s], feed:[%s], user:[%s]}\n",
      _ssid.c_str(), _bssid.c_str(), _adc.c_str(), _mdns_status, _bkr.c_str(), _port.c_str(), _feed.c_str(), _user.c_str());
  #endif
  
  uint8_t _bssid_set = 0x00;
  uint8_t _bssid_eeprom[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  if(_bssid.length() == 17){
    parseBytes(_bssid.c_str(), ':', _bssid_eeprom, 6, 16);
    _bssid_set = 0x01;
  }

  if(_ssid.length() == 0){
    status = false;
  }else if(_bkr.length() == 0 && _mdns_status == 0x01){
    status = false;
  }else if(_feed.length() == 0){
    _feed = "esp";  //If feed lenght is ZERO, take "esp" as default feed
  }else if(_port.length() == 0){
    _port = "1883"; //If port lenght is ZERO, take "1883" as default port
  }

  if(status){   
    //Save things in eeprom 
    
    //Update feedId
    strcpy(feedId, _feed.c_str());
    updateFeedId(true);

    //Update auto dicover control to Enabled or Disabled (mDNS)
    hwWriteConfig(EEPROM_INTERNAL_ADDR_MQTT_MDNS, _mdns_status);

    //Update server/broker
    hwWriteConfigBlock((void *)_bkr.c_str(), (void *)EEPROM_INTERNAL_ADDR_MQTT_SERVER, 51);
    //Update port
    hwWriteConfigInteger(EEPROM_INTERNAL_ADDR_MQTT_PORT, atoi(_port.c_str()));
    //Update user
    hwWriteConfigBlock((void *)_user.c_str(), (void *)EEPROM_INTERNAL_ADDR_MQTT_USERNAME, 16);
    //Update password
    hwWriteConfigBlock((void *)_bkrPwd.c_str(), (void *)EEPROM_INTERNAL_ADDR_MQTT_PASSWORD, 16);
    
    //Update ssid and password
    hwWriteConfigBlock((void *)_ssid.c_str(), (void *)EEPROM_INTERNAL_ADDR_WIFI_SSID, 33);
    hwWriteConfigBlock((void *)_pwd.c_str(), (void *)EEPROM_INTERNAL_ADDR_WIFI_PASSWORD, 65);
    
    //Update bssid
    hwWriteConfig(EEPROM_INTERNAL_ADDR_WIFI_ENABLE_BSSID, _bssid_set);
    hwWriteConfigBlock((void *)&_bssid_eeprom, (void *)EEPROM_INTERNAL_ADDR_WIFI_BSSID, 6);
    
    //Change state to set
    hwWriteConfig(EEPROM_INTERNAL_SYSTEM_RESET, 0x01);    
    
    String page = FPSTR(HTTP_HEAD);
    page.replace("{v}", "Configurations Saved");
    page += FPSTR(HTTP_SCRIPT);
    page += FPSTR(HTTP_STYLE);
    page += FPSTR(HTTP_HEAD_END);
    page += FPSTR(HTTP_SAVED);
    page += FPSTR(HTTP_END);
    _webServer.send(200, "text/html", page);
    #ifdef ENABLE_DEBUG
      MC_SERIAL.printf("MC[D]: Configuration saved...\n");
    #endif
    //do device reboot
    reboot();
  }else{
    #ifdef ENABLE_ERROR
      MC_SERIAL.printf("MC[E]: Failed to save configuration...\n");
    #endif
    _webServer.send(200, "text/html", "Failed! check values and do try..");
  }
  
}

void handleInfo() {
  #ifdef ENABLE_TRACE
    MC_SERIAL.printf("MC[T]: on information page\n");
  #endif
  String page = FPSTR(HTTP_HEAD);
  page.replace("{v}", "Info");
  page += FPSTR(HTTP_SCRIPT);
  page += FPSTR(HTTP_STYLE);
  page += FPSTR(HTTP_HEAD_END);
  page += F("<dl>");
  page += F("<dt>Chip ID</dt><dd>");
  page += ESP.getChipId();
  page += F("</dd>");
  page += F("<dt>Flash Chip ID</dt><dd>");
  page += ESP.getFlashChipId();
  page += F("</dd>");
  page += F("<dt>IDE Flash Size</dt><dd>");
  page += ESP.getFlashChipSize();
  page += F(" bytes</dd>");
  page += F("<dt>Real Flash Size</dt><dd>");
  page += ESP.getFlashChipRealSize();
  page += F(" bytes</dd>");
  page += F("<dt>Soft AP IP</dt><dd>");
  page += WiFi.softAPIP().toString();
  page += F("</dd>");
  page += F("<dt>Soft AP MAC</dt><dd>");
  page += WiFi.softAPmacAddress();
  page += F("</dd>");
  page += F("<dt>Station MAC</dt><dd>");
  page += WiFi.macAddress();
  page += F("</dd>");
  page += F("</dl>");
  page += FPSTR(HTTP_END);

  _webServer.send(200, "text/html", page);
}

//setup manager
void configSetupManager(){
  WiFi.mode(WIFI_AP_STA);
  #ifdef CONFIGURATION_AP_PASSWORD
    WiFi.softAP(WiFi.hostname().c_str(), CONFIGURATION_AP_PASSWORD);
  #else
    WiFi.softAP(WiFi.hostname().c_str());
  #endif
  #ifdef ENABLE_INFO
      MC_SERIAL.printf("MC[I]: Setting up AP to get configuration details\n");
  #endif
  /*
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); 
    #ifdef ENABLE_INFO
      MC_SERIAL.print(".");
    #endif
  }
  * */
  #ifdef ENABLE_INFO
      MC_SERIAL.printf("MC[I]: Access point(AP), SSID:[%s] IP:[%s]\n", WiFi.hostname().c_str(), WiFi.softAPIP().toString().c_str());
  #endif
  _webServer.on("/", handleRoot);
  _webServer.on("/sconfig", handleSubmit);
  _webServer.on("/config", handleConfig);
  _webServer.on("/info", handleInfo);
  _webServer.begin();
  while(1){
    _webServer.handleClient();
  }
}

bool send(McMessage &message){
  if(mqttClient.connected()){
    char _topic[MAX_TOPIC_LENGTH];
    snprintf_P(_topic, MAX_TOPIC_LENGTH, PSTR("out_%s/%s/%s/%s/%s/%d"), feedId, getNodeEui(), message.sensorId, message.type, message.subType, message.ack);
    char *payload = "";
    payload = message.getString(payload);   
    #ifdef ENABLE_DEBUG
      MC_SERIAL.printf("MC[D]: About to publish a topic:[%s], Payload:[%s]\n", _topic, payload);
    #endif
    bool status = mqttClient.publish(_topic, payload);
    if((millis() - _lastMqttLoopRun) > 700){
      _lastMqttLoopRun = millis();
      mqttClient.loop();  //Run this when user sends continues messages 
    }
    return status;
  }else{
    #ifdef ENABLE_INFO
      MC_SERIAL.printf("MC[I]: MQTT client not connected, Failed to send message\n");
    #endif
    return false;
  }
}

void request(char* sensorId, char* subType){
  McMessage message;
  message.update(sensorId, C_REQ, subType);
  message.set(PAYLOAD_NULL);
  send(message);
}

void present(char* subType, char* sensorId, char* _name){
  McMessage message;
  message.update(sensorId, C_PRESENTATION, subType);
  if(_name){
      message.set(_name);
  }else{
      message.set(PAYLOAD_NULL);
  }
  send(message);
}

void sendSketchInfo(char* name, char* version){
  McMessage message;
  message.update(BC_SENSOR, C_INTERNAL, I_SKETCH_NAME);
  message.set(name);
  send(message);
  message.setSubType(I_SKETCH_VERSION);
  message.set(version);
  send(message);
}

void mcPresentation(){
  McMessage message;
  message.update(BC_SENSOR, C_PRESENTATION, S_ARDUINO_NODE);
  message.set(MC_LIB_VERSION);
  send(message);
  //Call user's presentation
  presentation();
}

void parseBytes(const char* str, char sep, byte* bytes, int maxBytes, int base) {
    for (int i = 0; i < maxBytes; i++) {
        bytes[i] = strtoul(str, NULL, base);  // Convert byte
        str = strchr(str, sep);               // Find next separator
        if (str == NULL || *str == '\0') {
            break;                            // No more separators, exit
        }
        str++;                                // Point to next character after separator
    }
}

bool protocolParse(McMessage &message, char* topic, byte* payload, unsigned int length) {
  char *str, *p, *value=NULL;
  uint8_t bvalue[MAX_PAYLOAD];
  uint8_t blen = 0;
  int i = 0;
  uint8_t command = 0;
  uint8_t ack = 0;
  // Extract command data coming on mqtt
  // mygateway1-in/9985114/wl/C_SET/V_TEXT/ACK_NO
  for (str = strtok_r(topic, "/", &p); str && i < 6; str = strtok_r(NULL, "/", &p)) {
  #ifdef ENABLE_TRACE
    MC_SERIAL.printf("MC[T]: i:%d, str:%s\n", i, str);
  #endif
    char topicSubscribe[10];
    snprintf_P(topicSubscribe, 10, "in_%s", feedId);
    switch (i) {
      case 0: // Check is topic subscribe
        if (strcmp(str, topicSubscribe) != 0) {
          // Message not for us or malformed!
          return false;
        }
        break;
      case 1: // node EUI
        if (strcmp(str, BC_NODE) == 0) {
          // Message not for us or malformed!
          return false;
        }
        break;
      case 2: // SnesorId
        message.sensorId = str;
        break;
      case 3: // Message type
        message.type = str;
        break;
      case 4: // Sub type
        message.subType = str;
        // Add payload
        if (message.isTypeOf(C_STREAM)) {
          blen = 0;
          uint8_t val;
          while (*payload) {
            val = protocolH2i(*payload++) << 4;
            val += protocolH2i(*payload++);
            bvalue[blen] = val;
            blen++;
          }
          message.set(bvalue, blen);
        }
        else {
          char* ca;
          ca = (char *)payload;
          ca += length;
          *ca = '\0';
          message.set((const char*)payload);
        }
        break;
      case 5:
        message.ack = atoi(str);
    }
    i++;
  }
  //debug(PSTR("Received %d"), i);
  // Check for invalid input
  if (i < 4)
    return false;
  message.valid = true;
  return true;
}

uint8_t protocolH2i(char c) {
  uint8_t i = 0;
  if (c <= '9')
    i += c - '0';
  else if (c >= 'a')
    i += c - 'a' + 10;
  else
    i += c - 'A' + 10;
  return i;
}

// EEPROM
void saveState(uint16_t pos, uint8_t value) {
  hwWriteConfig(EEPROM_USER_ADDRESS_INDEX+pos, value);
}
uint8_t loadState(uint16_t pos) {
  return hwReadConfig(EEPROM_USER_ADDRESS_INDEX+pos);
}

void hwWriteConfigInteger(uint16_t pos, uint16_t value) {
  int a = value/256;
  int b = value % 256;
  hwWriteConfig(pos, a);
  hwWriteConfig(pos+1, b);
}

uint16_t hwReadConfigInteger(uint16_t pos) {
  int a = hwReadConfig(pos);
  int b = hwReadConfig(pos+1);
  uint16_t value = a*256+b;
  return value;
}

// Hardware configuration
static void hwInitConfigBlock( size_t length = MAX_EEPROM_SIZE /*defined 4096 bytes is the maximum allowed in ESP8266*/ ) {
  static bool initDone = false;
  if (!initDone) {
    EEPROM.begin(length);
    initDone = true;
  }
}

void hwReadConfigBlock(void* buf, void* adr, size_t length) {
  hwInitConfigBlock();
  uint8_t* dst = static_cast<uint8_t*>(buf);
  int offs = reinterpret_cast<int>(adr);
  while (length-- > 0) {
    *dst++ = EEPROM.read(offs++); 
  }
}

void hwWriteConfigBlock(void* buf, void* adr, size_t length) {
  hwInitConfigBlock();
  uint8_t* src = static_cast<uint8_t*>(buf);
  int offs = reinterpret_cast<int>(adr);
  while (length-- > 0) {
    EEPROM.write(offs++, *src++);
  }
  EEPROM.commit();
}

uint8_t hwReadConfig(int adr) {
  uint8_t value;
  hwReadConfigBlock(&value, reinterpret_cast<void*>(adr), 1);
  return value;
}

void hwWriteConfig(int adr, uint8_t value) {
  uint8_t curr = hwReadConfig(adr);
  if (curr != value) {
    hwWriteConfigBlock(&value, reinterpret_cast<void*>(adr), 1);
  }
}

int getRSSIasQuality(int RSSI) {
  int quality = 0;
  if (RSSI <= -100) {
    quality = 0;
  } else if (RSSI >= -50) {
    quality = 100;
  } else {
    quality = 2 * (RSSI + 100);
  }
  return quality;
}
