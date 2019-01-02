#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_system.h"
#include "esp_spi_flash.h"

#include "nvs_flash.h"

#include "mytoolbox/wifi.h"

void app_main()
{
    // initialize NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    // start Wifi
    initialise_wifi_sta();

    // ESP chip info
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);

    printf("Model       : %d\n", chip_info.model);
    printf("Revision    : %d\n", chip_info.revision);
    printf("CPU cores   : %d\n", chip_info.cores);
    printf("Wifi b/g/n  : %s\n", 
        (chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "yes" : "no");
    printf("Bluetooth   : %s\n",
        (chip_info.features & CHIP_FEATURE_BT) ? "yes" : "no");
    printf("Bluetooth LE: %s\n",
        (chip_info.features & CHIP_FEATURE_BLE) ? "yes" : "no");

    // flash size
    printf("Flash       : %dMB\n", spi_flash_get_chip_size() / (1024 * 1024));

    // infinite loop
    while(true) {
        vTaskDelay(500/portTICK_PERIOD_MS); // 500ms ?
    }
}
