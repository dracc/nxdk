USBH_DIR=$(NXDK_DIR)/lib/usb/ohci_usbhostlib

#USB Host core
USB_CORE_SRCS := \
	$(USBH_DIR)/src_core/hub.c \
	$(USBH_DIR)/src_core/mem_alloc.c \
	$(USBH_DIR)/src_core/ohci.c \
	$(USBH_DIR)/src_core/support.c \
	$(USBH_DIR)/src_core/usb_core.c

#USB Human Input Device class driver
USB_HID_SRCS := \
	$(USBH_DIR)/src_hid/hid_core.c \
	$(USBH_DIR)/src_hid/hid_driver.c

#USB Communication Device class driver
USB_CDC_SRCS := \
	$(USBH_DIR)/src_cdc/cdc_core.c \
	$(USBH_DIR)/src_cdc/cdc_driver.c \
	$(USBH_DIR)/src_cdc/cdc_parser.c
	
#USB Audio class driver
USB_UAC_SRCS := \
	$(USBH_DIR)/src_uac/uac_core.c \
	$(USBH_DIR)/src_uac/uac_driver.c \
	$(USBH_DIR)/src_uac/uac_parser.c

# USB Video class driver
USB_UVC_SRCS := \
	$(USBH_DIR)/src_uvc/uvc_core.c \
	$(USBH_DIR)/src_uvc/uvc_driver.c \
	$(USBH_DIR)/src_uvc/uvc_parser.c

# Mass Storage Class driver
# FIXME: This driver is currently hardcoded so that it requires FATFS
# This requirement should be removed and abstracted out.
# USB_MSC_SRCS := \
#	$(USBH_DIR)/src_msc/msc_driver.c \
#	$(USBH_DIR)/src_msc/msc_xfer.c

USBH_OBJS = $(addsuffix .obj, $(basename $(USB_CORE_SRCS)))
USBH_OBJS += $(addsuffix .obj, $(basename $(USB_HID_SRCS)))
USBH_OBJS += $(addsuffix .obj, $(basename $(USB_CDC_SRCS)))
USBH_OBJS += $(addsuffix .obj, $(basename $(USB_UAC_SRCS)))
USBH_OBJS += $(addsuffix .obj, $(basename $(USB_UVC_SRCS)))
#USBH_OBJS += $(addsuffix .obj, $(basename $(USB_MSC_SRCS)))

$(NXDK_DIR)/lib/libnxdk_usb.lib: $(USBH_OBJS)

NXDK_CFLAGS += -I$(USBH_DIR)/inc
NXDK_CXXFLAGS += -I$(USBH_DIR)/inc

main.exe: $(NXDK_DIR)/lib/libnxdk_usb.lib

CLEANRULES += clean-usb

.PHONY: clean-usb
clean-usb:
	$(VE)rm -f $(USBH_OBJS) $(NXDK_DIR)/lib/libnxdk_usb.lib
