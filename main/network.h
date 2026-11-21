#pragma once

#include <stdbool.h>
#include <stddef.h>

// Shared callers use the selected backend; wifi_network_* remains Wi-Fi-only.
void network_init(void);
bool network_is_connected(void);
bool network_is_ethernet(void);
void network_get_ip(char *ip, size_t size);
