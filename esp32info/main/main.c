#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "nvs_flash.h"
#include "soc/efuse_reg.h"

#include "esp_http_server.h"
#include "cJSON.h"

#include "mytoolbox/wifi.h"
#include "mytoolbox/webserver.h"


// see esp_system.h for chip models list
const char *chip_model(int id)
{
    switch(id) {
        case 1: return "ESP32";
    };
    return "unknown";
};

// see soc/efuse_reg.h for packages list
const char *package_description(int id)
{
    switch(id) {
        case EFUSE_RD_CHIP_VER_PKG_ESP32D0WDQ6: return "ESP32D0WDQ6";
        case EFUSE_RD_CHIP_VER_PKG_ESP32D0WDQ5: return "ESP32D0WDQ5";
        case EFUSE_RD_CHIP_VER_PKG_ESP32D2WDQ5: return "ESP32D2WDQ5";
        case EFUSE_RD_CHIP_VER_PKG_ESP32PICOD2: return "ESP32PICOD2";
        case EFUSE_RD_CHIP_VER_PKG_ESP32PICOD4: return "ESP32PICOD4";
    };
    return "unknown";
}

esp_err_t get_hwinfo(httpd_req_t *req)
{
    cJSON *root = NULL;
    root = cJSON_CreateObject();
    
    // chip package is not available in esp_chip_info_t
    // see get_chip_info_esp32() in esp2/system_api.c
    uint32_t reg = REG_READ(EFUSE_BLK0_RDATA3_REG);
    int package = (reg & EFUSE_RD_CHIP_VER_PKG_M) >> EFUSE_RD_CHIP_VER_PKG_S;

    // chip info
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);

    cJSON *chip = NULL;
    cJSON_AddItemToObject(root, "chip", chip = cJSON_CreateObject());
    cJSON_AddStringToObject(chip, "model", chip_model(chip_info.model));
    cJSON_AddStringToObject(chip, "package", package_description(package));
    cJSON_AddNumberToObject(chip, "revision", chip_info.revision);
    cJSON_AddNumberToObject(chip, "cores", chip_info.cores);
    
    cJSON *chip_features = NULL;
    cJSON_AddItemToObject(chip, "features", chip_features = cJSON_CreateObject());
    cJSON_AddStringToObject(chip_features, "embedded flash", (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "yes" : "no");
    cJSON_AddStringToObject(chip_features, "wifi b/g/n", (chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "yes" : "no");
    cJSON_AddStringToObject(chip_features, "bluetooth", (chip_info.features & CHIP_FEATURE_BT) ? "yes" : "no");
    cJSON_AddStringToObject(chip_features, "bluetooth LE", (chip_info.features & CHIP_FEATURE_BLE) ? "yes" : "no");

    // flash
    cJSON *flash = NULL;
    cJSON_AddItemToObject(root, "flash", flash = cJSON_CreateObject());
    cJSON_AddNumberToObject(flash, "size", spi_flash_get_chip_size());

    // esp-idf
    cJSON *espidf = NULL;
    cJSON_AddItemToObject(root, "esp-idf", espidf = cJSON_CreateObject());
    cJSON_AddStringToObject(espidf, "version", esp_get_idf_version());

    // returns the JSON document
    char* resp = cJSON_Print(root);
    httpd_resp_set_type(req, "application/json");
    return httpd_resp_send(req, resp, strlen(resp));
}

// routes
const httpd_uri_t uri_get = {
    .uri  = "/hwinfo",
    .method = HTTP_GET,
    .handler = get_hwinfo,
    .user_ctx = NULL,
};

void app_main()
{
    esp_err_t err;

    // initialise NVS
    err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    // start Wifi
    ESP_ERROR_CHECK(initialise_wifi_sta("esp32-mdns"));

    // start webserver
    httpd_handle_t httpd;
    ESP_ERROR_CHECK(start_http_server(&httpd));
    ESP_ERROR_CHECK(httpd_register_uri_handler(httpd, &uri_get));

    // infinite loop
    while(true) {
        vTaskDelay(500/portTICK_PERIOD_MS); // 500ms ?
    }
}
