#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "soc/efuse_reg.h"

#include "esp_http_server.h"

#include "mytoolbox/wifi.h"
#include "mytoolbox/webserver.h"

#define HOSTNAME "esp32info"


esp_err_t get_info(httpd_req_t* req);

// routes
const httpd_uri_t uri_get = {
    .uri  = "/",
    .method = HTTP_GET,
    .handler = get_info,
    .user_ctx = NULL,
};

void app_main()
{
    // initialise NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    // start Wifi
    ESP_ERROR_CHECK(initialise_wifi_sta(HOSTNAME));

    // start webserver
    httpd_handle_t httpd;
    ESP_ERROR_CHECK(start_http_server(&httpd));
    ESP_ERROR_CHECK(httpd_register_uri_handler(httpd, &uri_get));

    // infinite loop
    while(true) {
        vTaskDelay(60000/portTICK_PERIOD_MS);
    }
}
