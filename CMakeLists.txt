cmake_minimum_required(VERSION 3.5)
PROJECT(Tablet C)

set(CMAKE_C_FLAGS "-g -Wall")
add_executable(tablet tablet.h tablet.c xevent.h xevent.c)
set(LIBS usb-1.0 X11)
target_link_libraries(tablet ${LIBS})
include_directories(/usr/include/libusb-1.0)
