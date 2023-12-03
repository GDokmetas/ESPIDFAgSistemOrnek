#ifndef WN_HTTP_SERVER_CONFIG_H
#define WN_HTTP_SERVER_CONFIG_H

//* User Configurations 

#define WN_SERVER_MQTT_CONFIG_ENABLED 1 
#define WN_SERVER_LOG_ENABLED 1
#define WN_SERVER_CUSTOM_ERROR_HTML 0
#define WN_SERVER_FORM_PARAM_CHECK_ENABLED 1
#define WN_SERVER_NVS_PARAMS_LOG_ENABLED 1
// NVS Keys 

#define WN_SERVER_NVS_CHECK_KEY             "iotSave"
#define WN_SERVER_NVS_AP_NAME_KEY           "ap_name"
#define WN_SERVER_NVS_AP_PASS_KEY           "ap_pass"
#define WN_SERVER_NVS_STA_SSID_KEY          "sta_ssid"
#define WN_SERVER_NVS_STA_PASS_KEY          "sta_pass"

#if (WN_SERVER_MQTT_CONFIG_ENABLED)
#define WN_SERVER_NVS_MQTT_SERVER_NAME_KEY  "mqtt_name"
#define WN_SERVER_NVS_MQTT_SERVER_PORT_KEY  "mqtt_port"
#define WN_SERVER_NVS_MQTT_USER_NAME_KEY    "mqtt_user"
#define WN_SERVER_NVS_MQTT_USER_PASS_KEY    "mqtt_pass"
#endif 

//* System Configurations
#endif 