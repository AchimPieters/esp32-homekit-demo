#ifndef LCM_H
#define LCM_H

#include <esp_err.h>
#include <esp_event.h>
#include <esp_wifi.h>
#include <esp_log.h>

// Custom error handling macro
#define CHECK_ERROR(x) do {                        \
                esp_err_t __err_rc = (x);                  \
                if (__err_rc != ESP_OK) {                  \
                        ESP_LOGE("INFORMATION", "Error: %s", esp_err_to_name(__err_rc)); \
                        handle_error(__err_rc);                \
                }                                          \
} while(0)

// Function prototypes
void handle_error(esp_err_t err);
void on_wifi_ready();
void wifi_init();
void start_ap_mode(const char *ssid, const char *password);
void start_web_server();

#endif // LCM_H
