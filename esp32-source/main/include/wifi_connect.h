/*
 * wifi_app.h
 */

#ifndef MAIN_WIFI_APP_H_
#define MAIN_WIFI_APP_H_
#include "esp_event.h"
#include "esp_wifi.h"
#define EXAMPLE_ESP_MAXIMUM_RETRY 5

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

#define WIFI_SSID "bambobobu"
#define WIFI_PASS "22101993"

void event_handler(void *, esp_event_base_t, int32_t, void *);
void wifi_init_start(void);
void wifi_stop_deinit(void);
#endif /* MAIN_WIFI_APP_H_ */