#include "hw_config.h"
#include "esp_mac.h"
#include <stdio.h>

esp_err_t hw_config_get_device_id(char *uid)
{
    uint8_t derived_mac_addr[6];
    
#if HARDWARE_VER == OLIMEX_ESP32_EVB_REVK
    esp_read_mac(derived_mac_addr, ESP_MAC_ETH);
#else
    esp_read_mac(derived_mac_addr, ESP_MAC_WIFI_SOFTAP);
#endif

    sprintf(uid, "%02x%02x%02x%02x%02x%02x",
            derived_mac_addr[0], derived_mac_addr[1], derived_mac_addr[2],
            derived_mac_addr[3], derived_mac_addr[4], derived_mac_addr[5]);

    return ESP_OK;
}
