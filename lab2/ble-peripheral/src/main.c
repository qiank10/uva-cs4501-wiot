/* main.c - Application main entry point */

/*
 * Copyright (c) 2015-2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/kernel.h>

#include <zephyr/settings/settings.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/services/bas.h>
#include <zephyr/bluetooth/services/cts.h>
#include <zephyr/bluetooth/services/hrs.h>
#include <zephyr/bluetooth/services/ias.h>

/* Custom Service Variables */
#define LAB2_SERVICE_UUID BT_UUID_128_ENCODE(0x5253FF4B, 0xE47C, 0x4EC8, 0x9792, 0x69FDF4923B4A)

static ssize_t characteristic_read(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset);

// Global value that saves state for the characteristic.
uint32_t characteristic_value = 0x7;

// Set up the advertisement data.
#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

static const struct bt_data ad[] = {
	BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
	BT_DATA_BYTES(BT_DATA_UUID128_ALL, LAB2_SERVICE_UUID)
};

// Setup the the service and characteristics.
BT_GATT_SERVICE_DEFINE(lab2_service,
	BT_GATT_PRIMARY_SERVICE(
		BT_UUID_DECLARE_128(LAB2_SERVICE_UUID)
	),
	BT_GATT_CHARACTERISTIC(BT_UUID_DECLARE_16(0x000a), BT_GATT_CHRC_READ,
			       BT_GATT_PERM_READ, characteristic_read, NULL, &characteristic_value),
);

static ssize_t characteristic_read(struct bt_conn *conn,
			                       const struct bt_gatt_attr *attr,
								   void *buf,
			                       uint16_t len,
								   uint16_t offset)
{
	// The `user_data` corresponds to the pointer provided as the last "argument"
	// to the `BT_GATT_CHARACTERISTIC` macro.
	uint32_t *value = attr->user_data;

	// Need to encode data into a buffer to send to client.
	uint8_t out_buffer[4] = {0};

	out_buffer[0]=0xc5;
	out_buffer[1]=0xec;
	out_buffer[2]=0x45;
	out_buffer[3]=0x01;

	// User helper function to encode the output data to send to
	// the client.
	return bt_gatt_attr_read(conn, attr, buf, len, offset, out_buffer, 4);
}


static void connected(struct bt_conn *conn, uint8_t err)
{
	if (err) {
		printk("Connection failed, err 0x%02x %s\n", err, bt_hci_err_to_str(err));
	} else {
		printk("Connected\n");
	}
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	printk("Disconnected, reason 0x%02x %s\n", reason, bt_hci_err_to_str(reason));
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
	.connected = connected,
	.disconnected = disconnected,
};

static void bt_ready(int err)
{
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return;
	}

	printk("Bluetooth initialized\n");

	err = bt_le_adv_start(BT_LE_ADV_CONN, ad, ARRAY_SIZE(ad), NULL, 0);
	if (err) {
		printk("Advertising failed to start (err %d)\n", err);
		return;
	}

	printk("Advertising successfully started\n");
}

int main(void)
{
	int err;

	err = bt_enable(bt_ready);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return 0;
	}
}
