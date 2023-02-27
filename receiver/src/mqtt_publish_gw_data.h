#pragma once
/*
mqtt functions for gateway
*/

// #include "config.h"
// #include "variables.h"


// this device (gateway) definition to be able to create devices in HA
//MQTT_DEVICE_IDENTIFIER is in setup_wifi() as MAC is needed
#define CREATE_GW_MQTT_DEVICE \
dev = config.createNestedObject("device"); \
dev["ids"]=MQTT_DEVICE_IDENTIFIER;  \
dev["name"]="ESPnow_" + String(ROLE_NAME) + "_" + String(HOSTNAME); \
dev["mdl"]=MODEL; \
dev["mf"]="ZH"; \
dev["sw"]= String(VERSION)+ "," + String(__DATE__) + "_" + String(__TIME__);
// gateway device definition END

// gateway status config - sent by heartbeat() periodically to HA
bool mqtt_publish_gw_status_config()
{
  if (!mqtt_connected) return false;
  bool publish_status = false;      //changes locally in each config publish
  bool total_publish_status = true; //changes globally in each config publish when any fails

  // mac
  char mac_conf_topic[60];
  snprintf(mac_conf_topic,sizeof(mac_conf_topic),"homeassistant/sensor/%s/mac/config",HOSTNAME);
  if (debug_mode) Serial.println("mac_conf_topic="+String(mac_conf_topic));

  char mac_name[30];
  snprintf(mac_name,sizeof(mac_name),"%s_mac",HOSTNAME);
  if (debug_mode) Serial.println("mac_name="+String(mac_name));

  // ip
  char ip_conf_topic[60];
  snprintf(ip_conf_topic,sizeof(ip_conf_topic),"homeassistant/sensor/%s/ip/config",HOSTNAME);
  if (debug_mode) Serial.println("ip_conf_topic="+String(ip_conf_topic));

  char ip_name[30];
  snprintf(ip_name,sizeof(ip_name),"%s_ip",HOSTNAME);
  if (debug_mode) Serial.println("ip_name="+String(ip_name));  

  // status
  char status_conf_topic[60];
  snprintf(status_conf_topic,sizeof(status_conf_topic),"homeassistant/sensor/%s/status/config",HOSTNAME);
  if (debug_mode) Serial.println("status_conf_topic="+String(status_conf_topic));

  char status_name[30];
  snprintf(status_name,sizeof(status_name),"%s_mystatus",HOSTNAME);
  if (debug_mode) Serial.println("status_name="+String(status_name));

  // uptime
  char uptime_conf_topic[60];
  snprintf(uptime_conf_topic,sizeof(uptime_conf_topic),"homeassistant/sensor/%s/uptime/config",HOSTNAME);
  if (debug_mode) Serial.println("uptime_conf_topic="+String(uptime_conf_topic));

  char uptime_name[30];
  snprintf(uptime_name,sizeof(uptime_name),"%s_uptime",HOSTNAME);
  if (debug_mode) Serial.println("uptime_name="+String(uptime_name));

  // version
  char version_conf_topic[60];
  snprintf(version_conf_topic,sizeof(version_conf_topic),"homeassistant/sensor/%s/version/config",HOSTNAME);
  if (debug_mode) Serial.println("version_conf_topic="+String(version_conf_topic));

  char version_name[30];
  snprintf(version_name,sizeof(version_name),"%s_version",HOSTNAME);
  if (debug_mode) Serial.println("version_name="+String(version_name));

  // rssi of gateway<->AP
  char rssi_conf_topic[60];
  snprintf(rssi_conf_topic,sizeof(rssi_conf_topic),"homeassistant/sensor/%s/rssi/config",HOSTNAME);
  if (debug_mode) Serial.println("rssi_conf_topic="+String(rssi_conf_topic));

  char rssi_name[30];
  snprintf(rssi_name,sizeof(rssi_name),"%s_rssi",HOSTNAME);
  if (debug_mode) Serial.println("rssi_name="+String(rssi_name));

  // common topic
  char status_state_topic[60];
  snprintf(status_state_topic,sizeof(status_state_topic),"%s/sensor/state",HOSTNAME);
  if (debug_mode) Serial.println("status_state_topic="+String(status_state_topic));

  StaticJsonDocument<JSON_CONFIG_SIZE> config;
  JsonObject dev;
  int size_c;
  char config_json[JSON_CONFIG_SIZE];

// gw mac config
  config.clear();
  config["name"] = mac_name;
  config["stat_t"] = status_state_topic;
  config["val_tpl"] = "{{value_json.mac}}";
  config["uniq_id"] = mac_name;
  config["frc_upd"] = "true";
  config["entity_category"] = "diagnostic";
  // config["exp_aft"] = 60;

  CREATE_GW_MQTT_DEVICE

  size_c = serializeJson(config, config_json);

  if (!mqttc.publish(mac_conf_topic,(uint8_t*)config_json,strlen(config_json), false))
  {
    publish_status = false; total_publish_status = false;
    Serial.printf("[%s]: PUBLISH FAILED for %s\n",__func__,mac_conf_topic);
  } else
  {
    publish_status = true;
    if (debug_mode) {Serial.printf("[%s]: PUBLISH SUCCESSFULL for %s\n",__func__,mac_conf_topic);}
  }

  if (debug_mode) {
    Serial.println("\n============ DEBUG CONFIG MAC ============");
    Serial.println("Size of mac config="+String(size_c)+" bytes");
    Serial.println("Serialised config_json:");
    Serial.println(config_json);
    Serial.println("serializeJsonPretty");
    serializeJsonPretty(config, Serial);
    if (publish_status) {
      Serial.println("\n MAC CONFIG OK");
    } else
    {
      Serial.println("\n PRETTYONTIME CONFIG UNSUCCESSFULL");
    }
    Serial.println("============ DEBUG CONFIG MAC END ========\n");
  }

// gw ip config
  config.clear();
  config["name"] = ip_name;
  config["stat_t"] = status_state_topic;
  config["val_tpl"] = "{{value_json.ip}}";
  config["uniq_id"] = ip_name;
  config["frc_upd"] = "true";
  config["entity_category"] = "diagnostic";
  // config["exp_aft"] = 60;

  CREATE_GW_MQTT_DEVICE

  size_c = serializeJson(config, config_json);

  if (!mqttc.publish(ip_conf_topic,(uint8_t*)config_json,strlen(config_json), false))
  {
    publish_status = false; total_publish_status = false;
    Serial.printf("[%s]: PUBLISH FAILED for %s\n",__func__,ip_conf_topic);
  } else
  {
    publish_status = true;
    if (debug_mode) {Serial.printf("[%s]: PUBLISH SUCCESSFULL for %s\n",__func__,ip_conf_topic);}
  }

  if (debug_mode) {
    Serial.println("\n============ DEBUG CONFIG MAC ============");
    Serial.println("Size of ip config="+String(size_c)+" bytes");
    Serial.println("Serialised config_json:");
    Serial.println(config_json);
    Serial.println("serializeJsonPretty");
    serializeJsonPretty(config, Serial);
    if (publish_status) {
      Serial.println("\n IP CONFIG OK");
    } else
    {
      Serial.println("\n PRETTYONTIME CONFIG UNSUCCESSFULL");
    }
    Serial.println("============ DEBUG CONFIG IP END ========\n");
  }



// status config
  config.clear();
  config["name"] = status_name;
  config["stat_t"] = status_state_topic;
  config["qos"] = 2;
  config["uniq_id"] = status_name;
  config["val_tpl"] = "{{value_json.status}}";
  config["frc_upd"] = "true";
  config["entity_category"] = "diagnostic";
  // config["exp_aft"] = 60;

  CREATE_GW_MQTT_DEVICE

  size_c = serializeJson(config, config_json);

  if (!mqttc.publish(status_conf_topic,(uint8_t*)config_json,strlen(config_json), false))
  {
    publish_status = false; total_publish_status = false;
    Serial.printf("[%s]: PUBLISH FAILED for %s\n",__func__,status_conf_topic);
  } else
  {
    publish_status = true;
    if (debug_mode) {Serial.printf("[%s]: PUBLISH SUCCESSFULL for %s\n",__func__,status_conf_topic);}
  }

  if (debug_mode) {
    Serial.println("\n============ DEBUG [STATUS] CONFIG:  ============");
    Serial.println("Size of config_json="+String(size_c)+" bytes");
    Serial.println("Serialised config_json:");
    Serial.println(config_json);
    Serial.println("serializeJsonPretty(config, Serial)");
    serializeJsonPretty(config, Serial);
    if (publish_status) {
      Serial.println("\n CONFIG published OK");
    } else
    {
      Serial.println("\n CONFIG UNSUCCESSFULL");
    }
    Serial.println("============ DEBUG [STATUS] CONFIG END ========\n");
  }

// uptime config
  config.clear();
  config["name"] = uptime_name;
  config["stat_t"] = status_state_topic;
  config["qos"] = 2;
  config["retain"] = "true";
  config["uniq_id"] = uptime_name;
  config["val_tpl"] = "{{value_json.uptime}}";
  config["frc_upd"] = "true";
  config["entity_category"] = "diagnostic";
  // config["exp_aft"] = 60;

  CREATE_GW_MQTT_DEVICE

  size_c = serializeJson(config, config_json);

  if (!mqttc.publish(uptime_conf_topic,(uint8_t*)config_json,strlen(config_json), false))
  {
    publish_status = false; total_publish_status = false;
    Serial.printf("[%s]: PUBLISH FAILED for %s\n",__func__,uptime_conf_topic);
  } else
  {
    publish_status = true;
    if (debug_mode) {Serial.printf("[%s]: PUBLISH SUCCESSFULL for %s\n",__func__,uptime_conf_topic);}
  }

  if (debug_mode) {
    Serial.println("\n============ DEBUG [UPTIME] CONFIG:  ============");
    Serial.println("Size of config_json="+String(size_c)+" bytes");
    Serial.println("Serialised config_json:");
    Serial.println(config_json);
    Serial.println("serializeJsonPretty(config, Serial)");
    serializeJsonPretty(config, Serial);
    if (publish_status) {
      Serial.println("\n CONFIG published OK");
    } else
    {
      Serial.println("\n CONFIG UNSUCCESSFULL");
    }
    Serial.println("============ DEBUG [UPTIME] CONFIG END ========\n");
  }

// version config
  config.clear();
  config["name"] = version_name;
  config["stat_t"] = status_state_topic;
  config["qos"] = 2;
  config["retain"] = "true";
  config["uniq_id"] = version_name;
  config["val_tpl"] = "{{value_json.version}}";
  config["frc_upd"] = "true";
  config["entity_category"] = "diagnostic";
  // config["exp_aft"] = 60;

  CREATE_GW_MQTT_DEVICE

  size_c = serializeJson(config, config_json);

  if (!mqttc.publish(version_conf_topic,(uint8_t*)config_json,strlen(config_json), false))
  {
    publish_status = false; total_publish_status = false;
    Serial.printf("[%s]: PUBLISH FAILED for %s\n",__func__,version_conf_topic);
  } else
  {
    publish_status = true;
    if (debug_mode) {Serial.printf("[%s]: PUBLISH SUCCESSFULL for %s\n",__func__,version_conf_topic);}
  }

  if (debug_mode) {
    Serial.println("\n============ DEBUG [VERSION] CONFIG:  ============");
    Serial.println("Size of config_json="+String(size_c)+" bytes");
    Serial.println("Serialised config_json:");
    Serial.println(config_json);
    Serial.println("serializeJsonPretty(config, Serial)");
    serializeJsonPretty(config, Serial);
    if (publish_status) {
      Serial.println("\n CONFIG published OK");
    } else
    {
      Serial.println("\n CONFIG UNSUCCESSFULL");
    }
    Serial.println("============ DEBUG [VERSION] CONFIG END ========\n");
  }

// rssi config
  config.clear();
  config["name"] = rssi_name;
  config["dev_cla"] = "signal_strength";
  config["stat_cla"] = "measurement";
  config["stat_t"] = status_state_topic;
  config["unit_of_meas"] = "dBm";
  config["val_tpl"] = "{{value_json.rssi}}";
  config["uniq_id"] = rssi_name;
  config["frc_upd"] = "true";
  config["entity_category"] = "diagnostic";
  config["exp_aft"] = 60;

  CREATE_GW_MQTT_DEVICE

  size_c = serializeJson(config, config_json);

  if (!mqttc.publish(rssi_conf_topic,(uint8_t*)config_json,strlen(config_json), false))
  {
    publish_status = false; total_publish_status = false;
    Serial.printf("[%s]: PUBLISH FAILED for %s\n",__func__,rssi_conf_topic);
  } else
  {
    publish_status = true;
    if (debug_mode) {Serial.printf("[%s]: PUBLISH SUCCESSFULL for %s\n",__func__,rssi_conf_topic);}
  }

  if (debug_mode) {
    Serial.println("\n============ DEBUG [RSSI] CONFIG  ============");
    Serial.println("Size of config="+String(size_c)+" bytes");
    Serial.println("Serialised config_json:");
    Serial.println(config_json);
    Serial.println("serializeJsonPretty");
    serializeJsonPretty(config, Serial);
    if (publish_status) {
      Serial.println("\n CONFIG published OK");
    } else
    {
      Serial.println("\n CONFIG UNSUCCESSFULL");
    }
    Serial.println("============ DEBUG [RSSI] CONFIG END ========\n");
  }

  return total_publish_status;
}

