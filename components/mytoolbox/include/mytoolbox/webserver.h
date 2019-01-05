#ifndef __MY_TOOLBOX_WEBSERVER_H__
#define __MY_TOOLBOX_WEBSERVER_H__

#include "esp_http_server.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t start_http_server(httpd_handle_t *server);

#ifdef __cplusplus
}
#endif

#endif