/*
 * Copyright (c) 2023 Maximilian Huber
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "uiclient.h"
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(uiclient, LOG_LEVEL_DBG);

#define MAX_RECV_BUF_LEN 1000 /* TODO */
static uint8_t recv_buf_ipv4[MAX_RECV_BUF_LEN];

/* We need to allocate bigger buffer for the websocket data we receive so that
 * the websocket header fits into it.
 */
#define EXTRA_BUF_SPACE 30
static uint8_t temp_recv_buf_ipv4[MAX_RECV_BUF_LEN + EXTRA_BUF_SPACE];

static int setup_socket(sa_family_t family, const char *server, int port,
                        int *sock, struct sockaddr *addr, socklen_t addr_len)
{
    const char *family_str = family == AF_INET ? "IPv4" : "IPv6";
    int ret = 0;

    memset(addr, 0, addr_len);

    if (family == AF_INET)
    {
        net_sin(addr)->sin_family = AF_INET;
        net_sin(addr)->sin_port = htons(port);
        inet_pton(family, server, &net_sin(addr)->sin_addr);
    }

    *sock = socket(family, SOCK_STREAM, IPPROTO_TCP);

    if (*sock < 0)
    {
        LOG_ERR("Failed to create %s HTTP socket (%d)", family_str,
                -errno);
    }

    return ret;

fail:
    if (*sock >= 0)
    {
        close(*sock);
        *sock = -1;
    }

    return ret;
}

static int connect_socket(sa_family_t family, const char *server, int port,
                          int *sock, struct sockaddr *addr, socklen_t addr_len)
{
    int ret;

    ret = setup_socket(family, server, port, sock, addr, addr_len);
    if (ret < 0 || *sock < 0)
    {
        return -1;
    }

    ret = connect(*sock, addr, addr_len);
    if (ret < 0)
    {
        LOG_ERR("Cannot connect to %s remote (%d)",
                family == AF_INET ? "IPv4" : "IPv6",
                -errno);
        ret = -errno;
    }

    return ret;
}

static int connect_cb(int sock, struct http_request *req, void *user_data)
{
    // LOG_INF("Websocket %d connected.", sock;

    return 0;
}

int UICLIENT::init_socket(void)
{
    const char *extra_headers[] = {
        "Origin: http://foobar\r\n",
        NULL};
    int sock4 = -1;
    int websock4 = -1;
    int32_t timeout = 3 * MSEC_PER_SEC;
    struct sockaddr_in addr4;
    size_t amount;
    int ret;

    (void)connect_socket(AF_INET, ip, port,
                         &sock4, (struct sockaddr *)&addr4,
                         sizeof(addr4));

    if (sock4 < 0)
    {
        LOG_ERR("Cannot create HTTP connection.");
        k_sleep(K_FOREVER);
    }

    struct websocket_request req;

    memset(&req, 0, sizeof(req));

    req.host = ip;
    req.url = "/";
    req.optional_headers = extra_headers;
    req.cb = connect_cb;
    req.tmp_buf = temp_recv_buf_ipv4;
    req.tmp_buf_len = sizeof(temp_recv_buf_ipv4);

    websock4 = websocket_connect(sock4, &req, timeout, NULL);
    if (websock4 < 0)
    {
        LOG_ERR("Cannot connect to %s:%d", ip, port);
        close(sock4);
    }

    if (websock4 < 0)
    {
        LOG_ERR("No IPv4 or IPv6 connectivity");
        k_sleep(K_FOREVER);
    }

    LOG_INF("Websocket IPv4 %d", websock4);

    return websock4;
}

UICLIENT::UICLIENT(const char *_ip, int _port)
{
    LOG_MODULE_DECLARE(uiclient);
    ip = _ip;
    port = _port;

    socket = init_socket();
}

void UICLIENT::get(int channel)
{
}