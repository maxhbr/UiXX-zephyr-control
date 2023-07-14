/*
 * Copyright (c) 2023 Maximilian Huber
 * Copyright (c) 2018 Intel Corporation
 * Copyright (c) 2017 Linaro Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <string.h>

#define LOG_LEVEL 4
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(status_led);

#include "status_led.h"

STATUS_LED::STATUS_LED(const struct device *_strip, int _strip_num_pixels)
{
  LOG_MODULE_DECLARE(status_led);
  strip = _strip;
  strip_num_pixels = _strip_num_pixels;
  if (device_is_ready(strip))
  {
    LOG_INF("Found LED strip device %s", strip->name);
  }
  //   pin = _pin;
  //   dev = device_get_binding(label);
  // static const struct device *const strip = DEVICE_DT_GET(STRIP_NODE);
}

void STATUS_LED::blue()
{
  LOG_MODULE_DECLARE(status_led);
}

void STATUS_LED::red()
{
  LOG_MODULE_DECLARE(status_led);
}

void STATUS_LED::green()
{
  LOG_MODULE_DECLARE(status_led);
}