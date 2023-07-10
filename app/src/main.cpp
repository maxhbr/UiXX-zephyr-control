/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/arch/cpu.h>
#include <zephyr/sys/printk.h>

extern "C" int client_main(void);

int main(void)
{
	// printk("Hello World! %s\n", CONFIG_BOARD);

	// return client_main();

	return 0;
}
