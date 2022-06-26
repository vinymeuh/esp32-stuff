#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_system.h"
//#include "esp_spi_flash.h"
//#include "nvs_flash.h"

//#include "mytoolbox/wifi.h"
#include "dht.h"

#define DHT_PIN 23
#define MAIN_LOOP_PERIOD_S 60

static const char* TAG = "sensor-dht";

void app_main()
{
    dht_measure_t measure;
    while(true) {
        dht_err_t err = dht_read(DHT_PIN, &measure);
        switch (err) {
            case DHT_OK :
                ESP_LOGI(TAG, "humidity=%0.1f, temperature=%0.1f", measure.humidity, measure.temperature);
                break;
            case DHT_ERR_INVALID_PIN:
                ESP_LOGI(TAG, "DHT_ERR_INVALID_PIN");
                break;
            case DHT_ERR_RESPONSE_TIMEOUT:
                ESP_LOGI(TAG, "DHT_ERR_RESPONSE_TIMEOUT");
                break;
            case DHT_ERR_GETREADY_TIMEOUT:
                ESP_LOGI(TAG, "DHT_ERR_GETREADY_TIMEOUT");
                break;
            case DHT_ERR_START_READ_BIT_TIMEOUT:
                ESP_LOGI(TAG, "DHT_ERR_START_READ_BIT_TIMEOUT");
                break;
            case DHT_ERR_READ_BIT_TIMEOUT:
                ESP_LOGI(TAG, "DHT_ERR_READ_BIT_TIMEOUT");
                break;
            case DHT_ERR_CHECKSUM:
                ESP_LOGI(TAG, "DHT_ERR_CHECKSUM");
                break;
            default:
                ESP_LOGE(TAG, "unknown error !");
        }
        vTaskDelay(MAIN_LOOP_PERIOD_S * 1000 / portTICK_PERIOD_MS);
    }
}