// gateway status value
bool mqtt_publish_gw_status_values(const char* status)
{
  if (!mqtt_connected) return false;
  bool publish_status = false;

  char status_state_topic[60];
  snprintf(status_state_topic,sizeof(status_state_topic),"%s/sensor/state",HOSTNAME);
  if (debug_mode) Serial.println("status_state_topic="+String(status_state_topic));

  if (!mqtt_publish_gw_status_config()){
    Serial.println("\n GW STATUS CONFIG NOT published");
    return false;
  }

  StaticJsonDocument<JSON_PAYLOAD_SIZE> payload;
  payload["status"] = status;
  payload["rssi"] = WiFi.RSSI();
  payload["mac"]  = WiFi.macAddress();
  char ip_char[20];
  snprintf(ip_char,sizeof(ip_char),"%s",WiFi.localIP().toString());
  payload["ip"]  = ip_char;

  char ret_clk[60];
  uptime(ret_clk);
  payload["uptime"] = ret_clk;

  payload["version"] = VERSION;

  char payload_json[JSON_PAYLOAD_SIZE];
  int size_pl = serializeJson(payload, payload_json);

  if (!mqttc.publish(status_state_topic,(uint8_t*)payload_json,strlen(payload_json), false))
  {
    publish_status = false;
    Serial.printf("[%s]: PUBLISH FAILED for %s\n",__func__,status_state_topic);
  } else
  {
    publish_status = true;
    if (debug_mode) {Serial.printf("[%s]: PUBLISH SUCCESSFULL for %s\n",__func__,status_state_topic);}
  }

  if (debug_mode) {
    Serial.println("\n============ DEBUG [STATUS] PAYLOAD ============");
    Serial.println("Size of payload="+String(size_pl)+" bytes");
    Serial.println("Serialised payload_json:");
    Serial.println(payload_json);
    Serial.println("serializeJsonPretty");
    serializeJsonPretty(payload, Serial);
    if (publish_status) {
      Serial.println("\n STATUS VALUES published OK");
    } else
    {
      Serial.println("\n STATUS VALUES NOT published");
    }
    Serial.println("============ DEBUG [STATUS] PAYLOAD END ========\n");
  }

  #if (USE_WEB_SERIAL == 1)
    if (print2web) WebSerial.println(payload_json);
  #endif

  return publish_status;
}

