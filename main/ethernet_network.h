#pragma once

#include <stdbool.h>
#include <stddef.h>

void ethernet_network_init(void);
bool ethernet_network_is_connected(void);
void ethernet_network_get_ip(char *ip, size_t size);
