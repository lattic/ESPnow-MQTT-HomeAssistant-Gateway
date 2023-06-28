#ifndef littlefs_configfile_functions_h
#define littlefs_configfile_functions_h

Cipher * cipher = new Cipher();                                                             // that has to be declared here, before littlefs_configfile_functions_h

//declarations
String cr_getFile(fs::FS &fs, const char * path);
void cr_listDir(fs::FS &fs, const char * dirname, uint8_t levels);
void cr_printFile(const char *filename);
void cr_deleteFile(fs::FS &fs, const char * filename);
bool cr_writeFileStr(fs::FS &fs, const char * filename, String content);

void convertToJson(const Config &src, JsonVariant dst);
void convertFromJson(JsonVariantConst src, Config &dst);

bool loadEncConfigFile(const char *filename, Config &config);                               //encrypted
bool loadPlainConfigFile(const char *filename, Config &config);                             //plain

bool saveEncConfigFile(const char *filename, const Config &config, const char* reason);     //encrypted
bool savePlainConfigFile(const char *filename, const Config &config, const char* reason);   //plain

//functions

//reads file and returns as string
String cr_getFile(fs::FS &fs, const char * path) {
  File file = fs.open(path);
  String output = "";
  for(int j = 0; j < file.size(); j++) {
    output += (char)file.read();
  }
  return output;
}


//list directory
void cr_listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("[%s]: Listing directory: %s ",__func__,dirname);
    File root = fs.open(dirname);
    if(!root){
        Serial.printf("- failed to open directory\n");
        return;
    }
    if(!root.isDirectory()){
        Serial.printf(" - not a directory\n");
        return;
    }
    File file = root.openNextFile();
    Serial.printf("\n");
    while(file){
        if(file.isDirectory()){
            Serial.printf("[%s]:   DIR : %s\n",__func__,file.name());

            if(levels){
                cr_listDir(fs, file.name(), levels -1);
            }
        } else {
            Serial.printf("[%s]: FILE: %s, SIZE: %d\n",__func__,file.name(),file.size());
        }
        file = root.openNextFile();
    }
}


//Prints the content of a file to the Serial
void cr_printFile(const char *filename) {
  File file = LittleFS.open(filename, "r");
  if (!file) {
    #ifdef DEBUG
      Serial.printf("[%s]: Failed to open file %s\n",__func__,filename);
    #endif
    return;
  }
  Serial.printf("[%s]: Content of file: %s\n",__func__,filename);
  while (file.available()) {
    Serial.printf("%s",(char)file.read());

  }
  Serial.printf("\n");
}


//delete file
void cr_deleteFile(fs::FS &fs, const char * filename){
    #ifdef DEBUG
      // Serial.printf("Deleting file: %s\r\n", filename);
      Serial.printf("[%s]: Deleting file: %s ",__func__,filename);
    #endif
    if(fs.remove(filename)){
        #ifdef DEBUG
          Serial.printf("- file deleted\n");
        #endif
    } else {
        #ifdef DEBUG
          Serial.printf("- delete failed\n");
        #endif
    }
}


//writes any string to file
bool cr_writeFileStr(fs::FS &fs, const char * filename, String content){
    #ifdef DEBUG
      Serial.printf("[%s]: Writing to file: %s ",__func__,filename);
    #endif
    File file = fs.open(filename, FILE_WRITE);
    if(!file){
        #ifdef DEBUG
          Serial.printf(" - failed to open file for writing\n");
        #endif
        return false;
    }
    if(file.print(content)){
        #ifdef DEBUG
          Serial.printf(" - file written\n");
        #endif
        file.close();
        return true;
    } else {
        #ifdef DEBUG
          Serial.printf(" - write failed\n");
        #endif
        file.close();
        return false;
    }

}

//JSON functions
//converts to Json
void convertToJson(const Config &src, JsonVariant dst) {
  #ifdef DEBUG
    Serial.printf("[%s]: converting data to JSONn",__func__);
  #endif
  dst["c_sleeptime_s"]          = g_sleeptime_s;
  dst["c_bootCount"]            = g_bootCount;
  dst["c_saved_ontime_ms"]      = g_saved_ontime_ms;
  dst["c_wifi_channel"]         = g_wifi_channel;
  dst["c_wifi_ok"]              = g_wifi_ok;
  dst["c_last_gw"]              = g_last_gw;
  dst["c_ssid"]                 = g_wifi_ssid;
  dst["c_password"]             = g_wifi_password;
  dst["c_led_pwm"]              = g_led_pwm;
  dst["c_motion"]               = g_motion;
  dst["c_lux_high_sens"]        = g_lux_high_sens;
  dst["c_last_working_time_ms"] = g_last_working_time_ms;
  dst["c_valid"]                = g_valid;
}


// converts from Json
void convertFromJson(JsonVariantConst src, Config &dst) {
  #ifdef DEBUG
    Serial.printf("[%s]: converting JSON to data\n",__func__);
  #endif
  g_sleeptime_s                 = src["c_sleeptime_s"];
  g_bootCount                   = src["c_bootCount"];
  g_saved_ontime_ms             = src["c_saved_ontime_ms"];
  g_wifi_channel                = src["c_wifi_channel"];
  g_wifi_ok                     = src["c_wifi_ok"];
  g_last_gw                     = src["c_last_gw"];
  g_led_pwm                     = src["c_led_pwm"];
  g_motion                      = src["c_motion"];
  g_lux_high_sens               = src["c_lux_high_sens"];
  g_last_working_time_ms        = src["c_last_working_time_ms"];
  g_valid                       = src["c_valid"];

  strlcpy(g_wifi_ssid, src["c_ssid"], sizeof(g_wifi_ssid));
  strlcpy(g_wifi_password, src["c_password"], sizeof(g_wifi_password));
}


