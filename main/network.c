#include "network.h"
#include "hw_config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "wifi_network.h"
#include "config_server.h"
#include <stdio.h>

#if HARDWARE_VER == OLIMEX_ESP32_EVB_REVK
#include "ethernet_network.h"
#endif

void network_init(void)
{
#if HARDWARE_VER == OLIMEX_ESP32_EVB_REVK
    ethernet_network_init();
#else
    wifi_network_init(NULL, NULL);
#endif
}

bool network_is_connected(void)
{
#if HARDWARE_VER == OLIMEX_ESP32_EVB_REVK
    return ethernet_network_is_connected();
#else
    return wifi_network_is_connected();
#endif
}

bool network_is_ethernet(void)
{
    return HARDWARE_VER == OLIMEX_ESP32_EVB_REVK;
}

void network_get_ip(char *ip, size_t size)
{
    if(!ip || !size)
    {
        return;
    }
#if HARDWARE_VER == OLIMEX_ESP32_EVB_REVK
    ethernet_network_get_ip(ip, size);
#else
    char sta_ip[20] = {0};
    config_server_get_sta_ip(sta_ip);
    snprintf(ip, size, "%s", sta_ip);
#endif
}
