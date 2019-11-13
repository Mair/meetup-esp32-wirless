#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_event_loop.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_http_server.h"

#define SSID CONFIG_WIFI_SSID
#define PASSWORD CONFIG_WIFI_PASSWORD

xSemaphoreHandle onConnectionHandler;

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
  switch (event->event_id)
  {
  case SYSTEM_EVENT_STA_START:
    esp_wifi_connect();
    printf("connecting...\n");
    break;

  case SYSTEM_EVENT_STA_CONNECTED:
    printf("connected\n");
    break;

  case SYSTEM_EVENT_STA_GOT_IP:
    printf("got ip\n");

    break;

  case SYSTEM_EVENT_STA_DISCONNECTED:
    printf("disconnected\n");
    break;

  default:
    break;
  }
  return ESP_OK;
}

void wifiInit()
{
  ESP_ERROR_CHECK(nvs_flash_init());
  tcpip_adapter_init();
  ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));

  wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  wifi_config_t wifi_config =
      {
          .sta = {
              .ssid = CONFIG_WIFI_SSID,
              .password = CONFIG_WIFI_PASSWORD}};
  esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);

  // ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
  // wifi_config_t wifi_config =
  //     {
  //         .ap = {
  //             .ssid = "my esp32",
  //             .password = "P@ssword",
  //             .max_connection = 4,
  //             .authmode = WIFI_AUTH_WPA_WPA2_PSK}};
  // esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config);

  ESP_ERROR_CHECK(esp_wifi_start());
  xSemaphoreGive(onConnectionHandler);
}

static esp_err_t on_url_hit(httpd_req_t *req)
{
  printf("url %s was hit", req->uri);
  char *message = "hello world!";
  httpd_resp_send(req, message, strlen(message));
  return ESP_OK;
}

void onConnected(void *param)
{
  while (true)
  {
    if (xSemaphoreTake(onConnectionHandler, 10 * 1000 / portTICK_PERIOD_MS))
    {
      httpd_handle_t server = NULL;
      httpd_config_t config = HTTPD_DEFAULT_CONFIG();
      httpd_start(&server, &config);
      httpd_uri_t first_end_point_config = {
          .uri = "/",
          .method = HTTP_GET,
          .handler = on_url_hit};
      httpd_register_uri_handler(server, &first_end_point_config);
    }
  }
}

void app_main()
{
  onConnectionHandler = xSemaphoreCreateBinary();
  wifiInit();
  xTaskCreate(&onConnected, "On Connected", 1024 * 4, NULL, 5, NULL);
}