#include <string.h>
#include <stdio.h>
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "protocol_examples_common.h"
#include "esp_log.h"
#include "esp_http_server.h"

#define TAG "MIN SERVER"

static esp_err_t on_url_hit(httpd_req_t *req)
{
    ESP_LOGI(TAG, "url %s was hit", req->uri);
    char *message = "hello world!";
    httpd_resp_send(req, message, strlen(message));
    return ESP_OK;
}

void app_main(void)
{
  ESP_ERROR_CHECK(nvs_flash_init());
  tcpip_adapter_init();
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  ESP_ERROR_CHECK(example_connect());

  httpd_handle_t server = NULL;
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();

  ESP_LOGI(TAG, "starting server");
  if (httpd_start(&server, &config) != ESP_OK)
  {
    ESP_LOGE(TAG, "COULD NOT START SERVER");
  }

  httpd_uri_t first_end_point_config = {
      .uri = "/",
      .method = HTTP_GET,
      .handler = on_url_hit};
  httpd_register_uri_handler(server, &first_end_point_config);
}
