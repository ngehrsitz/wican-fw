 /* This file is part of the WiCAN project.
 *
 * Copyright (C) 2022  Meatpi Electronics.
 * Written by Ali Slim <ali@meatpi.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef GW_CONFIG_h
#define GW_CONFIG_h

#include "esp_err.h"
#include "driver/gpio.h"

#if HARDWARE_VER == OLIMEX_ESP32_EVB_REVK
#define FS_MOUNT_POINT              "/littlefs"
#define TX_GPIO_NUM                 5
#define RX_GPIO_NUM                 35
// SIT1050T standby is tied low on the board; there is no control GPIO.
#define CAN_STDBY_GPIO_NUM          (-1)
#define CONNECTED_LED_GPIO_NUM      (-1)
#define ACTIVE_LED_GPIO_NUM         (-1)
#define BLE_EN_PIN_NUM              (-1)
#define PWR_LED_GPIO_NUM            (-1)
#define ETH_PHY_ADDR                0
#define ETH_MDC_GPIO_NUM            23
#define ETH_MDIO_GPIO_NUM           18
#define ETH_PHY_RESET_GPIO_NUM      (-1)
#define RELAY1_GPIO_NUM             32
#define RELAY2_GPIO_NUM             33

#elif HARDWARE_VER == WICAN_PRO
#define FS_MOUNT_POINT              "/fatfs"
#define TX_GPIO_NUM             	2
#define RX_GPIO_NUM             	1
#define CAN_STDBY_GPIO_NUM			38

#define SDCARD_CLK                  21                 
#define SDCARD_CMD                  47
#define SDCARD_D0                   14
#define SDCARD_D1                   13
#define SDCARD_D2                   12
#define SDCARD_D3                   48

#define OBD_RESET_PIN           (GPIO_NUM_41)
#define OBD_LED_EN_PIN          (GPIO_NUM_42)
#define OBD_READY_PIN           (GPIO_NUM_7)    // High = Active, Low = Sleep
// #define CONNECTED_LED_GPIO_NUM		41  //NC pin
// #define ACTIVE_LED_GPIO_NUM			41  //NC pin
// #define BLE_EN_PIN_NUM				42  //NC pin
// #define PWR_LED_GPIO_NUM			41  //NC pin

#define BUTTON_GPIO_NUM			    8
#define IMU_INT_GPIO_NUM			3

#else

#define FS_MOUNT_POINT              "/littlefs"
#define TX_GPIO_NUM             	0
#define RX_GPIO_NUM             	3
#define CONNECTED_LED_GPIO_NUM		8
#define ACTIVE_LED_GPIO_NUM			9
#define BLE_EN_PIN_NUM				5
#define PWR_LED_GPIO_NUM			7
#define CAN_STDBY_GPIO_NUM			6

#endif

// Existing interfaces store LED pins as uint8_t: absent -1 becomes 255.
// Both representations are rejected before accessing or shifting a GPIO.
static inline uint64_t hw_gpio_output_mask(int pin)
{
    return pin >= 0 && GPIO_IS_VALID_OUTPUT_GPIO(pin) ? (1ULL << pin) : 0;
}

static inline void hw_gpio_set_level(int pin, int level)
{
    if(hw_gpio_output_mask(pin))
    {
        gpio_set_level((gpio_num_t)pin, level);
    }
}

esp_err_t hw_config_get_device_id(char *uid);

#endif