// gw last_updated_sensor config - once sensor data is sent to HA, show on HA which sensor was last updated or queue full
bool mqtt_publish_gw_last_updated_sensor_config()
{
  if (!mqtt_connected) return false;
  bool publish_status = false;      //changes locally in each config publish
  bool total_publish_status = true; //changes globally in each config publish when any fails

  char status_conf_topic[60];
  snprintf(status_conf_topic,sizeof(status_conf_topic),"homeassistant/sensor/%s/last/config",HOSTNAME);
  if (debug_mode) Serial.println("last_updated_sensor_conf_topic="+String(status_conf_topic));

  char status_name[30];
  snprintf(status_name,sizeof(status_name),"%s_last",HOSTNAME);
  if (debug_mode) Serial.println("last_updated_sensor_name="+String(status_name));

  char status_state_topic[60];
  snprintf(status_state_topic,sizeof(status_state_topic),"%s/sensor/last",HOSTNAME);
  if (debug_mode) Serial.println("last_updated_sensor_topic_state="+String(status_state_topic));

  StaticJsonDocument<JSON_CONFIG_SIZE> config;
  JsonObject dev;
  int size_c;
  char config_json[JSON_CONFIG_SIZE];

// status config
  config.clear();
  config["name"] = status_name;
  config["stat_t"] = status_state_topic;
  config["qos"] = 2;
  config["retain"] = "true";
  config["uniq_id"] = status_name;
  config["val_tpl"] = "{{value_json.status}}";
  config["frc_upd"] = "true";
  config["entity_category"] = "diagnostic";
  // config["exp_aft"] = 600;

  CREATE_GW_MQTT_DEVICE

  size_c = serializeJson(config, config_json);

  if (!mqttc.publish(status_conf_topic,(uint8_t*)config_json,strlen(config_json), false))
  {
    publish_status = false; total_publish_status = false;
    Serial.printf("[%s]: PUBLISH FAILED for %s\n",__func__,status_conf_topic);
  } else
  {
    publish_status = true;
    if (debug_mode) {Serial.printf("[%s]: PUBLISH SUCCESSFULL for %s\n",__func__,status_conf_topic);}
  }

  if (debug_mode) {
    Serial.println("\n============ DEBUG [LAST UPDATED SENSOR] CONFIG:  ============");
    Serial.println("Size of config_json="+String(size_c)+" bytes");
    Serial.println("Serialised config_json:");
    Serial.println(config_json);
    Serial.println("serializeJsonPretty(config, Serial)");
    serializeJsonPretty(config, Serial);
    if (publish_status) {
      Serial.println("\n CONFIG published OK");
    } else
    {
      Serial.println("\n CONFIG UNSUCCESSFULL");
    }
    Serial.println("============ DEBUG [LAST UPDATED SENSOR] CONFIG END ========\n");
  }
  return total_publish_status;
}

// gw last_updated_sensor value
bool mqtt_publish_gw_last_updated_sensor_values(const char* status)
{
  if (!mqtt_connected) return false;
  bool publish_status = false;

  char status_state_topic[60];
  snprintf(status_state_topic,sizeof(status_state_topic),"%s/sensor/last",HOSTNAME);
  if (debug_mode) Serial.println("last_updated_sensor_topic_state="+String(status_state_topic));

  if (!mqtt_publish_gw_last_updated_sensor_config()){
    Serial.println("\n STATUS CONFIG NOT published");
    return false;
  }

  StaticJsonDocument<JSON_PAYLOAD_SIZE> payload;
  payload["status"] = status;

  char payload_json[JSON_PAYLOAD_SIZE];
  int size_pl = serializeJson(payload, payload_json);

  if (!mqttc.publish(status_state_topic,(uint8_t*)payload_json,strlen(payload_json), false))
  {
    publish_status = false;
    Serial.printf("[%s]: PUBLISH FAILED for %s\n",__func__,status_state_topic);
  } else
  {
    publish_status = true;
    if (debug_mode) {Serial.printf("[%s]: PUBLISH SUCCESSFULL for %s\n",__func__,status_state_topic);}
  }

  if (debug_mode) {
    Serial.println("\n============ DEBUG: [LAST UPDATED SENSOR] VALUES ============");
    Serial.println("Size of payload="+String(size_pl)+" bytes");
    Serial.println("Serialised payload_json:");
    Serial.println(payload_json);
    Serial.println("serializeJsonPretty");
    serializeJsonPretty(payload, Serial);
    if (publish_status) {
      Serial.println("\n VALUES published OK");
    } else
    {
      Serial.println("\n VALUES NOT published");
    }
    Serial.println("============ DEBUG: PAYLOAD [LAST UPDATED SENSOR] VALUES END ========\n");
  }

  #if (USE_WEB_SERIAL == 1)
    if (print2web) WebSerial.println(payload_json);
  #endif

  return publish_status;
}

