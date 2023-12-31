/*
 * Copyright (c) 2023 Maximilian Huber
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __WIFI_H_
#define __WIFI_H_

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>
#include <zephyr/kernel.h>
#include <zephyr/net/net_event.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/net_ip.h>
#include <zephyr/net/wifi_mgmt.h>

static K_SEM_DEFINE(wifi_connected, 0, 1);
static K_SEM_DEFINE(ipv4_address_obtained, 0, 1);

static struct net_mgmt_event_callback wifi_cb;
static struct net_mgmt_event_callback ipv4_cb;

class WIFI
{
  const char *ssid;
  char *psk;

public:
  WIFI(const char *_ssid, char *_psk);
  void connect();
  void status();
  void disconnect();
};

#endif // __WIFI_H_
