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

//LittleFS:

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
    Serial.printf("Listing directory: %s\r\n", dirname);
    File root = fs.open(dirname);
    if(!root){
        Serial.println("- failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println(" - not a directory");
        return;
    }
    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels){
                cr_listDir(fs, file.name(), levels -1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("\tSIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}


//Prints the content of a file to the Serial
void cr_printFile(const char *filename) {
  File file = LittleFS.open(filename, "r");
  if (!file) {
    #ifdef DEBUG
      Serial.println("Failed to open file" + String(filename));
    #endif
    return;
  }
  Serial.println("Content of file: " + String(filename));
  while (file.available()) {
    Serial.print((char)file.read());
  }
  Serial.println();
}


//delete file
void cr_deleteFile(fs::FS &fs, const char * filename){
    #ifdef DEBUG
      Serial.printf("Deleting file: %s\r\n", filename);
    #endif
    if(fs.remove(filename)){
        #ifdef DEBUG
          Serial.println("- file deleted");
        #endif
    } else {
        #ifdef DEBUG
          Serial.println("- delete failed");
        #endif
    }
}


//writes any string to file
bool cr_writeFileStr(fs::FS &fs, const char * filename, String content){
    #ifdef DEBUG
      Serial.print("Writing to file: " + String(filename));
    #endif
    File file = fs.open(filename, FILE_WRITE);
    if(!file){
        #ifdef DEBUG
          Serial.println(" - failed to open file for writing");
        #endif
        return false;
    }
    if(file.print(content)){
        #ifdef DEBUG
          Serial.println(" - file written");
        #endif
        file.close();
        return true;
    } else {
        #ifdef DEBUG
          Serial.println(" - write failed");
        #endif
        file.close();
        return false;
    }

}


/*
struct Config 
{
Config variables - all variables with "c_" to distinguish from global variables
  uint16_t  c_sleeptime_s;            // just test
  uint32_t  c_bootCount;              // bootCount
  uint32_t  c_saved_ontime_ms;        // ontime in ms
  uint8_t   c_channel;                // WiFi channel
  uint8_t   c_wifi_ok = 0;            // WiFi successful data provided
  uint8_t   c_last_gw = 0;            // last used gateway
  char      c_ssid[33];               // WiFi ssid
  char      c_password[65];           // WiFi password
  uint8_t   c_led_pwm;                // DC for LED PWM
};

Global variables - all variables with "g_" to distinguish from config variables
uint16_t    g_sleeptime_s;            // just test
uint32_t    g_bootCount;              // bootCount
uint32_t    g_saved_ontime_ms;        // ontime in ms
uint8_t     g_wifi_channel;           // WiFi channel
uint8_t     g_wifi_ok = 0;            // WiFi successful data provided
uint8_t     g_last_gw = 0;            // last used gateway
char        g_wifi_ssid[33];          // WiFi ssid
char        g_wifi_password[65];      // WiFi password
uint8_t     g_led_pwm;                // DC for LED PWM
*/

//JSON functions
//converts to Json
void convertToJson(const Config &src, JsonVariant dst) {
  #ifdef DEBUG
    Serial.printf("[%s]: ...\n",__func__);
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
}


// converts from Json
void convertFromJson(JsonVariantConst src, Config &dst) {
  #ifdef DEBUG
    Serial.printf("[%s]: ...\n",__func__);
  #endif
  g_sleeptime_s                 = src["c_sleeptime_s"];
  g_bootCount                   = src["c_bootCount"];
  g_saved_ontime_ms             = src["c_saved_ontime_ms"];
  g_wifi_channel                = src["c_wifi_channel"];
  g_wifi_ok                     = src["c_wifi_ok"];
  g_last_gw                     = src["c_last_gw"];
  g_led_pwm                     = src["c_led_pwm"];
  g_motion                      = src["c_motion"];

  strlcpy(g_wifi_ssid, src["c_ssid"], sizeof(g_wifi_ssid));
  strlcpy(g_wifi_password, src["c_password"], sizeof(g_wifi_password));
}


// Load the configuration from a file "filename" to variable "config" - encrypted text version
bool loadEncConfigFile(const char *filename, Config &config) 
{
  // unsigned long function_start = micros();
  DynamicJsonDocument doc(JSON_CONFIG_FILE_SIZE);
  DeserializationError err = deserializeJson(doc, cipher->decryptString(cr_getFile(LittleFS, filename)).c_str());
  if (err) {
    Serial.printf("[%s]: Failed to deserialize configuration: %s\n",__func__,err.f_str());
    return false;
  }
  // Extract config from the JSON document
  config = doc.as<Config>();
  #ifdef DEBUG
    Serial.println("loaded data to variable=\"config\" from file: " + String(filename));
    char doc_json[JSON_CONFIG_FILE_SIZE];
    int size_pl = serializeJson(doc, doc_json);
    Serial.println("\n============ DEBUG: CONFIG FILE LOADED ============");
    Serial.println("Size of config file doc="+String(size_pl)+" bytes");
    // Serial.println("serializeJsonPretty");
    serializeJsonPretty(doc, Serial);
    Serial.println("\n============ DEBUG: CONFIG FILE LOADED END ============\n");
  #endif
  // Serial.printf("[%s]: took %uus\n",__func__,(micros()-function_start));
  return true;
}

// Load the configuration from a file "filename" to variable "config" - plain text version
bool loadPlainConfigFile(const char *filename, Config &config) 
{
  // unsigned long function_start = micros();
  File file = LittleFS.open(filename, "r");
  if (!file) {
    Serial.printf("[%s]: Failed to open config file\n",__func__);
    return false;
  }
  DynamicJsonDocument doc(JSON_CONFIG_FILE_SIZE);
  DeserializationError err = deserializeJson(doc, file);
  if (err) {
    Serial.printf("[%s]: Failed to deserialize configuration: %s\n",__func__,err.f_str());
    return false;
  }
  // Extract config from the JSON document
  config = doc.as<Config>();
  // Serial.println("loaded to variable=\"config\"  from file: " + String(filename));
  #ifdef DEBUG
    Serial.println("loaded data to variable=\"config\" from file: " + String(filename));
    char doc_json[JSON_CONFIG_FILE_SIZE];
    int size_pl = serializeJson(doc, doc_json);
    Serial.println("\n============ DEBUG: CONFIG FILE LOADED ============");
    Serial.println("Size of config file doc="+String(size_pl)+" bytes");
    // Serial.println("serializeJsonPretty");
    serializeJsonPretty(doc, Serial);
    Serial.println("\n============ DEBUG: CONFIG FILE LOADED END ============\n");
  #endif
  // Serial.printf("[%s]: took %uus\n",__func__,(micros()-function_start));
  return true;
}

// Save configuration to a file from variable "config" to file "filename" - encrypted text version
bool saveEncConfigFile(const char *filename, const Config &config, const char* reason) 
{
  // unsigned long function_start = micros();
  DynamicJsonDocument doc(JSON_CONFIG_FILE_SIZE);
  doc.set(config);
                    // doc["c_ontime"] = ontime;
  String serialized_to_string;
  // bool success = serializeJsonPretty(doc2, serialized_to_string); //flat or pretty format
  bool success = serializeJson(doc, serialized_to_string);
  if (!success) {
    Serial.printf("[%s]: Failed to serialize configuration to string\n",__func__);
    return false;
  }
   else {
    String text = cipher->encryptString(serialized_to_string);
    if (cr_writeFileStr(LittleFS, filename, text)){
      #ifdef DEBUG
        Serial.println("\n============ DEBUG: CONFIG FILE SAVED ============");
        Serial.println("Configuration saved to file: " + String(filename) + " due to: " + String(reason) );
        char doc_json[JSON_CONFIG_FILE_SIZE];
        int size_pl = serializeJson(doc, doc_json);
        Serial.println("Size of config file doc="+String(size_pl)+" bytes");
        Serial.println("serializeJsonPretty");
        serializeJsonPretty(doc, Serial);
        Serial.println("\n============ DEBUG: CONFIG FILE SAVED END ============");
      #endif
      // Serial.printf("[%s]: took %uus\n",__func__,(micros()-function_start));
      return true;
    } else {
      //writing failed
        Serial.printf("[%s]: writing to file FAILED\n",__func__);
      return false;
    }
  }
}

// Save configuration to a file from variable "config" to file "filename" - plain text version
bool savePlainConfigFile(const char *filename, const Config &config, const char* reason) 
{
  // unsigned long function_start = micros();
  File file = LittleFS.open(filename, "w");
  if (!file) {
    Serial.printf("[%s]: Failed to write to config file\n",__func__);
    return false;
  }
  // serializeJson(doc, file);
  DynamicJsonDocument doc(JSON_CONFIG_FILE_SIZE);
  doc.set(config);
  // Serialize JSON to file
  // bool success = serializeJsonPretty(doc, file) > 0;
  bool success = serializeJson(doc, file) > 0;
  if (!success) {
      //writing failed
    Serial.printf("[%s]: writing to file FAILED\n",__func__);
    return false;
  } else {
      #ifdef DEBUG
        Serial.println("\n============ DEBUG: CONFIG FILE SAVED ============");
        Serial.println("Configuration saved to file: " + String(filename) + " due to: " + String(reason) );
        char doc_json[JSON_CONFIG_FILE_SIZE];
        int size_pl = serializeJson(doc, doc_json);
        Serial.println("Size of config file doc="+String(size_pl)+" bytes");
        Serial.println("serializeJsonPretty");
        serializeJsonPretty(doc, Serial);
        Serial.println("\n============ DEBUG: CONFIG FILE SAVED END ============");
      #endif
      // Serial.printf("[%s]: took %uus\n",__func__,(micros()-function_start));
      return true;
  }
}


#endif /* #ifndef littlefs_configfile_functions_h */