#ifndef __WIFI_H_
#define __WIFI_H_

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/led_strip.h>
#include <zephyr/net/net_event.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/net_ip.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/websocket.h>
#include <zephyr/net/wifi_mgmt.h>
#include <zephyr/random/rand32.h>
#include <zephyr/shell/shell.h>

static K_SEM_DEFINE(wifi_connected, 0, 1);
static K_SEM_DEFINE(ipv4_address_obtained, 0, 1);

static struct net_mgmt_event_callback wifi_cb;
static struct net_mgmt_event_callback ipv4_cb;

class WIFI {
  const char *ssid;
  char *psk;

  void wifi_connect();
  void wifi_status();
  void wifi_disconnect();

public:
  WIFI(const char *_ssid, char *_psk);
  void connect();
};

#endif // __WIFI_H_
