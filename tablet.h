/*
Copyright (C) 2016 Michał Nieznański

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License version 2 as
published by the Free Software Foundation.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with this program; if not, see <http://www.gnu.org/licenses/>.
===========================================================================
*/

#pragma once

#include <stdint.h>
#include <stdio.h>
#include <libusb.h>

#include "xwrap.h"

typedef struct {
	uint8_t scroll_max;
	uint8_t scroll_min;
	uint8_t prev_scroll;
	uint16_t horizontal_max;
	uint16_t vertical_max;
	uint8_t prev_point_state;
	uint8_t prev_key_state;
} tablet_t;

/* Initialize values for Pentagram Virtuoso P 2005 */
void tablet_init(tablet_t* t);
/* Find Pentagram Virtuoso P 2005, return its device pointer
 * else return NULL */
libusb_device* tablet_get_device(libusb_device** devs);

void tablet_parse_data(tablet_t* t, unsigned char* data, xwrap_t* xw);