// Load the configuration from a file "filename" to variable "config" - encrypted text version
bool loadEncConfigFile(const char *filename, Config &config) 
{
  DynamicJsonDocument doc(JSON_CONFIG_FILE_SIZE);
  DeserializationError err = deserializeJson(doc, cipher->decryptString(cr_getFile(LittleFS, filename)).c_str());
  if (err) 
  {
    Serial.printf("[%s]: Failed to deserialize encrypted configuration: %s\n",__func__,err.f_str());
    return false;
  } 
  #ifdef DEBUG
    Serial.printf("[%s]: Loaded encrypted config file %s\n",__func__,filename);
  #endif

  // Extract config from the JSON document
  config = doc.as<Config>();
  #ifdef DEBUG
    Serial.printf("[%s]: loaded data to variable=\"config\" from file: %s \n",__func__,filename);
    char doc_json[JSON_CONFIG_FILE_SIZE];
    int size_pl = serializeJson(doc, doc_json);
    Serial.printf("[%s]: ============ DEBUG: CONFIG FILE LOADED ============\n",__func__);
    Serial.printf("[%s]: Size of config file: %s=%d bytes \n",__func__,filename,size_pl);
    serializeJsonPretty(doc, Serial);
    Serial.printf("\n[%s]: ============ DEBUG: CONFIG FILE LOADED END ============\n",__func__);
  #endif
  return true;
}

// Load the configuration from a file "filename" to variable "config" - plain text version
bool loadPlainConfigFile(const char *filename, Config &config) 
{
  File file = LittleFS.open(filename, "r");
  if (!file) {
    Serial.printf("[%s]: Failed to open config file %s\n",__func__,filename);
    return false;
  } 
  #ifdef DEBUG
    Serial.printf("[%s]: Loaded config file %s\n",__func__,filename);
  #endif
  DynamicJsonDocument doc(JSON_CONFIG_FILE_SIZE);
  DeserializationError err = deserializeJson(doc, file);
  if (err) 
  {
    Serial.printf("[%s]: Failed to deserialize configuration: %s\n",__func__,err.f_str());
    return false;
  }
  // Extract config from the JSON document
  config = doc.as<Config>();
  #ifdef DEBUG
    Serial.printf("[%s]: loaded data to variable=\"config\" from file: %s \n",__func__,filename);
    char doc_json[JSON_CONFIG_FILE_SIZE];
    int size_pl = serializeJson(doc, doc_json);
    Serial.printf("[%s]: ============ DEBUG: CONFIG FILE LOADED ============\n",__func__);
    Serial.printf("[%s]: Size of config file: %s=%d bytes \n",__func__,filename,size_pl);
    serializeJsonPretty(doc, Serial);
    Serial.printf("\n[%s]: ============ DEBUG: CONFIG FILE LOADED END ============\n",__func__);
  #endif  

  return true;
}

// Save configuration to a file from variable "config" to file "filename" - encrypted text version
bool saveEncConfigFile(const char *filename, const Config &config, const char* reason) 
{
  DynamicJsonDocument doc(JSON_CONFIG_FILE_SIZE);
  doc.set(config);
  String serialized_to_string;
  bool success = serializeJson(doc, serialized_to_string);
  if (!success) 
  {
    Serial.printf("[%s]: Failed to serialize configuration to string\n",__func__);
    return false;
  } 

  String text = cipher->encryptString(serialized_to_string);
  if (cr_writeFileStr(LittleFS, filename, text))
  {
    #ifdef DEBUG
      Serial.printf("[%s]: ============ DEBUG: CONFIG FILE SAVED ============\n",__func__);
      Serial.printf("[%s]: Configuration saved to file: %s due to: \"%s\"\n",__func__,filename,reason);
      char doc_json[JSON_CONFIG_FILE_SIZE];
      int size_pl = serializeJson(doc, doc_json);
      Serial.printf("[%s]: Size of config file: %s=%d bytes \n",__func__,filename,size_pl);
      serializeJsonPretty(doc, Serial);
      Serial.printf("\n[%s]: ============ DEBUG: CONFIG FILE SAVED END ============\n",__func__);
    #endif
    return true;
  } else 
  {
    Serial.printf("[%s]: writing to file %S FAILED\n",__func__,filename);
    return false;
  }

}

// Save configuration to a file from variable "config" to file "filename" - plain text version
bool savePlainConfigFile(const char *filename, const Config &config, const char* reason) 
{
  File file = LittleFS.open(filename, "w");
  if (!file) 
  {
    Serial.printf("[%s]: Failed to write to config file\n",__func__);
    return false;
  }
  DynamicJsonDocument doc(JSON_CONFIG_FILE_SIZE);
  doc.set(config);
  // Serialize JSON to file
  bool success = serializeJson(doc, file) > 0;
  if (!success) 
  {
    Serial.printf("[%s]: writing to file FAILED\n",__func__);
    return false;
  } 
  #ifdef DEBUG
    Serial.printf("[%s]: ============ DEBUG: CONFIG FILE SAVED ============\n",__func__);
    Serial.printf("[%s]: Configuration saved to file: %s due to: \"%s\"\n",__func__,filename,reason);
    char doc_json[JSON_CONFIG_FILE_SIZE];
    int size_pl = serializeJson(doc, doc_json);
    Serial.printf("[%s]: Size of config file: %s=%d bytes \n",__func__,filename,size_pl);
    serializeJsonPretty(doc, Serial);
    Serial.printf("\n[%s]: ============ DEBUG: CONFIG FILE SAVED END ============\n",__func__);
  #endif
  return true;

}


#endif /* #ifndef littlefs_configfile_functions_h */