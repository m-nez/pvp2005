#include "xevent.h"

void x_get_root_window(window_t* w) {
	int screen_num;
	w->display = XOpenDisplay(0);
	screen_num = DefaultScreen(w->display);
	w->width = DisplayWidth(w->display, screen_num);
	w->height = DisplayHeight(w->display, screen_num);
	w->w = XRootWindow(w->display, 0);
}

void x_set_pointer(window_t* w, float x, float y) {

	XWarpPointer(w->display,
		   	None, w->w, 0, 0, 0, 0, 
			x * w->width, y * w->height);

	XFlush(w->display);
}

void x_send_button(window_t* w) {
	XButtonEvent e;
	e.send_event = 1;
	e.type = ButtonPress;
	e.display = w->display;
	e.window = w->w;
	e.root = w->w;
	e.subwindow = None;
	e.time = CurrentTime;
	e.x = 300;
	e.y = 300;
	e.x_root = 300;
	e.y_root = 300;
	e.state = 0; // TO change
	e.button = Button2;
	e.same_screen = 1;

XSendEvent(w->display, w->w, 1, ButtonPressMask, (XEvent *)&e);
}
