#include <stdio.h>

#include "esp_random.h"

#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "cJSON.h"

#include "esp_log.h"

#include "wifi_connect.h"

#include "mqtt_connect.h"

#include "cJSON.h"

#include "dht11.h"
#include "oled.h"

static const char *TAG = "mqtt connection";

const TickType_t xDelay = SEND_CYCLE / portTICK_PERIOD_MS;

extern MQTT_Handler_Struct mqtt_h;
MQTT_Handler_Struct *mqtt_t;

uint64_t generateId(uint64_t v1, uint64_t v2)
{
    uint64_t id;
    id = (((uint64_t)v1) << 8) | (((uint64_t)v2) << 16);
    return id;
}

void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0)
    {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

void pub_data_clk(void *param)
{
    double temperature;
    double humidity;
    char str[50];
    while (1)
    {
        temperature = DHT11_read().temperature;
        humidity = DHT11_read().humidity;
        cJSON *root;
        root = cJSON_CreateObject();
        if (DHT11_read().status != DHT11_OK)
        {
            cJSON_AddNumberToObject(root, "uid", esp_random());
            cJSON_AddNumberToObject(root, "temperature", -1);
            cJSON_AddNumberToObject(root, "hump", -1);
            cJSON_AddNumberToObject(root, "error", 1);
        }
        else
        {
            cJSON_AddNumberToObject(root, "uid", esp_random());
            cJSON_AddNumberToObject(root, "temperature", temperature);
            cJSON_AddNumberToObject(root, "humidity", humidity);
            cJSON_AddNumberToObject(root, "error", 0);
        }

        sprintf(str, "Temp: %.1lf\nHumid: %.1lf", temperature, humidity);
        task_ssd1306_display_clear();
        task_ssd1306_display_text(str);
        memset(str, 0, sizeof(str));
        char *rendered = cJSON_Print(root);
        mqtt_client_publish(mqtt_t, TOPIC_PUB, rendered);
        vTaskDelay(xDelay);
    }
}

void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    mqtt_t = (MQTT_Handler_Struct *)handler_args;
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    // esp_mqtt_client_handle_t client = event->client;
    switch ((esp_mqtt_event_id_t)event_id)
    {
    case MQTT_EVENT_CONNECTED:
        xTaskCreate(&pub_data_clk, "DHT_task", 8192, NULL, 5, NULL);
        mqtt_client_publish(mqtt_t, CONNECT_PUB, "Esp32");
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;
    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT)
        {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno", event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}
bool mqtt_client_publish(MQTT_Handler_Struct *mqtt_t, char *topic, char *publish_string)
{
    if (mqtt_t->client)
    {
        int msg_id = esp_mqtt_client_publish(mqtt_t->client, topic, publish_string, 0, 1, 0);
        ESP_LOGI(TAG, "Sent publish returned msg_id=%d", msg_id);
        return true;
    }
    return false;
}

void mqtt_init_start(MQTT_Handler_Struct *mqtt_t)
{
    mqtt_t->client = esp_mqtt_client_init(mqtt_t->mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(mqtt_t->client, ESP_EVENT_ANY_ID, mqtt_event_handler, mqtt_t);
    esp_mqtt_client_start(mqtt_t->client);
}