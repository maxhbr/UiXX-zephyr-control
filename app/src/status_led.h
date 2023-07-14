/*
 * Copyright (c) 2023 Maximilian Huber
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __STATUS_LED_H_
#define __STATUS_LED_H_

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/led_strip.h>
#include <zephyr/device.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/sys/util.h>

#define DELAY_TIME K_MSEC(50)

#define RGB(_r, _g, _b)                 \
    {                                   \
        .r = (_r), .g = (_g), .b = (_b) \
    }

static const struct led_rgb colors[] = {
    RGB(0x0f, 0x00, 0x00), /* red */
    RGB(0x00, 0x0f, 0x00), /* green */
    RGB(0x00, 0x00, 0x0f), /* blue */
};

class STATUS_LED
{
    const struct device *strip;
    int strip_num_pixels;

public:
    STATUS_LED(const struct device *_strip, int _strip_num_pixels);
    void blue();
    void red();
    void green();
};

#endif // __STATUS_LED_H_