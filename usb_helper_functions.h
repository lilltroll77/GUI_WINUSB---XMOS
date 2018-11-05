#ifndef USB_HELPER_FUNCTIONS_H
#define USB_HELPER_FUNCTIONS_H

#include "libusb.h"

#define XMOS_BULK_EP_IN 0x81
#define XMOS_BULK_EP_OUT 0x01

const char* const speed_name[5] = { "Unknown", "1.5 Mbit/s (USB LowSpeed)", "12 Mbit/s (USB FullSpeed)",
    "480 Mbit/s (USB HighSpeed)", "5000 Mbit/s (USB SuperSpeed)" };

libusb_device * print_devs(libusb_device *devs[] , int VID , int PID);
int test_device(uint16_t vid, uint16_t pid);

#endif // USB_HELPER_FUNCTIONS_H
