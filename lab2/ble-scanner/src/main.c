/**
 * Copyright (c) 2024 Croxel, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/types.h>
#include <stddef.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>

static void scan_recv(const struct bt_le_scan_recv_info *info, struct net_buf_simple *buf)
{
	char src_addr[18];

	// Convert address to typical MAC address format.
	bt_addr_le_to_str(info->addr, src_addr, 18);

	// Print the advertiser's source address and received signal strength indicator (RSSI)
	printk("[BLE ADV] src: %s (rssi: %i)\n", src_addr, info->rssi);
}

static struct bt_le_scan_cb scan_callbacks = {
	.recv = scan_recv,
};

int main(void)
{
	/* Configuration of the Bluetooth Scanner */
	struct bt_le_scan_param scan_param = {
		.type       = BT_HCI_LE_SCAN_PASSIVE,
		.options    = BT_LE_SCAN_OPT_NONE,
		.interval   = 0x0010,
		.window     = 0x0010,
	};

	int err;

	printf("Starting Scanner\n");

	/* Initialize the Bluetooth Subsystem */
	err = bt_enable(NULL);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return -1;
	}

	/* Register the scan callback function*/
	bt_le_scan_cb_register(&scan_callbacks);

	/* Start the Bluetooth scanner*/
	err = bt_le_scan_start(&scan_param, NULL);
	if (err) {
		printk("failed (err %d)\n", err);
		return -1;
	}
}