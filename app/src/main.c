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
        printk("Connection request failed (%d)\n", status->status);
    }
    else
    {
        printk("Connected\n");
        k_sem_give(&wifi_connected);
    }
}

static void handle_wifi_disconnect_result(struct net_mgmt_event_callback *cb)
{
    const struct wifi_status *status = (const struct wifi_status *)cb->info;

    if (status->status)
    {
        printk("Disconnection request (%d)\n", status->status);
    }
    else
    {
        printk("Disconnected\n");
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

        printk("IPv4 address: %s\n",
               net_addr_ntop(AF_INET,
                             &iface->config.ip.ipv4->unicast[i].address.in_addr,
                             buf, sizeof(buf)));
        printk("Subnet: %s\n",
               net_addr_ntop(AF_INET,
                             &iface->config.ip.ipv4->netmask,
                             buf, sizeof(buf)));
        printk("Router: %s\n",
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

    printk("Connecting to SSID: %s\n", wifi_params.ssid);

    if (net_mgmt(NET_REQUEST_WIFI_CONNECT, iface, &wifi_params, sizeof(struct wifi_connect_req_params)))
    {
        printk("WiFi Connection Request Failed\n");
    }
}

void wifi_status(void)
{
    struct net_if *iface = net_if_get_default();

    struct wifi_iface_status status = {0};

    if (net_mgmt(NET_REQUEST_WIFI_IFACE_STATUS, iface, &status, sizeof(struct wifi_iface_status)))
    {
        printk("WiFi Status Request Failed\n");
    }

    printk("\n");

    if (status.state >= WIFI_STATE_ASSOCIATED)
    {
        printk("SSID: %-32s\n", status.ssid);
        printk("Band: %s\n", wifi_band_txt(status.band));
        printk("Channel: %d\n", status.channel);
        printk("Security: %s\n", wifi_security_txt(status.security));
        printk("RSSI: %d\n", status.rssi);
    }
}

void wifi_disconnect(void)
{
    struct net_if *iface = net_if_get_default();

    if (net_mgmt(NET_REQUEST_WIFI_DISCONNECT, iface, NULL, 0))
    {
        printk("WiFi Disconnection Request Failed\n");
    }
}

void main(void)
{
    int sock;

    printk("WiFi Example\nBoard: %s\n", CONFIG_BOARD);

    net_mgmt_init_event_callback(&wifi_cb, wifi_mgmt_event_handler,
                                 NET_EVENT_WIFI_CONNECT_RESULT | NET_EVENT_WIFI_DISCONNECT_RESULT);

    net_mgmt_init_event_callback(&ipv4_cb, wifi_mgmt_event_handler, NET_EVENT_IPV4_ADDR_ADD);

    net_mgmt_add_event_callback(&wifi_cb);
    net_mgmt_add_event_callback(&ipv4_cb);

#ifdef CONFIG_BOARD_NRF7002DK_NRF5340_CPUAPP
    // Delay to prevent "Unable to get wpa_s handle for wlan0" on nRF Connect SDK 2.3.0.?
    printk("Sleeping for 1 second while wlan0 comes up\n");
    k_sleep(K_SECONDS(1));
#endif

    wifi_connect();
    k_sem_take(&wifi_connected, K_FOREVER);
    wifi_status();
    k_sem_take(&ipv4_address_obtained, K_FOREVER);
    printk("Ready...\n\n");
}







// #define SERVER_PORT 80

// #if defined(CONFIG_NET_CONFIG_PEER_IPV4_ADDR)
// #define SERVER_ADDR4 CONFIG_NET_CONFIG_PEER_IPV4_ADDR
// #else
// #define SERVER_ADDR4 "ws://10.2.1.19/socket.io/1/websocket"
// #endif

// static const char lorem_ipsum[] =
//     "3:::SETD^i.0.mute^1"
//     "3:::SETD^i.0.mix^0.5"
// 	"\n"
// 	;
// #define MAX_RECV_BUF_LEN (sizeof(lorem_ipsum) - 1)

// const int ipsum_len = MAX_RECV_BUF_LEN;

// static uint8_t recv_buf_ipv4[MAX_RECV_BUF_LEN];

// /* We need to allocate bigger buffer for the websocket data we receive so that
//  * the websocket header fits into it.
//  */
// #define EXTRA_BUF_SPACE 30

// static uint8_t temp_recv_buf_ipv4[MAX_RECV_BUF_LEN + EXTRA_BUF_SPACE];

// static int setup_socket(sa_family_t family, const char *server, int port,
// 						int *sock, struct sockaddr *addr, socklen_t addr_len)
// {
// 	int ret = 0;

// 	memset(addr, 0, addr_len);

// 	if (family == AF_INET)
// 	{
// 		net_sin(addr)->sin_family = AF_INET;
// 		net_sin(addr)->sin_port = htons(port);
// 		inet_pton(family, server, &net_sin(addr)->sin_addr);
// 	}

// 	*sock = socket(family, SOCK_STREAM, IPPROTO_TCP);

// 	if (*sock < 0)
// 	{
// 		LOG_ERR("Failed to create HTTP socket (%d)", -errno);
// 	}

// 	return ret;

// fail:
// 	if (*sock >= 0)
// 	{
// 		close(*sock);
// 		*sock = -1;
// 	}

// 	return ret;
// }

// static int connect_socket(sa_family_t family, const char *server, int port,
// 						  int *sock, struct sockaddr *addr, socklen_t addr_len)
// {
// 	int ret;

// 	ret = setup_socket(family, server, port, sock, addr, addr_len);
// 	if (ret < 0 || *sock < 0)
// 	{
// 		return -1;
// 	}

// 	ret = connect(*sock, addr, addr_len);
// 	if (ret < 0)
// 	{
// 		LOG_ERR("Cannot connect to IPv4 remote (%d)",
// 				-errno);
// 		ret = -errno;
// 	}

// 	return ret;
// }

// static int connect_cb(int sock, struct http_request *req, void *user_data)
// {
// 	LOG_INF("Websocket %d for %s connected.", sock, (char *)user_data);

// 	return 0;
// }

// static size_t how_much_to_send(size_t max_len)
// {
// 	size_t amount;

// 	do
// 	{
// 		amount = sys_rand32_get() % max_len;
// 	} while (amount == 0U);

// 	return amount;
// }

// static ssize_t sendall_with_ws_api(int sock, const void *buf, size_t len)
// {
// 	return websocket_send_msg(sock, buf, len, WEBSOCKET_OPCODE_DATA_TEXT,
// 							  true, true, SYS_FOREVER_MS);
// }

// static ssize_t sendall_with_bsd_api(int sock, const void *buf, size_t len)
// {
// 	return send(sock, buf, len, 0);
// }

// static void recv_data_wso_api(int sock, size_t amount, uint8_t *buf,
// 							  size_t buf_len, const char *proto)
// {
// 	uint64_t remaining = ULLONG_MAX;
// 	int total_read;
// 	uint32_t message_type;
// 	int ret, read_pos;

// 	read_pos = 0;
// 	total_read = 0;

// 	while (remaining > 0)
// 	{
// 		ret = websocket_recv_msg(sock, buf + read_pos,
// 								 buf_len - read_pos,
// 								 &message_type,
// 								 &remaining,
// 								 0);
// 		if (ret < 0)
// 		{
// 			if (ret == -EAGAIN)
// 			{
// 				k_sleep(K_MSEC(50));
// 				continue;
// 			}

// 			LOG_DBG("%s connection closed while "
// 					"waiting (%d/%d)",
// 					proto, ret, errno);
// 			break;
// 		}

// 		read_pos += ret;
// 		total_read += ret;
// 	}

// 	if (remaining != 0 || total_read != amount ||
// 		/* Do not check the final \n at the end of the msg */
// 		memcmp(lorem_ipsum, buf, amount - 1) != 0)
// 	{
// 		LOG_ERR("%s data recv failure %zd/%d bytes (remaining %" PRId64 ")",
// 				proto, amount, total_read, remaining);
// 		LOG_HEXDUMP_DBG(buf, total_read, "received ws buf");
// 		LOG_HEXDUMP_DBG(lorem_ipsum, total_read, "sent ws buf");
// 	}
// 	else
// 	{
// 		LOG_DBG("%s recv %d bytes", proto, total_read);
// 	}
// }

// static void recv_data_bsd_api(int sock, size_t amount, uint8_t *buf,
// 							  size_t buf_len, const char *proto)
// {
// 	int remaining;
// 	int ret, read_pos;

// 	remaining = amount;
// 	read_pos = 0;

// 	while (remaining > 0)
// 	{
// 		ret = recv(sock, buf + read_pos, buf_len - read_pos, 0);
// 		if (ret <= 0)
// 		{
// 			if (errno == EAGAIN || errno == ETIMEDOUT)
// 			{
// 				k_sleep(K_MSEC(50));
// 				continue;
// 			}

// 			LOG_DBG("%s connection closed while "
// 					"waiting (%d/%d)",
// 					proto, ret, errno);
// 			break;
// 		}

// 		read_pos += ret;
// 		remaining -= ret;
// 	}

// 	if (remaining != 0 ||
// 		/* Do not check the final \n at the end of the msg */
// 		memcmp(lorem_ipsum, buf, amount - 1) != 0)
// 	{
// 		LOG_ERR("%s data recv failure %zd/%d bytes (remaining %d)",
// 				proto, amount, read_pos, remaining);
// 		LOG_HEXDUMP_DBG(buf, read_pos, "received bsd buf");
// 		LOG_HEXDUMP_DBG(lorem_ipsum, read_pos, "sent bsd buf");
// 	}
// 	else
// 	{
// 		LOG_DBG("%s recv %d bytes", proto, read_pos);
// 	}
// }

// static bool send_and_wait_msg(int sock, size_t amount, const char *proto,
// 							  uint8_t *buf, size_t buf_len)
// {
// 	static int count;
// 	int ret;

// 	if (sock < 0)
// 	{
// 		return true;
// 	}

// 	/* Terminate the sent data with \n so that we can use the
// 	 *      websocketd --port=9001 cat
// 	 * command in server side.
// 	 */
// 	memcpy(buf, lorem_ipsum, amount);
// 	buf[amount] = '\n';

// 	/* Send every 2nd message using dedicated websocket API and generic
// 	 * BSD socket API. Real applications would not work like this but here
// 	 * we want to test both APIs. We also need to send the \n so add it
// 	 * here to amount variable.
// 	 */
// 	if (count % 2)
// 	{
// 		ret = sendall_with_ws_api(sock, buf, amount + 1);
// 	}
// 	else
// 	{
// 		ret = sendall_with_bsd_api(sock, buf, amount + 1);
// 	}

// 	if (ret <= 0)
// 	{
// 		if (ret < 0)
// 		{
// 			LOG_ERR("%s failed to send data using %s (%d)", proto,
// 					(count % 2) ? "ws API" : "socket API", ret);
// 		}
// 		else
// 		{
// 			LOG_DBG("%s connection closed", proto);
// 		}

// 		return false;
// 	}
// 	else
// 	{
// 		LOG_DBG("%s sent %d bytes", proto, ret);
// 	}

// 	if (count % 2)
// 	{
// 		recv_data_wso_api(sock, amount + 1, buf, buf_len, proto);
// 	}
// 	else
// 	{
// 		recv_data_bsd_api(sock, amount + 1, buf, buf_len, proto);
// 	}

// 	count++;

// 	return true;
// }

// int main(void)
// {
// 	/* Just an example how to set extra headers */
// 	const char *extra_headers[] = {
// 		"Origin: http://foobar\r\n",
// 		NULL};
// 	int sock4 = -1;
// 	int websock4 = -1;
// 	int32_t timeout = 3 * MSEC_PER_SEC;
// 	struct sockaddr_in addr4;
// 	size_t amount;
// 	int ret;

// 	k_sleep(K_MSEC(100000));
// 	LOG_INF("starting...");

// 	if (IS_ENABLED(CONFIG_NET_IPV4))
// 	{
// 		(void)connect_socket(AF_INET, SERVER_ADDR4, SERVER_PORT,
// 							 &sock4, (struct sockaddr *)&addr4,
// 							 sizeof(addr4));
// 	}

// 	if (sock4 < 0)
// 	{
// 		LOG_ERR("Cannot create HTTP connection.");
// 		k_sleep(K_FOREVER);
// 	}

// 	if (sock4 >= 0 && IS_ENABLED(CONFIG_NET_IPV4))
// 	{
// 		struct websocket_request req;

// 		memset(&req, 0, sizeof(req));

// 		req.host = SERVER_ADDR4;
// 		req.url = "/";
// 		req.optional_headers = extra_headers;
// 		req.cb = connect_cb;
// 		req.tmp_buf = temp_recv_buf_ipv4;
// 		req.tmp_buf_len = sizeof(temp_recv_buf_ipv4);

// 		websock4 = websocket_connect(sock4, &req, timeout, "IPv4");
// 		if (websock4 < 0)
// 		{
// 			LOG_ERR("Cannot connect to %s:%d", SERVER_ADDR4,
// 					SERVER_PORT);
// 			close(sock4);
// 		}
// 	}

// 	if (websock4 < 0)
// 	{
// 		LOG_ERR("No IPv4 connectivity");
// 		k_sleep(K_FOREVER);
// 	}

// 	LOG_INF("Websocket IPv4 %d", websock4);

// 	while (1)
// 	{
// 		amount = how_much_to_send(ipsum_len);

// 		if (websock4 >= 0 &&
// 			!send_and_wait_msg(websock4, amount, "IPv4",
// 							   recv_buf_ipv4, sizeof(recv_buf_ipv4)))
// 		{
// 			break;
// 		}

// 		k_sleep(K_MSEC(250));
// 	}

// 	if (websock4 >= 0)
// 	{
// 		close(websock4);
// 	}

// 	k_sleep(K_FOREVER);
// 	return 0;
// }
