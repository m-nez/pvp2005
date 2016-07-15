#include "tablet.h"
#include <signal.h>

void tablet_init(tablet_t* t) {
	t->scroll_max = 0x26;
	t->scroll_min = 0x3;
	t->horizontal_max = 0x27DE;
	t->vertical_max = 0x1CFE;
	t->prev_point_state = 0xA0;
	t->prev_button_state = 0x0;
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
			return NULL;
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
void tablet_parse_data(tablet_t* t, unsigned char* data, window_t* w) {
	if (data[0] == 0x1) { /* No pen */
		if (data[2] & 0xA0) { /* Buttons */
		
		} else { /* Scroll */
			
		}
	} else { /* Pen */
		if (data[1] != 0x0) { /* Pen is near the drawing plane */
			uint16_t x = reverse_bytes(*(uint16_t*)(data + 2));
			uint16_t y = reverse_bytes(*(uint16_t*)(data + 4));
			float fx = (float)x / t->horizontal_max;
			float fy = (float)y / t->vertical_max;
			printf("  %8d %8d %8g %8g %8d %8d", x, y, fx, fy, t->horizontal_max, t->vertical_max);
			x_set_pointer(w, fx, fy);
			if(!(t->prev_button_state & 0x01)) {
				if (data[1] & 0x01) {
					x_send_button(w);
					printf("  BUTTON");
					t->prev_button_state |= 0x01;
				}
			} else {
				t->prev_button_state &= 0xFE;
			}
		}
	}
}

static char interrupt_sent = 0;

static void interrupt(int s) {
	interrupt_sent = 1;
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


	libusb_device* t_dev = tablet_get_device(devs);
	if (t_dev == NULL) {
		return 1;
	}
	libusb_device_handle* handle;
	if((r = libusb_open(t_dev, &handle))) {
		perror("Can't open device");
		return r;
	}

	tablet_t tablet;
	window_t window;
	tablet_init(&tablet);
	x_get_root_window(&window);
	unsigned char data[10];
	int length = 10;
	int transferred = -1;
	if ((r = libusb_set_auto_detach_kernel_driver(handle, 1))) {
		perror("libusb_set_auto_detach_kernel_driver");
	}
	if ((r = libusb_claim_interface(handle, 0))) {
		perror("libusb_claim_interface");
		return r;
	}
	for(int i = 0; i < 10; ++i) {
		printf("%2X", i);
	}
	printf("\n");
	while(!interrupt_sent) {
	r = libusb_interrupt_transfer(handle, 0x81, data, length, &transferred, 0);
		printf("\r");
		for(int i = 0; i < transferred; ++i) {
			printf("%2X", data[i]);
		}
		tablet_parse_data(&tablet, data, &window);
		fflush(stdout);
	}
	printf("\n");


	libusb_release_interface(handle, 0);
	libusb_close(handle);
	libusb_free_device_list(devs, 1);

	libusb_exit(NULL);
	return 0;
}
