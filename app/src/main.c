/*
 * Copyright (c) 2023 Maximilian Huber
 * Copyright (c) 2023 Craig Peacock.
 * Copyright (c) 2019 Intel Corporation
 * Copyright (c) 2017 ARM Ltd.
 * Copyright (c) 2016 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(app, LOG_LEVEL_DBG);

#include <errno.h>
#include <zephyr/kernel.h>
#include <zephyr/net/net_event.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/net_ip.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/websocket.h>
#include <zephyr/net/wifi_mgmt.h>
#include <zephyr/random/rand32.h>
#include <zephyr/shell/shell.h>

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

static K_SEM_DEFINE(wifi_connected, 0, 1);
static K_SEM_DEFINE(ipv4_address_obtained, 0, 1);

static struct net_mgmt_event_callback wifi_cb;
static struct net_mgmt_event_callback ipv4_cb;

static void handle_wifi_connect_result(struct net_mgmt_event_callback *cb)
{
	const struct wifi_status *status = (const struct wifi_status *)cb->info;

	if (status->status)
	{
		LOG_ERR("Connection request failed (%d)\n", status->status);
	}
	else
	{
		LOG_INF("Connected\n");
		k_sem_give(&wifi_connected);
	}
}

static void handle_wifi_disconnect_result(struct net_mgmt_event_callback *cb)
{
	const struct wifi_status *status = (const struct wifi_status *)cb->info;

	if (status->status)
	{
		LOG_INF("Disconnection request (%d)\n", status->status);
	}
	else
	{
		LOG_INF("Disconnected\n");
		k_sem_take(&wifi_connected, K_NO_WAIT);
	}
}

static void handle_ipv4_result(struct net_if *iface)
{
	int i = 0;

	for (i = 0; i < NET_IF_MAX_IPV4_ADDR; i++)
	{

		char buf[NET_IPV4_ADDR_LEN];

		if (iface->config.ip.ipv4->unicast[i].addr_type != NET_ADDR_DHCP)
		{
			continue;
		}

		LOG_DBG("IPv4 address: %s\n",
				net_addr_ntop(AF_INET,
							  &iface->config.ip.ipv4->unicast[i].address.in_addr,
							  buf, sizeof(buf)));
		LOG_DBG("Subnet: %s\n",
				net_addr_ntop(AF_INET,
							  &iface->config.ip.ipv4->netmask,
							  buf, sizeof(buf)));
		LOG_DBG("Router: %s\n",
				net_addr_ntop(AF_INET,
							  &iface->config.ip.ipv4->gw,
							  buf, sizeof(buf)));
	}

	k_sem_give(&ipv4_address_obtained);
}

static void wifi_mgmt_event_handler(struct net_mgmt_event_callback *cb, uint32_t mgmt_event, struct net_if *iface)
{
	switch (mgmt_event)
	{

	case NET_EVENT_WIFI_CONNECT_RESULT:
		handle_wifi_connect_result(cb);
		break;

	case NET_EVENT_WIFI_DISCONNECT_RESULT:
		handle_wifi_disconnect_result(cb);
		break;

	case NET_EVENT_IPV4_ADDR_ADD:
		handle_ipv4_result(iface);
		break;

	default:
		break;
	}
}

void wifi_connect(void)
{
	struct net_if *iface = net_if_get_default();

	struct wifi_connect_req_params wifi_params = {0};

	wifi_params.ssid = SSID;
	wifi_params.psk = PSK;
	wifi_params.ssid_length = strlen(SSID);
	wifi_params.psk_length = strlen(PSK);
	wifi_params.channel = WIFI_CHANNEL_ANY;
	wifi_params.security = WIFI_SECURITY_TYPE_PSK;
	wifi_params.band = WIFI_FREQ_BAND_2_4_GHZ;
	wifi_params.mfp = WIFI_MFP_OPTIONAL;

	LOG_DBG("Connecting to SSID: %s\n", wifi_params.ssid);

	if (net_mgmt(NET_REQUEST_WIFI_CONNECT, iface, &wifi_params, sizeof(struct wifi_connect_req_params)))
	{
		LOG_ERR("WiFi Connection Request Failed\n");
	}
}

void wifi_status(void)
{
	struct net_if *iface = net_if_get_default();

	struct wifi_iface_status status = {0};

	if (net_mgmt(NET_REQUEST_WIFI_IFACE_STATUS, iface, &status, sizeof(struct wifi_iface_status)))
	{
		LOG_ERR("WiFi Status Request Failed\n");
	}

	if (status.state >= WIFI_STATE_ASSOCIATED)
	{
		LOG_DBG("SSID: %-32s\n", status.ssid);
		LOG_DBG("Band: %s\n", wifi_band_txt(status.band));
		LOG_DBG("Channel: %d\n", status.channel);
		LOG_DBG("Security: %s\n", wifi_security_txt(status.security));
		LOG_DBG("RSSI: %d\n", status.rssi);
	}
}

void wifi_disconnect(void)
{
	struct net_if *iface = net_if_get_default();

	if (net_mgmt(NET_REQUEST_WIFI_DISCONNECT, iface, NULL, 0))
	{
		LOG_ERR("WiFi Disconnection Request Failed\n");
	}
}

void main(void)
{
	int sock;

	LOG_INF("Board: %s\n", CONFIG_BOARD);

	net_mgmt_init_event_callback(&wifi_cb, wifi_mgmt_event_handler,
								 NET_EVENT_WIFI_CONNECT_RESULT | NET_EVENT_WIFI_DISCONNECT_RESULT);

	net_mgmt_init_event_callback(&ipv4_cb, wifi_mgmt_event_handler, NET_EVENT_IPV4_ADDR_ADD);

	net_mgmt_add_event_callback(&wifi_cb);
	net_mgmt_add_event_callback(&ipv4_cb);

#ifdef CONFIG_BOARD_NRF7002DK_NRF5340_CPUAPP
	// Delay to prevent "Unable to get wpa_s handle for wlan0" on nRF Connect SDK 2.3.0.?
	LOG_DBG("Sleeping for 1 second while wlan0 comes up\n");
	k_sleep(K_SECONDS(1));
#else
	LOG_DBG("Sleeping for 1 second anyways\n");
	k_sleep(K_SECONDS(1));
#endif

	wifi_connect();
	k_sem_take(&wifi_connected, K_FOREVER);
	wifi_status();
	k_sem_take(&ipv4_address_obtained, K_FOREVER);
	LOG_INF("Ready...");
}