// gateway FW update - button on HA
bool mqtt_publish_button_update_config()
{
  if (!mqtt_connected) return false;
  bool publish_status = false;      //changes locally in each config publish
  bool total_publish_status = true; //changes globally in each config publish when any fails

  char update_conf_topic[60];
  snprintf(update_conf_topic,sizeof(update_conf_topic),"homeassistant/button/%s/update/config",HOSTNAME);
  if (debug_mode) Serial.println("update_conf_topic="+String(update_conf_topic));

  char update_name[30];
  snprintf(update_name,sizeof(update_name),"%s_update",HOSTNAME);
  if (debug_mode) Serial.println("update_name="+String(update_name));

  char update_state_topic[60];
  snprintf(update_state_topic,sizeof(update_state_topic),"%s/button/update",HOSTNAME);
  if (debug_mode) Serial.println("update_state_topic="+String(update_state_topic));

  char update_cmd_topic[30];
  snprintf(update_cmd_topic,sizeof(update_cmd_topic),"%s/cmd/update",HOSTNAME);
  if (debug_mode) Serial.println("update_cmd_topic="+String(update_cmd_topic));

  StaticJsonDocument<JSON_CONFIG_SIZE> config;
  JsonObject dev;
  int size_c;
  char config_json[JSON_CONFIG_SIZE];

// update config
  config.clear();
  config["name"] = update_name;
  config["command_topic"] = update_cmd_topic;
  config["payload_press"] = "ON";
  config["qos"] = "2";
  config["retain"] = "true";
  config["uniq_id"] = update_name;
  config["val_tpl"] = "{{value_json.update}}";
  // config["exp_aft"] = 60;

  CREATE_GW_MQTT_DEVICE

  size_c = serializeJson(config, config_json);


  if (!mqttc.publish(update_conf_topic,(uint8_t*)config_json,strlen(config_json), false))
  {
    publish_status = false; total_publish_status = false;
    Serial.printf("[%s]: PUBLISH FAILED for %s\n",__func__,update_conf_topic);
  } else
  {
    publish_status = true;
    if (debug_mode) {Serial.printf("[%s]: PUBLISH SUCCESSFULL for %s\n",__func__,update_conf_topic);}
  }

  if (debug_mode) {
    Serial.println("\n============ DEBUG [UPDATE] CONFIG:  ============");
    Serial.println("Size of config_json="+String(size_c)+" bytes");
    Serial.println("Serialised config_json:");
    Serial.println(config_json);
    Serial.println("serializeJsonPretty(config, Serial)");
    serializeJsonPretty(config, Serial);
    if (publish_status) {
      Serial.println("\n CONFIG published OK");
    } else
    {
      Serial.println("\n CONFIG UNSUCCESSFULL");
    }
    Serial.println("============ DEBUG [UPDATE] CONFIG END ========\n");
  }
  return total_publish_status;
}

// gateway restart - button on HA
bool mqtt_publish_button_restart_config()
{
  if (!mqtt_connected) return false;
  bool publish_status = false;      //changes locally in each config publish
  bool total_publish_status = true; //changes globally in each config publish when any fails

  char restart_conf_topic[60];
  snprintf(restart_conf_topic,sizeof(restart_conf_topic),"homeassistant/button/%s/restart/config",HOSTNAME);
  if (debug_mode) Serial.println("restart_conf_topic="+String(restart_conf_topic));

  char restart_name[30];
  snprintf(restart_name,sizeof(restart_name),"%s_restart",HOSTNAME);
  if (debug_mode) Serial.println("restart_name="+String(restart_name));

  char restart_state_topic[60];
  snprintf(restart_state_topic,sizeof(restart_state_topic),"%s/button/restart",HOSTNAME);
  if (debug_mode) Serial.println("restart_state_topic="+String(restart_state_topic));

  char restart_cmd_topic[30];
  snprintf(restart_cmd_topic,sizeof(restart_cmd_topic),"%s/cmd/restart",HOSTNAME);
  if (debug_mode) Serial.println("restart_cmd_topic="+String(restart_cmd_topic));

  StaticJsonDocument<JSON_CONFIG_SIZE> config;
  JsonObject dev;
  int size_c;
  char config_json[JSON_CONFIG_SIZE];

// restart config
  config.clear();
  config["name"] = restart_name;
  config["command_topic"] = restart_cmd_topic;
  config["payload_press"] = "ON";
  config["qos"] = "2";
  config["retain"] = "true";
  config["uniq_id"] = restart_name;
  config["val_tpl"] = "{{value_json.restart}}";
  // config["exp_aft"] = 60;

  CREATE_GW_MQTT_DEVICE

  size_c = serializeJson(config, config_json);

  if (!mqttc.publish(restart_conf_topic,(uint8_t*)config_json,strlen(config_json), false))
  {
    publish_status = false; total_publish_status = false;
    Serial.printf("[%s]: PUBLISH FAILED for %s\n",__func__,restart_conf_topic);
  } else
  {
    publish_status = true;
    if (debug_mode) {Serial.printf("[%s]: PUBLISH SUCCESSFULL for %s\n",__func__,restart_conf_topic);}
  }

  if (debug_mode) {
    Serial.println("\n============ DEBUG [RESTART] CONFIG:  ============");
    Serial.println("Size of config_json="+String(size_c)+" bytes");
    Serial.println("Serialised config_json:");
    Serial.println(config_json);
    Serial.println("serializeJsonPretty(config, Serial)");
    serializeJsonPretty(config, Serial);
    if (publish_status) {
      Serial.println("\n CONFIG published OK");
    } else
    {
      Serial.println("\n CONFIG UNSUCCESSFULL");
    }
    Serial.println("============ DEBUG [RESTART] CONFIG END ========\n");
  }
  return total_publish_status;
}

// stop/start updating HA with sensors data - switch on HA
bool mqtt_publish_switch_publish_config()
{
  if (!mqtt_connected) return false;
  bool publish_status = false;      //changes locally in each config publish
  bool total_publish_status = true; //changes globally in each config publish when any fails

  char publish_conf_topic[60];
  snprintf(publish_conf_topic,sizeof(publish_conf_topic),"homeassistant/switch/%s/publish/config",HOSTNAME);
  if (debug_mode) Serial.println("publish_conf_topic="+String(publish_conf_topic));

  char publish_name[30];
  snprintf(publish_name,sizeof(publish_name),"%s_publish",HOSTNAME);
  if (debug_mode) Serial.println("publish_name="+String(publish_name));

  char publish_state_topic[60];
  snprintf(publish_state_topic,sizeof(publish_state_topic),"%s/switch/publish",HOSTNAME);
  if (debug_mode) Serial.println("publish_state_topic="+String(publish_state_topic));

  char publish_cmd_topic[30];
  snprintf(publish_cmd_topic,sizeof(publish_cmd_topic),"%s/cmd/publish",HOSTNAME);
  if (debug_mode) Serial.println("restart_cmd_topic="+String(publish_cmd_topic));

  StaticJsonDocument<JSON_CONFIG_SIZE> config;
  JsonObject dev;
  int size_c;
  char config_json[JSON_CONFIG_SIZE];

// publish config
  config.clear();
  config["name"] = publish_name;
  config["stat_t"] = publish_state_topic;
  config["command_topic"] = publish_cmd_topic;
  config["payload_on"] = "ON";
  config["payload_off"] = "OFF";
  // config["optimistic"] = "true";
  config["qos"] = "2";
  config["retain"] = "true";
  config["device_class"] = "switch";
  config["uniq_id"] = publish_name;
  config["entity_category"] = "config";
  config["val_tpl"] = "{{value_json.publish}}";
  // config["exp_aft"] = 60;

  CREATE_GW_MQTT_DEVICE

  size_c = serializeJson(config, config_json);

  if (!mqttc.publish(publish_conf_topic,(uint8_t*)config_json,strlen(config_json), false))
  {
    publish_status = false; total_publish_status = false;
    Serial.printf("[%s]: PUBLISH FAILED for %s\n",__func__,publish_conf_topic);
  } else
  {
    publish_status = true;
    if (debug_mode) {Serial.printf("[%s]: PUBLISH SUCCESSFULL for %s\n",__func__,publish_conf_topic);}
  }

  if (debug_mode) {
    Serial.println("\n============ DEBUG [PUBLISH SWITCH] CONFIG  ============");
    Serial.println("Size of config_json="+String(size_c)+" bytes");
    Serial.println("Serialised config_json:");
    Serial.println(config_json);
    Serial.println("serializeJsonPretty(config, Serial)");
    serializeJsonPretty(config, Serial);
    if (publish_status) {
      Serial.println("\n CONFIG published OK");
    } else
    {
      Serial.println("\n CONFIG UNSUCCESSFULL");
    }
    Serial.println("============ DEBUG [PUBLISH SWITCH] CONFIG END ========\n");
  }
  return total_publish_status;
}

