#include <stdint.h>
#include <stddef.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_system.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "mqtt_client.h"

#include "common.h"
#include "lock.h"

#define DEVICE_TYPE ("LOCK")

static const char *TAG = "LOCK";

//
// Globals
// TODO: Put into a struct and pass it around
//
char topic_register[MAX_TOPIC_LEN];
char topic_unlock[MAX_TOPIC_LEN];


void device_unlock() {
    ESP_LOGI(TAG, "UNLOCK");
    gpio_set_level(GPIO_04, GPIO_OFF);
}

void device_lock() {
    ESP_LOGI(TAG, "LOCK");
    gpio_set_level(GPIO_04, GPIO_ON);
}

void v_lock_task( void * fp_sleep_time_secs ) {

    int sleep_time_secs = (int)fp_sleep_time_secs;

    if (sleep_time_secs <= 0 || sleep_time_secs >= TIME_24_HOURS_IN_SEC) {
        vTaskDelete(NULL);
        return;
    }

    device_unlock();
    ESP_LOGW(TAG, "Sleeping for %d seconds", sleep_time_secs);
    vTaskDelay((sleep_time_secs * 1000) / portTICK_PERIOD_MS);
    device_lock();

    // Delete self
    vTaskDelete(NULL);
}


void device_unlock_for_time(int sleep_time_secs) {
    TaskHandle_t xHandle = NULL;

    // Create a task that will stop after a certain time
    xTaskCreate( v_lock_task, "LOCKER_CODE", 2048, (void *)sleep_time_secs, tskIDLE_PRIORITY, &xHandle );
    configASSERT( xHandle );
}

void lock_handle_incoming_data(esp_mqtt_event_handle_t event) {
    if (strcmp(topic_unlock, event->topic) != 0) {
        ESP_LOGI(TAG, "Got unlock command.  Unlocking for (%.*s) seconds", event->data_len, event->data);

        #define MAX_TOPIC_DATA_LEN 6
        if (event->data_len > MAX_TOPIC_DATA_LEN) {
            ESP_LOGE(TAG, "Invalid data length: %d.  Rebooting", event->data_len);
            esp_restart();
        }

        char sleep_time_secs_str[MAX_TOPIC_DATA_LEN + 1];

        snprintf(sleep_time_secs_str, event->data_len + 1, "%s", event->data);
        int sleep_time_secs = atoi(sleep_time_secs_str);
        device_unlock_for_time(sleep_time_secs);   
    }
}

char *lock_get_topic_register() {
    return topic_register;
}

char *lock_get_topic_unlock() {
    return topic_unlock;
}

const char *lock_get_device_type() {
    return DEVICE_TYPE;
}

void lock_init() {
    // Get the MAC address, from that we can create the topic sub/pubs
    set_my_uuid();
    sprintf(topic_register, BASE_TOPIC "/%s/register", get_uuid());
    sprintf(topic_unlock,   BASE_TOPIC "/%s/unlock", get_uuid());

    device_lock(); // By defaut we are locked

    //configure GPIO with the given settings
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);
}