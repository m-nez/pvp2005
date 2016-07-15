#pragma once

#include <stdint.h>
#include <stdio.h>
#include <libusb.h>

#include "xevent.h"

typedef struct {
	uint8_t scroll_max;
	uint8_t scroll_min;
	uint16_t horizontal_max;
	uint16_t vertical_max;
	uint8_t prev_point_state;
	uint8_t prev_button_state;
} tablet_t;

/* Initialize values for Pentagram Virtuoso P 2005 */
void tablet_init(tablet_t* t);
/* Find Pentagram Virtuoso P 2005, return its device pointer
 * else return NULL */
libusb_device* tablet_get_device(libusb_device** devs);

void tablet_parse_data(tablet_t* t, unsigned char* data, window_t* w);
