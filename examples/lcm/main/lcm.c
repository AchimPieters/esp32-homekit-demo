#include <stdio.h>
#include <string.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <esp_http_server.h>
#include "lcm.h"

static const char *TAG = "LCM";

void handle_error(esp_err_t err) {
    if (err == ESP_ERR_WIFI_NOT_STARTED || err == ESP_ERR_WIFI_CONN) {
        ESP_LOGI(TAG, "Restarting WiFi...");
        esp_wifi_stop();
        esp_wifi_start();
    } else {
        ESP_LOGE(TAG, "Critical error, restarting device...");
        esp_restart();
    }
}

void on_wifi_ready() {
    ESP_LOGI(TAG, "WiFi is ready for use.");
}

void start_ap_mode(const char *ssid, const char *password) {
    static bool ap_initialized = false;

    if (!ap_initialized) {
        esp_netif_t *ap_netif = esp_netif_create_default_wifi_ap();
        if (!ap_netif) {
            ESP_LOGE(TAG, "Failed to create default AP netif");
            return;
        }

        wifi_config_t wifi_config = {
            .ap = {
                .ssid = "",
                .ssid_len = strlen(ssid),
                .channel = 1,
                .password = "",
                .max_connection = 4,
                .authmode = WIFI_AUTH_WPA_WPA2_PSK
            }
        };

        strncpy((char *)wifi_config.ap.ssid, ssid, sizeof(wifi_config.ap.ssid));
        strncpy((char *)wifi_config.ap.password, password, sizeof(wifi_config.ap.password));

        if (strlen(password) == 0) {
            wifi_config.ap.authmode = WIFI_AUTH_OPEN;
        }

        esp_wifi_set_mode(WIFI_MODE_AP);
        esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config);
        esp_wifi_start();

        ESP_LOGI(TAG, "AP started with SSID: %s, Password: %s", ssid, password);
        ap_initialized = true;
    } else {
        ESP_LOGW(TAG, "AP mode already initialized");
    }
}

static esp_err_t index_get_handler(httpd_req_t *req) {
    const char *resp = "<html><body><form action='/set_credentials' method='POST'>"
                       "SSID: <input type='text' name='ssid'><br>"
                       "Password: <input type='password' name='password'><br>"
                       "<input type='submit' value='Submit'>"
                       "</form></body></html>";
    httpd_resp_send(req, resp, strlen(resp));
    return ESP_OK;
}

static esp_err_t set_credentials_handler(httpd_req_t *req) {
    char buf[128];
    int len = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (len <= 0) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    buf[len] = '\0';
    char ssid[32] = {0}, password[64] = {0};
    sscanf(buf, "ssid=%31[^&]&password=%63s", ssid, password);

    ESP_LOGI(TAG, "Received credentials: SSID=%s, Password=%s", ssid, password);

    httpd_resp_send(req, "Credentials set. Rebooting to apply settings.", HTTPD_RESP_USE_STRLEN);
    esp_restart();
    return ESP_OK;
}

void start_web_server() {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;

    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_uri_t index_uri = {
            .uri = "/",
            .method = HTTP_GET,
            .handler = index_get_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &index_uri);

        httpd_uri_t set_credentials_uri = {
            .uri = "/set_credentials",
            .method = HTTP_POST,
            .handler = set_credentials_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &set_credentials_uri);

        ESP_LOGI(TAG, "Web server started.");
    } else {
        ESP_LOGE(TAG, "Failed to start web server.");
    }
}

void wifi_init() {
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_wifi_set_storage(WIFI_STORAGE_RAM);
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_start();
}

void app_main(void) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS flash initialization failed, erasing...");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    start_ap_mode("HomeKitSetup", "password");
    start_web_server();
}
