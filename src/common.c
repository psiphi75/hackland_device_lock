#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

#include "esp_system.h"
#include "esp_log.h"

#include "common.h"

static const char *TAG = "COMMON";


char MY_UUID[MAC_LEN * 2 + 1] = "";


void to_hex_str(char *str, uint8_t *s, size_t len) {
    for(int i = 0; i < len; i++) {
        sprintf(&str[i * 2], "%02x", s[i]);
    }
}

void set_my_uuid() {
    uint8_t mac[MAC_LEN]; 
    esp_efuse_mac_get_default(mac);
    to_hex_str(MY_UUID, mac, MAC_LEN);
    ESP_LOGW(TAG, "MY_UUID: %s", MY_UUID);
}

char *get_uuid() {
    return MY_UUID;
}
