/*
 * Copyright (c) 2023 Maximilian Huber
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __UICLIENT_H_
#define __UICLIENT_H_

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>

class UICLIENT
{
    const char *ip;
    int port;
    int socket;

    int init_socket(void);

public:
    UICLIENT(const char *_ip, int _port);
    void get(int channel);
};

#endif // __UICLIENT_H_