bool mqtt_publish_switch_publish_values()
{
  if (!mqtt_connected) return false;
  bool publish_status = false;

  char publish_state_topic[60];
  snprintf(publish_state_topic,sizeof(publish_state_topic),"%s/switch/publish",HOSTNAME);
  if (debug_mode) Serial.println("publish_state_topic="+String(publish_state_topic));

  if (!mqtt_publish_switch_publish_config())
  {
    Serial.printf("[%s]: PUBLISH CONFIG NOT published, leaving\n",__func__);
    return false;
  }

  StaticJsonDocument<JSON_PAYLOAD_SIZE> payload;
  if (publish_sensors_to_ha) {payload["publish"] = "ON";} else {payload["publish"] = "OFF";}

  char payload_json[JSON_PAYLOAD_SIZE];
  int size_pl = serializeJson(payload, payload_json);

  if (!mqttc.publish(publish_state_topic,(uint8_t*)payload_json,strlen(payload_json), false))
  {
    publish_status = false;
    Serial.printf("[%s]: PUBLISH FAILED for %s\n",__func__,publish_state_topic);
  } else
  {
    publish_status = true;
    if (debug_mode) {Serial.printf("[%s]: PUBLISH SUCCESSFULL for %s\n",__func__,publish_state_topic);}
  }

  if (debug_mode) {
    Serial.println("\n============ DEBUG: [PUBLISH SWITCH] PAYLOAD ============");
    Serial.println("Size of payload="+String(size_pl)+" bytes");
    Serial.println("Serialised payload_json:");
    Serial.println(payload_json);
    Serial.println("serializeJsonPretty");
    serializeJsonPretty(payload, Serial);
    if (publish_status) {
      Serial.println("\n publish VALUES published OK");
    } else
    {
      Serial.println("\n publish VALUES NOT published");
    }
    Serial.println("============ DEBUG: [PUBLISH SWITCH] END ========\n");
  }

  #if (USE_WEB_SERIAL == 1)
    if (print2web) WebSerial.println(payload_json);
  #endif
  
  return publish_status;
}


void heartbeat()
{
  unsigned long function_start = millis();
  bool publish_status = true;

  set_gateway_led_level(1);

  char pretty_ontime[17]; // "999d 24h 60m 60s" = 16 characters
  ConvertSectoDay(millis()/1000,pretty_ontime);
  Serial.printf("[%s]: updating GW status %s after boot...",__func__,pretty_ontime);

  // publish_status = publish_status && mqtt_publish_gw_status_values("online");
  if (calibration_in_progress) 
  {
    publish_status = publish_status && mqtt_publish_gw_status_values("calibration");
  } else
  {
    publish_status = publish_status && mqtt_publish_gw_status_values("online");
  }
  publish_status = publish_status && mqtt_publish_button_update_config();
  publish_status = publish_status && mqtt_publish_button_restart_config();
  publish_status = publish_status && mqtt_publish_switch_publish_values();
  publish_status = publish_status && mqtt_publish_button_config("reset");
  publish_status = publish_status && mqtt_publish_button_config("reset_cmd_queue");
  publish_status = publish_status && mqtt_publish_light_config("led_sensors",false);
  publish_status = publish_status && mqtt_publish_light_config("led_gateway",false);
  publish_status = publish_status && mqtt_publish_light_config("led_standby",false);


  if (!mqtt_published_to_ha)
  {
    if (publish_status) 
    { 
      mqtt_published_to_ha = true;
      write_badbootcount(0);
    }
  }

  #if (USE_BMP280 == 1)
    char pressure_chr[10]; char tempbmp280_chr[10];
    measure_temp_pressure(pressure_chr,tempbmp280_chr);
    publish_status = publish_status && mqtt_publish_sensor_with_unit_value("airpressure","hPa",pressure_chr);
    publish_status = publish_status && mqtt_publish_sensor_with_unit_value("bmp280_temperature","°C",tempbmp280_chr);
  #endif 

  #if (MEASURE_LUX == 1)
    char lux[7];
    get_lux(lux);
    publish_status = publish_status && mqtt_publish_sensor_with_unit_value("lux","lx",lux);
  #endif

  #if (USE_MHZ19_CO2 == 1)
    publish_status = publish_status && mqtt_publish_sensor_with_unit_value("co2", "ppm",co2);
  #endif

  int queue_count = uxQueueMessagesWaiting(queue);
  // send queue status
  if (!publish_sensors_to_ha)
  {
    char queue_status[20];
    snprintf(queue_status, sizeof(queue_status), "queue: %d/%d",queue_count,MAX_QUEUE_COUNT);
    publish_status = publish_status && mqtt_publish_gw_last_updated_sensor_values(queue_status);
  }
  // send warning
  if (queue_count == MAX_QUEUE_COUNT)
  {
    Serial.printf("[%s]: ERROR: queue is full!\n",__func__);
    publish_status = publish_status && mqtt_publish_gw_last_updated_sensor_values("queue: FULL");
  }

  if (publish_status)
  {
    Serial.println("SUCCESSFULL");
  } else
  {
    Serial.println("FAILED");
  }
  
  u_int8_t queue_commands_count = uxQueueMessagesWaiting(queue_commands);
  char queue_size_char[4];
  

  // check if correction works it as it was counting time from restart ;-)
  // reset the queue with commands for senders if timeout expired, count from last command received
  if (queue_commands_count > 0)
  {
    if ((millis() - last_cmd_received) >= (COMMAND_QUEUE_TIMEOUT_S * 1000))
    {
      xQueueReset( queue_commands );
      queue_commands_count = uxQueueMessagesWaiting(queue_commands);
      snprintf(queue_size_char,sizeof(queue_size_char),"%d",queue_commands_count);
      mqtt_publish_text_sensor_value("cmd_queue_size", queue_size_char);
    }
  }


  snprintf(queue_size_char,sizeof(queue_size_char),"%d",queue_commands_count);
  mqtt_publish_text_sensor_value("cmd_queue_size", queue_size_char);
  #ifdef DEBUG
    Serial.printf("[%s]: Commands for senders in the queue: %d\n",__func__,queue_commands_count);
  #endif
  // reset the queue with commands for senders if queue is full
  if (queue_commands_count >= MAX_QUEUE_COMMANDS_COUNT)
  {
    publish_status = publish_status && mqtt_publish_gw_status_values("cmd queue FULL");
    Serial.print("MAX_QUEUE_COMMANDS_COUNT reached: ");Serial.println(queue_commands_count);
    Serial.println("resetting the queue with commands for senders...");
    xQueueReset( queue_commands );
    queue_commands_count = uxQueueMessagesWaiting(queue_commands);
    #ifdef DEBUG
      Serial.print("New commands in the queue: ");Serial.println(queue_commands_count);
    #endif
    publish_status = publish_status && mqtt_publish_gw_status_values("online");
  }
  // reset the queue with commands for senders if queue is full END

  set_gateway_led_level(0);
  // Serial.printf(" -> it took %ums\n",(millis()-function_start));
}

