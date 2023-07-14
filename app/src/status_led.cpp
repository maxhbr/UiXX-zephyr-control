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

#define STRIP_NODE DT_ALIAS(led_strip)
#define STRIP_NUM_PIXELS DT_PROP(DT_ALIAS(led_strip), chain_length)

STATUS_LED::STATUS_LED()
{
  LOG_MODULE_DECLARE(led);
  strip = DEVICE_DT_GET(STRIP_NODE);
	if (device_is_ready(strip)) {
		LOG_INF("Found LED strip device %s", strip->name);
  }
  //   pin = _pin;
  //   dev = device_get_binding(label);
  // static const struct device *const strip = DEVICE_DT_GET(STRIP_NODE);
}