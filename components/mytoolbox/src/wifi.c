#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_wifi.h"

static const char* TAG = "MY_WIFI";

static esp_err_t wifi_event_handler(void *ctx, system_event_t *event) {
    switch (event->event_id) {
        case SYSTEM_EVENT_STA_START:
            ESP_LOGI(TAG, "SYSTEM_EVENT_STA_START");
            ESP_ERROR_CHECK(esp_wifi_connect());
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            ESP_LOGI(TAG, "SYSTEM_EVENT_STA_GOT_IP");
            ESP_LOGI(TAG, "Got IP: %s\n", ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            ESP_LOGI(TAG, "SYSTEM_EVENT_STA_DISCONNECTED");
            ESP_ERROR_CHECK(esp_wifi_connect());
            break;
        default:
            break;
    }
    return ESP_OK;
}

void initialise_wifi_sta() {

    esp_log_level_set("wifi", ESP_LOG_WARN);

    // Wi-Fi/LwIP Init Phase
    tcpip_adapter_init();

    ESP_ERROR_CHECK(esp_event_loop_init(wifi_event_handler, NULL));

    wifi_init_config_t wifi_init_cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_cfg))

    // Wi-Fi Configuration Phase
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));    
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    wifi_config_t wifi_cfg = {
        .sta = {
            .ssid = CONFIG_MY_WIFI_STA_SSID,
            .password = CONFIG_MY_WIFI_STA_PASSWORD
        }
    };
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_cfg));

    // Wi-Fi Start Phase
    ESP_ERROR_CHECK(esp_wifi_start());
}