// universal button on HA - config
bool mqtt_publish_button_config(const char* button)
{
  if (!mqtt_connected) return false;
  bool debug_mode = false;
  bool publish_status = false;      //changes locally in each config publish
  bool total_publish_status = true; //changes globally in each config publish when any fails

  char button_conf_topic[100];
  snprintf(button_conf_topic,sizeof(button_conf_topic),"homeassistant/button/%s/%s/config",HOSTNAME,button);
  if (debug_mode) Serial.println("button_conf_topic="+String(button_conf_topic));

  char button_name[30];
  snprintf(button_name,sizeof(button_name),"%s_%s",HOSTNAME,button);
  if (debug_mode) Serial.println("button_name="+String(button_name));

  char button_state_topic[60];
  snprintf(button_state_topic,sizeof(button_state_topic),"%s/button/%s",HOSTNAME,button);
  if (debug_mode) Serial.println("button_state_topic="+String(button_state_topic));

  char button_cmd_topic[30];
  snprintf(button_cmd_topic,sizeof(button_cmd_topic),"%s/cmd/%s",HOSTNAME,button);
  if (debug_mode) Serial.println("button_cmd_topic="+String(button_cmd_topic));

  char button_val_tpl[30];
  snprintf(button_val_tpl,sizeof(button_val_tpl),"{{value_json.%s}}",button);
  if (debug_mode) Serial.println("button_val_tpl="+String(button_val_tpl));

  StaticJsonDocument<JSON_CONFIG_SIZE> config;
  JsonObject dev;
  int size_c;
  char config_json[JSON_CONFIG_SIZE];

// button config
  config.clear();
  config["name"] = button_name;
  config["command_topic"] = button_cmd_topic;
  config["payload_press"] = "ON";
  config["qos"] = "2";
  config["retain"] = "false";
  config["uniq_id"] = button_name;
  config["val_tpl"] = button_val_tpl;
  // config["exp_aft"] = 60;

  CREATE_GW_MQTT_DEVICE

  size_c = serializeJson(config, config_json);

  if (!mqttc.publish(button_conf_topic,(uint8_t*)config_json,strlen(config_json), false))
  {
    publish_status = false; total_publish_status = false;
    Serial.printf("[%s]: PUBLISH FAILED for %s\n",__func__,button_conf_topic);
  } else
  {
    publish_status = true;
    if (debug_mode) {Serial.printf("[%s]: PUBLISH SUCCESSFULL for %s\n",__func__,button_conf_topic);}
  }

  if (debug_mode) {
    Serial.println("\n============ DEBUG [BUTTON] CONFIG:  ============");
    Serial.println("Size of config_json="+String(size_c)+" bytes");
    Serial.println("Serialised config_json:");
    Serial.println(config_json);
    Serial.println("serializeJsonPretty(config, Serial)");
    serializeJsonPretty(config, Serial);
    if (publish_status) {
      Serial.println("\n BUTTON CONFIG published OK");
    } else
    {
      Serial.println("\n BUTTON CONFIG UNSUCCESSFULL");
    }
    Serial.println("============ DEBUG [BUTTON] CONFIG END ========\n");
  }
  return total_publish_status;
}

// universal standalone text sensor on HA - config
bool mqtt_publish_text_sensor_config(const char* text_sensor)
{
  if (!mqtt_connected) return false;
  bool debug_mode = false;
  bool publish_status = false;      //changes locally in each config publish
  bool total_publish_status = true; //changes globally in each config publish when any fails

  char text_sensor_conf_topic[100];
  snprintf(text_sensor_conf_topic,sizeof(text_sensor_conf_topic),"homeassistant/sensor/%s/%s/config",HOSTNAME,text_sensor);
  if (debug_mode) Serial.println("text_sensor_conf_topic="+String(text_sensor_conf_topic));

  char text_sensor_name[30];
  snprintf(text_sensor_name,sizeof(text_sensor_name),"%s_%s",HOSTNAME,text_sensor);
  if (debug_mode) Serial.println("text_sensor_name="+String(text_sensor_name));

  char text_sensor_state_topic[60];
  snprintf(text_sensor_state_topic,sizeof(text_sensor_state_topic),"%s/sensor/%s",HOSTNAME,text_sensor);
  if (debug_mode) Serial.println("text_sensor_state_topic="+String(text_sensor_state_topic));

  char text_sensor_val_tpl[30];
  snprintf(text_sensor_val_tpl,sizeof(text_sensor_val_tpl),"{{value_json.%s}}",text_sensor);
  if (debug_mode) Serial.println("text_sensor_val_tpl="+String(text_sensor_val_tpl));

  StaticJsonDocument<JSON_CONFIG_SIZE> config;
  JsonObject dev;
  int size_c;
  char config_json[JSON_CONFIG_SIZE];

// text_sensor config
  config.clear();
  config["name"] = text_sensor_name;
  config["qos"] = "2";
  config["retain"] = "true";
  config["uniq_id"] = text_sensor_name;
  config["val_tpl"] = text_sensor_val_tpl;
  config["stat_t"] = text_sensor_state_topic;
  config["frc_upd"] = "true";
  // config["exp_aft"] = 60;

  CREATE_GW_MQTT_DEVICE

  size_c = serializeJson(config, config_json);

  if (!mqttc.publish(text_sensor_conf_topic,(uint8_t*)config_json,strlen(config_json), false))
  {
    publish_status = false; total_publish_status = false;
    Serial.printf("[%s]: PUBLISH FAILED for %s\n",__func__,text_sensor_conf_topic);
  } else
  {
    publish_status = true;
    if (debug_mode) {Serial.printf("[%s]: PUBLISH SUCCESSFULL for %s\n",__func__,text_sensor_conf_topic);}
  }

  if (debug_mode) {
    Serial.println("\n============ DEBUG [text_sensor] CONFIG:  ============");
    Serial.println("Size of config_json="+String(size_c)+" bytes");
    Serial.println("Serialised config_json:");
    Serial.println(config_json);
    Serial.println("serializeJsonPretty(config, Serial)");
    serializeJsonPretty(config, Serial);
    if (publish_status) {
      Serial.println("\n text_sensor CONFIG published OK");
    } else
    {
      Serial.println("\n text_sensor CONFIG UNSUCCESSFULL");
    }
    Serial.println("============ DEBUG [text_sensor] CONFIG END ========\n");
  }
  return total_publish_status;
}

