#include <hal/debug.h>
#include <hal/video.h>
#include <windows.h>
#include <usbh_lib.h>

/*
 * This demo will initialise the usb stack, detect events on the usb ports
 * and print the device PID and VID.
*/
void device_connection_callback(UDEV_T *udev, int status)
{
    debugPrint("Device connected on port %u (PID: %04x VID: %04x)\n", udev->port_num,
                                                                      udev->descriptor.idProduct,
                                                                      udev->descriptor.idVendor);
}

void device_disconnect_callback(UDEV_T *udev, int status)
{
     debugPrint("Device disconnected on port %u (PID: %04x VID: %04x)\n", udev->port_num,
                                                                          udev->descriptor.idProduct,
                                                                          udev->descriptor.idVendor);
}

int main(void)
{
    XVideoSetMode(640, 480, 32, REFRESH_DEFAULT);

    usbh_core_init();
    usbh_install_conn_callback(&device_connection_callback, &device_disconnect_callback);
    debugPrint("Plug and unplug USB devices to test\n");

    while (1) {
        usbh_pooling_hubs();
    }
    usbh_core_deinit();
    return 0;
}
