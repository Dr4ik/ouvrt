/*
 * Oculus Rift CV1 Radio
 * Copyright 2016 Philipp Zabel
 * SPDX-License-Identifier:	LGPL-2.0+ or BSL-1.0
 */
#include <asm/byteorder.h>
#include <glib.h>
#include <stdint.h>

#include "rift-hid-reports.h"
#include "rift-radio.h"

static void rift_dump_message(const unsigned char *buf, size_t len)
{
	unsigned int i;

	for (i = 0; i < len; i++)
		g_print(" %02x", buf[i]);
	g_print("\n");
}

static void rift_decode_remote_message(struct rift_remote *remote,
				       const struct rift_radio_message *message)
{
	int16_t buttons = __le16_to_cpu(message->remote.buttons);

	if (remote->buttons != buttons)
		remote->buttons = buttons;
}

static void rift_decode_touch_message(const struct rift_radio_message *message)
{
	const struct rift_touch_message *touch = &message->touch;
	int16_t accel[3] = {
		__le16_to_cpu(touch->accel[0]),
		__le16_to_cpu(touch->accel[1]),
		__le16_to_cpu(touch->accel[2]),
	};
	int16_t gyro[3] = {
		__le16_to_cpu(touch->gyro[0]),
		__le16_to_cpu(touch->gyro[1]),
		__le16_to_cpu(touch->gyro[2]),
	};
	uint16_t trigger = touch->trigger_grip_stick[0] |
			   ((touch->trigger_grip_stick[1] & 0x03) << 8);
	uint16_t grip = ((touch->trigger_grip_stick[1] & 0xfc) >> 2) |
			((touch->trigger_grip_stick[2] & 0x0f) << 6);
	uint16_t stick[2] = {
		((touch->trigger_grip_stick[2] & 0xf0) >> 4) |
		((touch->trigger_grip_stick[3] & 0x3f) << 4),
		((touch->trigger_grip_stick[3] & 0xc0) >> 6) |
		((touch->trigger_grip_stick[4] & 0xff) << 2),
	};

	(void)accel;
	(void)gyro;
	(void)trigger;
	(void)grip;
	(void)stick;
}

void rift_decode_radio_message(struct rift_radio *radio,
			       const unsigned char *buf, size_t len)
{
	const struct rift_radio_message *message = (const void *)buf;

	if (message->id == RIFT_RADIO_MESSAGE_ID) {
		if (message->device_type == RIFT_REMOTE) {
			rift_decode_remote_message(&radio->remote, message);
		} else if (message->device_type == RIFT_TOUCH_CONTROLLER_LEFT ||
			 message->device_type == RIFT_TOUCH_CONTROLLER_RIGHT) {
			rift_decode_touch_message(message);
		} else {
			g_print("%s: unknown device %02x:", radio->name,
				message->device_type);
			rift_dump_message(buf, len);
		}
	} else {
		unsigned int i;

		for (i = 1; i < len && !buf[i]; i++);
		if (i != len) {
			g_print("%s: unknown message:", radio->name);
			rift_dump_message(buf, len);
			return;
		}
	}
}