// universal standalone text sensor on HA - publish value
bool mqtt_publish_text_sensor_value(const char* text_sensor, const char* text_sensor_value)
{
  if (!mqtt_connected) return false;
  bool debug_mode = false;
  bool publish_status = false;

  char text_sensor_state_topic[60];
  snprintf(text_sensor_state_topic,sizeof(text_sensor_state_topic),"%s/sensor/%s",HOSTNAME,text_sensor);
  if (debug_mode) Serial.println("text_sensor_state_topic="+String(text_sensor_state_topic));

  if (!mqtt_publish_text_sensor_config(text_sensor))
  {
    Serial.printf("[%s]: PUBLISH CONFIG NOT published, leaving\n",__func__);
    return false;
  }  

  StaticJsonDocument<JSON_PAYLOAD_SIZE> payload;
  payload[text_sensor] = text_sensor_value;

  char payload_json[JSON_PAYLOAD_SIZE];
  int size_pl = serializeJson(payload, payload_json);

  if (!mqttc.publish(text_sensor_state_topic,(uint8_t*)payload_json,strlen(payload_json), false))
  {
    publish_status = false;
    Serial.printf("[%s]: PUBLISH FAILED for %s\n",__func__,text_sensor_state_topic);
  } else
  {
    publish_status = true;
    if (debug_mode) {Serial.printf("[%s]: PUBLISH SUCCESSFULL for %s\n",__func__,text_sensor_state_topic);}
  }

  if (debug_mode) {
    Serial.println("\n============ DEBUG [text_sensor_value] PAYLOAD ============");
    Serial.println("Size of payload="+String(size_pl)+" bytes");
    Serial.println("Serialised payload_json:");
    Serial.println(payload_json);
    Serial.println("serializeJsonPretty");
    serializeJsonPretty(payload, Serial);
    if (publish_status) {
      Serial.println("\n text_sensor_value VALUES published OK");
    } else
    {
      Serial.println("\n text_sensor_value VALUES NOT published");
    }
    Serial.println("============ DEBUG [text_sensor_value] PAYLOAD END ========\n");
  }

  return publish_status;
}

// universal standalone sensor with unit on HA - config
bool mqtt_publish_sensor_with_unit_config(const char* sensor_with_unit, const char* sensor_unit)
{
  if (!mqtt_connected) return false;
  bool debug_mode = false;
  bool publish_status = false;      //changes locally in each config publish
  bool total_publish_status = true; //changes globally in each config publish when any fails

  char sensor_with_unit_conf_topic[150];
  snprintf(sensor_with_unit_conf_topic,sizeof(sensor_with_unit_conf_topic),"homeassistant/sensor/%s/%s/config",HOSTNAME,sensor_with_unit);
  if (debug_mode) Serial.println("sensor_with_unit_conf_topic="+String(sensor_with_unit_conf_topic));

  char sensor_with_unit_name[100];
  snprintf(sensor_with_unit_name,sizeof(sensor_with_unit_name),"%s_%s",HOSTNAME,sensor_with_unit);
  if (debug_mode) Serial.println("sensor_with_unit_name="+String(sensor_with_unit_name));

  char sensor_with_unit_state_topic[100];
  snprintf(sensor_with_unit_state_topic,sizeof(sensor_with_unit_state_topic),"%s/sensor/%s",HOSTNAME,sensor_with_unit);
  if (debug_mode) Serial.println("sensor_with_unit_state_topic="+String(sensor_with_unit_state_topic));

  char sensor_with_unit_val_tpl[100];
  snprintf(sensor_with_unit_val_tpl,sizeof(sensor_with_unit_val_tpl),"{{value_json.%s}}",sensor_with_unit);
  if (debug_mode) Serial.println("sensor_with_unit_val_tpl="+String(sensor_with_unit_val_tpl));

  StaticJsonDocument<JSON_CONFIG_SIZE> config;
  JsonObject dev;
  int size_c;
  char config_json[JSON_CONFIG_SIZE];

// sensor_with_unit config
  config.clear();
  config["name"] = sensor_with_unit_name;
  config["qos"] = "2";
  config["retain"] = "true";
  config["uniq_id"] = sensor_with_unit_name;
  config["val_tpl"] = sensor_with_unit_val_tpl;
  config["stat_t"] = sensor_with_unit_state_topic;
  config["frc_upd"] = "true";
  config["unit_of_meas"] = sensor_unit;
  // config["exp_aft"] = 60;

  CREATE_GW_MQTT_DEVICE

  size_c = serializeJson(config, config_json);

  if (!mqttc.publish(sensor_with_unit_conf_topic,(uint8_t*)config_json,strlen(config_json), false))
  {
    publish_status = false; total_publish_status = false;
    Serial.printf("[%s]: PUBLISH FAILED for %s\n",__func__,sensor_with_unit_conf_topic);
  } else
  {
    publish_status = true;
    if (debug_mode) {Serial.printf("[%s]: PUBLISH SUCCESSFULL for %s\n",__func__,sensor_with_unit_conf_topic);}
  }

  if (debug_mode) {
    Serial.println("\n============ DEBUG [sensor_with_unit] CONFIG:  ============");
    Serial.println("Size of config_json="+String(size_c)+" bytes");
    Serial.println("Serialised config_json:");
    Serial.println(config_json);
    Serial.println("serializeJsonPretty(config, Serial)");
    serializeJsonPretty(config, Serial);
    if (publish_status) {
      Serial.println("\n sensor_with_unit CONFIG published OK");
    } else
    {
      Serial.println("\n sensor_with_unit CONFIG UNSUCCESSFULL");
    }
    Serial.println("============ DEBUG [sensor_with_unit] CONFIG END ========\n");
  }
  return total_publish_status;
}

// universal standalone sensor with unit on HA - publish value - value is float - not string!
bool mqtt_publish_sensor_with_unit_value(const char* sensor_with_unit, const char* sensor_unit, const char* sensor_with_unit_value)
{
  if (!mqtt_connected) return false;
  bool debug_mode = false;
  bool publish_status = false;

  char sensor_with_unit_state_topic[60];
  snprintf(sensor_with_unit_state_topic,sizeof(sensor_with_unit_state_topic),"%s/sensor/%s",HOSTNAME,sensor_with_unit);
  if (debug_mode) Serial.println("sensor_with_unit_state_topic="+String(sensor_with_unit_state_topic));

  if (!mqtt_publish_sensor_with_unit_config(sensor_with_unit, sensor_unit))
  {
    Serial.printf("[%s]: PUBLISH CONFIG NOT published, leaving\n",__func__);
    return false;
  } 

  StaticJsonDocument<JSON_PAYLOAD_SIZE> payload;
  
  // check if this is always float - what if string? it will be then ZERO always

  float value = atof(sensor_with_unit_value);
  payload[sensor_with_unit] = value;

  char payload_json[JSON_PAYLOAD_SIZE];
  int size_pl = serializeJson(payload, payload_json);

  if (!mqttc.publish(sensor_with_unit_state_topic,(uint8_t*)payload_json,strlen(payload_json), false))
  {
    publish_status = false;
    Serial.printf("[%s]: PUBLISH FAILED for %s\n",__func__,sensor_with_unit_state_topic);
  } else
  {
    publish_status = true;
    if (debug_mode) {Serial.printf("[%s]: PUBLISH SUCCESSFULL for %s\n",__func__,sensor_with_unit_state_topic);}
  }

  if (debug_mode) {
    Serial.println("\n============ DEBUG [sensor_with_unit_value] PAYLOAD ============");
    Serial.println("Size of payload="+String(size_pl)+" bytes");
    Serial.println("Serialised payload_json:");
    Serial.println(payload_json);
    Serial.println("serializeJsonPretty");
    serializeJsonPretty(payload, Serial);
    if (publish_status) {
      Serial.println("\n sensor_with_unit_value VALUES published OK");
    } else
    {
      Serial.println("\n sensor_with_unit_value VALUES NOT published");
    }
    Serial.println("============ DEBUG [sensor_with_unit_value] PAYLOAD END ========\n");
  }

  return publish_status;
}

