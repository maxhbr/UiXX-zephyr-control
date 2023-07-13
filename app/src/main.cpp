/*
 * Copyright (c) 2023 Maximilian Huber
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/arch/cpu.h>
#include <zephyr/sys/printk.h>

#include "wifi.h"

#if defined(CONFIG_NET_CONFIG_SSID)
#define SSID CONFIG_NET_CONFIG_SSID
#else
#define SSID "SSID"
#endif

#if defined(CONFIG_NET_CONFIG_PSK)
#define PSK CONFIG_NET_CONFIG_PSK
#else
#define PSK "PSK"
#endif

int main(void)
{
	printk("Hello World! %s\n", CONFIG_BOARD);

    WIFI wifi(SSID, PSK);
    wifi.connect();


	return 0;
}

