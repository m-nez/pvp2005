#pragma once

#include "X11/X.h"
#include "X11/Xlib.h"

typedef struct {
	Display* display;
	Window w;
	unsigned int width;
	unsigned int height;
} window_t;

void x_get_root_window(window_t* w);
void x_set_pointer(window_t* w, float x, float y);
void x_send_button(window_t* w);
