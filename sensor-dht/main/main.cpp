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

extern "C" {
   void app_main();
}

void app_main()
{
    dht::Sensor sensor{gpio_num_t(DHT_PIN)};
    dht::Measure measure;

    /* 
     * As communication with DHT is based on timings, we need to disable 
     * the task switching while collecting measure
     */
    portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

    while(true) {
        portENTER_CRITICAL(&mux);
        measure = sensor.collect();
        portEXIT_CRITICAL(&mux);

        switch (measure.error) {
            case dht::Error::None:
                ESP_LOGI(TAG, "humidity=%0.1f, temperature=%0.1f", measure.humidity, measure.temperature);
                break;
            case dht::Error::Checksum:
                ESP_LOGI(TAG, "humidity=%0.1f, temperature=%0.1f (Checksum error)", measure.humidity, measure.temperature);
                break;
            default:
                ESP_LOGI(TAG, "timeout (error=%d)", measure.error);
                break;
        }
        vTaskDelay(MAIN_LOOP_PERIOD_S * 1000 / portTICK_PERIOD_MS);
    }
}
