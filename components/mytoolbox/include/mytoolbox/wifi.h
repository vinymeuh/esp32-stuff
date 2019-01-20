#ifndef __MY_TOOLBOX_WIFI_H__
#define __MY_TOOLBOX_WIFI_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "freertos/event_groups.h"

/* FreeRTOS event group to signal when we are connected */
extern EventGroupHandle_t wifi_event_group;
extern const int WIFI_CONNECTED_BIT;

esp_err_t initialise_wifi_sta(const char* hostname);

#ifdef __cplusplus
}
#endif

#endif
