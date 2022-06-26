#include "esp_event.h"
#include "esp_log.h"
#include "esp_wifi.h"

#include "mdns.h"
#include "nvs_flash.h"

#include "wifi.h"

static const char* TAG = "toolbox_wifi_sta";

/* FreeRTOS event group to signal when we are connected */
EventGroupHandle_t wifi_event_group;
const int WIFI_CONNECTED_BIT = BIT0;

static esp_err_t wifi_event_handler(void *ctx, system_event_t *event) 
{
    esp_err_t err;
    switch (event->event_id) {
        case SYSTEM_EVENT_STA_START:
            err = esp_wifi_connect();
            if (err) {
                ESP_LOGW(TAG, "esp_wifi_connect(): %s", esp_err_to_name(err));
            }
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            ESP_LOGW(TAG, "wifi sta disconnected");
            xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
            err = esp_wifi_connect();
            if (err) {
                ESP_LOGW(TAG, "esp_wifi_connect(): %s", esp_err_to_name(err));
            }
            break;
        default:
            break;
    }
    mdns_handle_system_event(ctx, event);   // always returns ESP_OK
    return ESP_OK;
}

esp_err_t initialise_wifi_sta(const char* hostname) 
{
    esp_err_t err;

    // reduce log verbosity for wifi
    esp_log_level_set("wifi", ESP_LOG_WARN);

    err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    if (err) {
        return err;
    }

    // initialise mDNS
    err = mdns_init();
    if (err == ESP_OK) {
        err = mdns_hostname_set(hostname);
        if (err == ESP_OK) {
            ESP_LOGI(TAG, "mDNS successfully started");
        } else {
            ESP_LOGW(TAG, "mdns_hostname_set(): %s", esp_err_to_name(err));
        }
    } else {
        ESP_LOGW(TAG, "mdns_init(): %s", esp_err_to_name(err));
    }

    // initialise wifi event group
    wifi_event_group = xEventGroupCreate();

    // Wi-Fi/LwIP Init Phase
    tcpip_adapter_init();

    err = esp_event_loop_init(wifi_event_handler, NULL);
    if (err) {
        return err;
    }

    wifi_init_config_t wifi_init_cfg = WIFI_INIT_CONFIG_DEFAULT();
    err = esp_wifi_init(&wifi_init_cfg);
    if (err) {
        return err;
    }

    // Wi-Fi Configuration Phase
    err = esp_wifi_set_storage(WIFI_STORAGE_RAM);
    if (err) {
        return err;
    }
    err = esp_wifi_set_mode(WIFI_MODE_STA);
    if (err) {
        return err;
    }
    wifi_config_t wifi_cfg = {
        .sta = {
            .ssid = CONFIG_MY_TOOLBOX_WIFI_STA_SSID,
            .password = CONFIG_MY_TOOLBOX_WIFI_STA_PASSWORD
        }
    };
    err = esp_wifi_set_config(WIFI_IF_STA, &wifi_cfg);
    if (err) {
        return err;
    }

    // Wi-Fi Start Phase
    err = esp_wifi_start();
    if (err) {
        return err;
    }

    // Set hostname when the interface is ready (better in SYSTEM_EVENT_STA_START ?)
    err = tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_STA, hostname);
    if (err) {
        ESP_LOGW(TAG, "tcpip_adapter_set_hostname(): %s", esp_err_to_name(err));
    }

    return ESP_OK;
}
