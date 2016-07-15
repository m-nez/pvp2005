/*
 * libusb example program to list devices on the bus
 * Copyright © 2007 Daniel Drake <dsd@gentoo.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <stdio.h>
#include <unistd.h>

#include "libusb.h"
#include <signal.h>

char interrupt_sent = 0;

void interrupt(int s) {
	interrupt_sent = 1;
}

static libusb_device* get_mouse(libusb_device **devs) {

	libusb_device *dev;
	int i = 0;
	//const uint16_t ven = 0x09da; mouse
	//const uint16_t pro = 0xc10a;
	const uint16_t ven = 0x0b57;
	const uint16_t pro = 0x8502;

	while ((dev = devs[i++]) != NULL) {
		struct libusb_device_descriptor desc;
		int r = libusb_get_device_descriptor(dev, &desc);
		if (r < 0) {
			fprintf(stderr, "failed to get device descriptor");
			return NULL;
		}
		

		if (desc.idVendor == ven && desc.idProduct == pro) {
			return dev;
		}
	}
	return NULL;
}

int main(void)
{
	signal(SIGINT, interrupt);
	libusb_device **devs;
	int r;
	ssize_t cnt;

	r = libusb_init(NULL);
	if (r < 0)
		return r;

	cnt = libusb_get_device_list(NULL, &devs);
	if (cnt < 0)
		return (int) cnt;


	libusb_device* mouse = get_mouse(devs);
	libusb_device_handle* handle;
	if(libusb_open(mouse, &handle)) {
		perror("Can't open device");
	}
	unsigned char data[512];
	int length = 128;
	int transferred = -1;
	int ret;
	printf("%d\n", libusb_set_auto_detach_kernel_driver(handle, 1));
	perror("");
	printf("%d\n", libusb_claim_interface(handle, 0));
	perror("");
	//printf("transferred= %d\n", transferred);
	for(int i = 0; i < 120; ++i) {
		printf("%2X", i);
	}
	printf("\n");
	while(!interrupt_sent) {
	ret = libusb_interrupt_transfer(handle, 0x81, data, length, &transferred, 0);
		printf("\r");
		for(int i = 0; i < transferred; ++i) {
			printf("%2X", data[i]);
		}
		fflush(stdout);
	}
	printf("\n");


	libusb_release_interface(handle, 0);
	libusb_close(handle);
	libusb_free_device_list(devs, 1);

	libusb_exit(NULL);
	return 0;
}
