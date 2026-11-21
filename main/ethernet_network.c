#include "ethernet_network.h"
#include "hw_config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_eth.h"
#include "esp_eth_mac_esp.h"
#include "esp_eth_netif_glue.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_netif.h"
#include <stdio.h>

#define ETH_IP_READY_BIT BIT0
static const char *TAG = "ethernet_network";
static EventGroupHandle_t eth_events;
static esp_netif_t *eth_netif;
static esp_eth_handle_t eth_handle;

static void ethernet_event_handler(void *arg, esp_event_base_t base,
                                   int32_t event_id, void *event_data)
{
    if(base == ETH_EVENT)
    {
        if(event_id == ETHERNET_EVENT_DISCONNECTED || event_id == ETHERNET_EVENT_STOP)
        {
            xEventGroupClearBits(eth_events, ETH_IP_READY_BIT);
            ESP_LOGI(TAG, "Ethernet disconnected");
        }
    }
    else if(base == IP_EVENT && event_id == IP_EVENT_ETH_GOT_IP)
    {
        ip_event_got_ip_t *event = event_data;
        if(event->esp_netif == eth_netif && event->ip_info.ip.addr != 0)
        {
            xEventGroupSetBits(eth_events, ETH_IP_READY_BIT);
            ESP_LOGI(TAG, "Ethernet IP: " IPSTR, IP2STR(&event->ip_info.ip));
        }
    }
    else if(base == IP_EVENT && event_id == IP_EVENT_ETH_LOST_IP)
    {
        xEventGroupClearBits(eth_events, ETH_IP_READY_BIT);
    }
}

void ethernet_network_init(void)
{
    if(eth_handle)
    {
        return;
    }
    eth_events = xEventGroupCreate();
    ESP_ERROR_CHECK(eth_events ? ESP_OK : ESP_ERR_NO_MEM);

    esp_netif_config_t netif_config = ESP_NETIF_DEFAULT_ETH();
    eth_netif = esp_netif_new(&netif_config);
    ESP_ERROR_CHECK(eth_netif ? ESP_OK : ESP_ERR_NO_MEM);

    uint8_t mac_addr[6];
    ESP_ERROR_CHECK(esp_read_mac(mac_addr, ESP_MAC_ETH));
    char hostname[24];
    snprintf(hostname, sizeof(hostname), "wican_%02x%02x%02x%02x%02x%02x",
             mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
    ESP_ERROR_CHECK(esp_netif_set_hostname(eth_netif, hostname));

    // Rev K has an external 50 MHz clock on GPIO0 and no PHY reset/power GPIO.
    // Wait for the board's hardware PHY startup before probing it over MDIO.
    vTaskDelay(pdMS_TO_TICKS(2000));
    eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
    eth_esp32_emac_config_t emac_config = ETH_ESP32_EMAC_DEFAULT_CONFIG();
    emac_config.smi_gpio.mdc_num = ETH_MDC_GPIO_NUM;
    emac_config.smi_gpio.mdio_num = ETH_MDIO_GPIO_NUM;
    emac_config.clock_config.rmii.clock_mode = EMAC_CLK_EXT_IN;
    emac_config.clock_config.rmii.clock_gpio = EMAC_CLK_IN_GPIO;
    esp_eth_mac_t *mac = esp_eth_mac_new_esp32(&emac_config, &mac_config);

    eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();
    phy_config.phy_addr = ETH_PHY_ADDR;
    phy_config.reset_gpio_num = ETH_PHY_RESET_GPIO_NUM;
    esp_eth_phy_t *phy = esp_eth_phy_new_lan87xx(&phy_config);
    ESP_ERROR_CHECK(mac && phy ? ESP_OK : ESP_ERR_NO_MEM);

    esp_eth_config_t eth_config = ETH_DEFAULT_CONFIG(mac, phy);
    ESP_ERROR_CHECK(esp_eth_driver_install(&eth_config, &eth_handle));
    ESP_ERROR_CHECK(esp_eth_ioctl(eth_handle, ETH_CMD_S_MAC_ADDR, mac_addr));
    esp_eth_netif_glue_handle_t glue = esp_eth_new_netif_glue(eth_handle);
    ESP_ERROR_CHECK(glue ? ESP_OK : ESP_ERR_NO_MEM);
    ESP_ERROR_CHECK(esp_netif_attach(eth_netif, glue));
    ESP_ERROR_CHECK(esp_event_handler_register(ETH_EVENT, ESP_EVENT_ANY_ID,
                                             ethernet_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP,
                                             ethernet_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_LOST_IP,
                                             ethernet_event_handler, NULL));
    ESP_ERROR_CHECK(esp_eth_start(eth_handle));
}

bool ethernet_network_is_connected(void)
{
    return eth_events && (xEventGroupGetBits(eth_events) & ETH_IP_READY_BIT);
}

void ethernet_network_get_ip(char *ip, size_t size)
{
    esp_netif_ip_info_t info;
    if(ethernet_network_is_connected() && esp_netif_get_ip_info(eth_netif, &info) == ESP_OK)
    {
        snprintf(ip, size, IPSTR, IP2STR(&info.ip));
    }
    else
    {
        snprintf(ip, size, "0.0.0.0");
    }
}
