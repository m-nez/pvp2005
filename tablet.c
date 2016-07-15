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


#include "tablet.h"
#include <signal.h>

void tablet_init(tablet_t* t) {
	t->scroll_max = 0x26;
	t->scroll_min = 0x3;
	t->prev_scroll = 0x0;
	t->horizontal_max = 0x27DE;
	t->vertical_max = 0x1CFE;
	t->prev_point_state = 0xA0;
	t->prev_key_state = 0x0;
}

libusb_device* tablet_get_device(libusb_device **devs) {
	libusb_device *dev;
	int i = 0;
	const uint16_t ven = 0x0b57;
	const uint16_t pro = 0x8502;

	while ((dev = devs[i++]) != NULL) {
		struct libusb_device_descriptor desc;
		int r = libusb_get_device_descriptor(dev, &desc);
		if (r < 0) {
			continue;
		}

		if (desc.idVendor == ven && desc.idProduct == pro) {
			return dev;
		}
	}
	fprintf(stderr, "Could not find the device\n");
	return NULL;
}

static uint16_t reverse_bytes(uint16_t v) {
	return (0xFF00 & v) >> 8 | (0xFF & v) << 8;
}

static void check_button(
		unsigned char button_mask,
		int button,
		unsigned char prev_state,
		unsigned char cur_state,
		xwrap_t* xw
		) {
	if (prev_state & button_mask) {
		/* Button was already pressed */
		if ((cur_state & button_mask) == 0x0) {
			/* Now it is released */
			xdo_mouse_up(xw->xdo, CURRENTWINDOW, button);
		}
	} else {
		/* Button was released */
		if (cur_state & button_mask) {
			/* Now it is pressed */
			xdo_mouse_down(xw->xdo, CURRENTWINDOW, button);
		}
	}
}

void tablet_parse_data(tablet_t* t, unsigned char* data, xwrap_t* xw) {
	int i;
	static const char* keyseq[] = {"f", "f", "Control_L", "Shift_L"};
	uint8_t key_mask;
	if (data[0] == 0x1) { /* No pen */
		if ((data[2] & 0xA0) == 0xA0) { /* Buttons */
			if (t->prev_scroll != 0x0) {
				xdo_mouse_up(xw->xdo, CURRENTWINDOW, 2); /* Map middle mouse button to scroll */
				t->prev_scroll = 0x0;
			}
			for (i = 0; i < 4; ++i) {
				key_mask = 0x1 << i;
				if (t->prev_key_state & key_mask) { /* Key was already pressed */
					if ((key_mask & data[2]) == 0x0) { /* Now it is released */
						xdo_send_keysequence_window_up(xw->xdo, CURRENTWINDOW, keyseq[i], 0);
					}
				} else { /* Key was released */
					if (key_mask & data[2]) { /* Now it is pressed */
						xdo_send_keysequence_window_down(xw->xdo, CURRENTWINDOW, keyseq[i], 0);
					}
				}
			}
			t->prev_key_state = data[2];
		} else { /* Scroll */
			if (t->prev_scroll == 0x0) {
				xdo_mouse_down(xw->xdo, CURRENTWINDOW, 2); /* Map middle mouse button to scroll */
			}
			t->prev_scroll = data[2];
		}
	} else { /* Pen */
		if (data[1] != 0x0) { /* Pen is near the drawing plane */
			uint16_t x = reverse_bytes(*(uint16_t*)(data + 2));
			uint16_t y = reverse_bytes(*(uint16_t*)(data + 4));
			float fx = (float)x / t->horizontal_max;
			float fy = (float)y / t->vertical_max;

			xdo_move_mouse(xw->xdo, fx * xw->width, fy * xw->height, 0);

			check_button(0x01, 1, t->prev_point_state, data[1], xw); /* Left mouse button */
			check_button(0x02, 3, t->prev_point_state, data[1], xw); /* Right mouse button */
			t->prev_point_state = data[1];
		}
	}
}

static char interrupt_sent = 0;

static void interrupt(int s) {
	interrupt_sent = 1;
}

int main(void)
{
	libusb_device **devs;
	int r;
	ssize_t cnt;
	tablet_t tablet;
	xwrap_t xwrap;

	unsigned char data[10];
	int length = 10;
	int transferred = -1;

	signal(SIGINT, interrupt);

	r = libusb_init(NULL);
	if (r < 0)
		return r;

	cnt = libusb_get_device_list(NULL, &devs);
	if (cnt < 0)
		return (int) cnt;


	libusb_device* t_dev = tablet_get_device(devs);
	if (t_dev == NULL) {
		return 1;
	}
	libusb_device_handle* handle;
	if((r = libusb_open(t_dev, &handle))) {
		perror("Can't open device");
		return r;
	}

	tablet_init(&tablet);
	xwrap_init(&xwrap);
	if ((r = libusb_set_auto_detach_kernel_driver(handle, 1))) {
		perror("libusb_set_auto_detach_kernel_driver");
	}
	if ((r = libusb_claim_interface(handle, 0))) {
		perror("libusb_claim_interface");
		return r;
	}
	while(!interrupt_sent) {
		r = libusb_interrupt_transfer(handle, 0x81, data, length, &transferred, 0);
		tablet_parse_data(&tablet, data, &xwrap);
	}

	libusb_release_interface(handle, 0);
	libusb_close(handle);
	libusb_free_device_list(devs, 1);

	libusb_exit(NULL);
	return 0;
}
