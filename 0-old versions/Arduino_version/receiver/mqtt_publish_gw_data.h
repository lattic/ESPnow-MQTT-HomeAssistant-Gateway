#pragma once
/*
mqtt functions for gateway
*/

#include "config.h"
#include "variables.h"


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
  bool publish_status = false;      //changes locally in each config publish
  bool total_publish_status = true; //changes globally in each config publish when any fails

  // mac
  char mac_conf_topic[60];
  snprintf(mac_conf_topic,sizeof(mac_conf_topic),"homeassistant/sensor/%s/mac/config",HOSTNAME);
  if (debug_mode) Serial.println("mac_conf_topic="+String(mac_conf_topic));

  char mac_name[30];
  snprintf(mac_name,sizeof(mac_name),"%s_mac",HOSTNAME);
  if (debug_mode) Serial.println("mac_name="+String(mac_name));

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

  // commont topic
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
  config["exp_aft"] = 60;

  CREATE_GW_MQTT_DEVICE

  size_c = serializeJson(config, config_json);

  if (!mqttc.publish(mac_conf_topic,(uint8_t*)config_json,strlen(config_json), true))
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

// status config
  config.clear();
  config["name"] = status_name;
  config["stat_t"] = status_state_topic;
  config["qos"] = 2;
  config["uniq_id"] = status_name;
  config["val_tpl"] = "{{value_json.status}}";
  config["frc_upd"] = "true";
  config["entity_category"] = "diagnostic";
  config["exp_aft"] = 60;

  CREATE_GW_MQTT_DEVICE

  size_c = serializeJson(config, config_json);

  if (!mqttc.publish(status_conf_topic,(uint8_t*)config_json,strlen(config_json), true))
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
  config["exp_aft"] = 60;

  CREATE_GW_MQTT_DEVICE

  size_c = serializeJson(config, config_json);

  if (!mqttc.publish(uptime_conf_topic,(uint8_t*)config_json,strlen(config_json), true))
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
  config["exp_aft"] = 60;

  CREATE_GW_MQTT_DEVICE

  size_c = serializeJson(config, config_json);

  if (!mqttc.publish(version_conf_topic,(uint8_t*)config_json,strlen(config_json), true))
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

  if (!mqttc.publish(rssi_conf_topic,(uint8_t*)config_json,strlen(config_json), true))
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

  char ret_clk[60];
  uptime(ret_clk);
  payload["uptime"] = ret_clk;

  payload["version"] = VERSION;

  char payload_json[JSON_PAYLOAD_SIZE];
  int size_pl = serializeJson(payload, payload_json);

  if (!mqttc.publish(status_state_topic,(uint8_t*)payload_json,strlen(payload_json), true))
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

  return publish_status;
}

// gw last_updated_sensor config - once sensor data is sent to HA, show on HA which sensor was last updated or queue full
bool mqtt_publish_gw_last_updated_sensor_config()
{
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
  config["exp_aft"] = 600;

  CREATE_GW_MQTT_DEVICE

  size_c = serializeJson(config, config_json);

  if (!mqttc.publish(status_conf_topic,(uint8_t*)config_json,strlen(config_json), true))
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

  if (!mqttc.publish(status_state_topic,(uint8_t*)payload_json,strlen(payload_json), true))
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

  return publish_status;
}

// gateway FW update - button on HA
bool mqtt_publish_button_update_config()
{
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
  config["exp_aft"] = 60;

  CREATE_GW_MQTT_DEVICE

  size_c = serializeJson(config, config_json);


  if (!mqttc.publish(update_conf_topic,(uint8_t*)config_json,strlen(config_json), true))
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
  config["exp_aft"] = 60;

  CREATE_GW_MQTT_DEVICE

  size_c = serializeJson(config, config_json);

  if (!mqttc.publish(restart_conf_topic,(uint8_t*)config_json,strlen(config_json), true))
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
  config["exp_aft"] = 60;

  CREATE_GW_MQTT_DEVICE

  size_c = serializeJson(config, config_json);

  if (!mqttc.publish(publish_conf_topic,(uint8_t*)config_json,strlen(config_json), true))
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

  if (!mqttc.publish(publish_state_topic,(uint8_t*)payload_json,strlen(payload_json), true))
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

  return publish_status;
}


void heartbeat()
{
  bool publish_status = true;
  #ifdef STATUS_GW_LED_GPIO_RED
    digitalWrite(STATUS_GW_LED_GPIO_RED,HIGH);
  #endif
  char pretty_ontime[17]; // "999d 24h 60m 60s" = 16 characters
  ConvertSectoDay(millis()/1000,pretty_ontime);
  Serial.printf("[%s]: updating GW status %s after boot...",__func__,pretty_ontime);

  publish_status = publish_status && mqtt_publish_gw_status_values("online");
  publish_status = publish_status && mqtt_publish_button_update_config();
  publish_status = publish_status && mqtt_publish_button_restart_config();
  publish_status = publish_status && mqtt_publish_gw_last_updated_sensor_config();
  publish_status = publish_status && mqtt_publish_switch_publish_values();

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
    Serial.printf("SUCCESSFULL\n");
  } else
  {
    Serial.printf("FAILED\n");
  }

  #ifdef STATUS_GW_LED_GPIO_RED
    digitalWrite(STATUS_GW_LED_GPIO_RED,LOW);
  #endif
}
