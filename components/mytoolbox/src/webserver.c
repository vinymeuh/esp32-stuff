#include "esp_http_server.h"
#include "esp_log.h"

static const char *TAG = "toolbox_webserver";

esp_err_t start_http_server(httpd_handle_t *server)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    esp_err_t err = httpd_start(server, &config);
    if (err) {
        return err;
    }
    ESP_LOGI(TAG, "httpd server started on port %d", config.server_port);
    return ESP_OK;
}