// universal light on HA - config
bool mqtt_publish_light_config(const char* light, bool optimistic)
{
  if (!mqtt_connected) return false;
  bool debug_mode = false;
  bool publish_status = false;      //changes locally in each config publish
  bool total_publish_status = true; //changes globally in each config publish when any fails

  char light_conf_topic[100];
  snprintf(light_conf_topic,sizeof(light_conf_topic),"homeassistant/light/%s/%s/config",HOSTNAME,light);
  if (debug_mode) Serial.println("light_conf_topic="+String(light_conf_topic));

  char light_name[60];
  snprintf(light_name,sizeof(light_name),"%s_%s",HOSTNAME,light);
  if (debug_mode) Serial.println("light_name="+String(light_name));

  char light_state_topic[60];
  snprintf(light_state_topic,sizeof(light_state_topic),"%s/light/%s/state",HOSTNAME,light);
  if (debug_mode) Serial.println("light_state_topic="+String(light_state_topic));

  char light_cmd_topic[60];
  snprintf(light_cmd_topic,sizeof(light_cmd_topic),"%s/light/%s/cmd/switch",HOSTNAME,light);
  if (debug_mode) Serial.println("light_cmd_topic="+String(light_cmd_topic));

  char light_val_tpl[60];
  snprintf(light_val_tpl,sizeof(light_val_tpl),"{{value_json.light}}");
  if (debug_mode) Serial.println("light_val_tpl="+String(light_val_tpl));

  char brightness_cmd_topic[60];
  snprintf(brightness_cmd_topic,sizeof(brightness_cmd_topic),"%s/light/%s/cmd/brightness",HOSTNAME,light);
  if (debug_mode) Serial.println("brightness_cmd_topic="+String(brightness_cmd_topic));

  char brightness_val_tpl[60];
  snprintf(brightness_val_tpl,sizeof(brightness_val_tpl),"{{value_json.brightness}}");
  if (debug_mode) Serial.println("brightness_val_tpl="+String(brightness_val_tpl));  

  StaticJsonDocument<JSON_CONFIG_SIZE> config;
  JsonObject dev;
  int size_c;
  char config_json[JSON_CONFIG_SIZE];

  config.clear();
  config["name"]=light_name;
  config["dev_cla"] = "light";

  config["state_topic"]=light_state_topic;
  config["command_topic"]=light_cmd_topic;
  config["state_value_template"]="{{value_json.state}}";

  config["brightness_state_topic"]=light_state_topic;
  config["brightness_command_topic"]=brightness_cmd_topic;
  config["brightness_value_template"]="{{value_json.brightness}}";

  config["payload_on"] = "ON";
  config["payload_off"] = "OFF";

  if (optimistic)
    config["optimistic"] = "true";
  else 
    config["optimistic"] = "false";;

  config["qos"] = "2";
  config["retain"] = "true";

  config["unique_id"]=light_name;

  config["frc_upd"] = "true";
  // config["exp_aft"] = 600;


  CREATE_GW_MQTT_DEVICE

  size_c = serializeJson(config, config_json);

  if (!mqttc.publish(light_conf_topic,(uint8_t*)config_json,strlen(config_json), false))
  {
    publish_status = false; total_publish_status = false;
    Serial.printf("[%s]: PUBLISH FAILED for %s\n",__func__,light_conf_topic);
  } else
  {
    publish_status = true;
    if (debug_mode) {Serial.printf("[%s]: PUBLISH SUCCESSFULL for %s\n",__func__,light_conf_topic);}
  }

  if (debug_mode) {
    Serial.println("\n============ DEBUG [light] CONFIG:  ============");
    Serial.println("Size of config_json="+String(size_c)+" bytes");
    Serial.println("Serialised config_json:");
    Serial.println(config_json);
    Serial.println("serializeJsonPretty(config, Serial)");
    serializeJsonPretty(config, Serial);
    if (publish_status) {
      Serial.println("\n light CONFIG published OK");
    } else
    {
      Serial.println("\n light CONFIG UNSUCCESSFULL");
    }
    Serial.println("============ DEBUG [light] CONFIG END ========\n");
  }
  return total_publish_status;
}

// universal light on HA - values
bool mqtt_publish_light_values(const char* light, bool power, u_int8_t brightness)
{
  if (!mqtt_connected) return false;
  bool debug_mode = false;
  bool publish_status = false;

  char light_state_topic[60];
  snprintf(light_state_topic,sizeof(light_state_topic),"%s/light/%s/state",HOSTNAME,light);
  if (debug_mode) Serial.println("light_state_topic="+String(light_state_topic));

  if (!mqtt_publish_light_config(light, false))
  {
    Serial.printf("[%s]: PUBLISH CONFIG NOT published, leaving\n",__func__);
    return false;
  }  

  StaticJsonDocument<JSON_PAYLOAD_SIZE> payload;
  if (power) payload["state"] = "ON"; else payload["state"] = "OFF";
  payload["brightness"] = brightness;

  char payload_json[JSON_PAYLOAD_SIZE];
  int size_pl = serializeJson(payload, payload_json);

  if (!mqttc.publish(light_state_topic,(uint8_t*)payload_json,strlen(payload_json), false))
  {
    publish_status = false;
    Serial.printf("[%s]: PUBLISH FAILED for %s\n",__func__,light_state_topic);
  } else
  {
    publish_status = true;
    if (debug_mode) {Serial.printf("[%s]: PUBLISH SUCCESSFULL for %s\n",__func__,light_state_topic);}
  }

  if (debug_mode) {
    Serial.println("\n============ DEBUG [mqtt_publish_light_values] PAYLOAD ============");
    Serial.println("Size of payload="+String(size_pl)+" bytes");
    Serial.println("Serialised payload_json:");
    Serial.println(payload_json);
    Serial.println("serializeJsonPretty");
    serializeJsonPretty(payload, Serial);
    if (publish_status) {
      Serial.println("\n mqtt_publish_light_values VALUES published OK");
    } else
    {
      Serial.println("\n mqtt_publish_light_values VALUES NOT published");
    }
    Serial.println("============ DEBUG [mqtt_publish_light_values] PAYLOAD END ========\n");
  }

  return publish_status;
}