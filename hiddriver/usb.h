#pragma once
#include <stdint.h>

struct usb_device_descriptor {
	uint8_t  bLength;             // Size of this descriptor in bytes (18)
	uint8_t  bDescriptorType;     // DEVICE descriptor type (1)
	uint16_t bcdUSB;              // USB Specification Release Number (e.g., 0x0200 for USB 2.0)
	uint8_t  bDeviceClass;        // Class code (assigned by USB-IF)
	uint8_t  bDeviceSubClass;     // Subclass code
	uint8_t  bDeviceProtocol;     // Protocol code
	uint8_t  bMaxPacketSize0;     // Max packet size for endpoint 0 (8, 16, 32, or 64)
	uint16_t idVendor;            // Vendor ID (assigned by USB-IF)
	uint16_t idProduct;           // Product ID (assigned by manufacturer)
	uint16_t bcdDevice;           // Device release number in binary-coded decimal
	uint8_t  iManufacturer;       // Index of string descriptor describing manufacturer
	uint8_t  iProduct;            // Index of string descriptor describing product
	uint8_t  iSerialNumber;       // Index of string descriptor for the device's serial number
	uint8_t  bNumConfigurations;  // Number of possible configurations
};

#pragma pack(push, 1)
struct usb_hid_descriptor {
	uint8_t  bLength;
	uint8_t  bDescriptorType;
	uint16_t bcdHID;
	uint8_t  bCountryCode;
	uint8_t  bNumDescriptors;
	uint8_t  bDescriptorType2;
	uint16_t wDescriptorLength;
};
#pragma pack(pop)

struct usb_interface_descriptor {
	uint8_t bLength;              // Size of this descriptor in bytes (9)
	uint8_t bDescriptorType;      // INTERFACE descriptor type (4)
	uint8_t bInterfaceNumber;     // Number of this interface
	uint8_t bAlternateSetting;    // Value used to select this alternate setting
	uint8_t bNumEndpoints;        // Number of endpoints used by this interface (excluding EP0)
	uint8_t bInterfaceClass;      // Class code (assigned by USB-IF)
	uint8_t bInterfaceSubClass;   // Subclass code
	uint8_t bInterfaceProtocol;   // Protocol code
	uint8_t iInterface;           // Index of string descriptor describing this interface
};


struct usb_endpoint_descriptor {
	uint8_t  bLength;          // Size of this descriptor in bytes (7)
	uint8_t  bDescriptorType;  // ENDPOINT descriptor type (5)
	uint8_t  bEndpointAddress; // Endpoint address and direction:
							   // Bit 7: Direction (0=OUT, 1=IN)
							   // Bits 3..0: Endpoint number (1�15)
	uint8_t  bmAttributes;     // Transfer type:
							   // Bits 1..0: 00=Control, 01=Isochronous, 10=Bulk, 11=Interrupt
							   // For Isochronous and Interrupt, additional bits define usage
	uint16_t wMaxPacketSize;   // Maximum packet size this endpoint is capable of sending or receiving
	uint8_t  bInterval;        // Polling interval for data transfers (in frames or microframes)
};


// defined by USB HID spec
enum HatSwitch {
	HAT_UP = 0,
	HAT_UP_RIGHT = 1,
	HAT_RIGHT = 2,
	HAT_DOWN_RIGHT = 3,
	HAT_DOWN = 4,
	HAT_DOWN_LEFT = 5,
	HAT_LEFT = 6,
	HAT_UP_LEFT = 7,
	HAT_NEUTRAL = 8
};

//USB HID Generic Desktop usage page / usages
#define HID_USAGE_PAGE_GENERIC_DESKTOP  0x01
#define HID_USAGE_PAGE_BUTTON           0x09
#define HID_USAGE_PAGE_VENDOR			0xFF00

#define HID_USAGE_AXIS_X        0x30
#define HID_USAGE_AXIS_Y        0x31
#define HID_USAGE_AXIS_Z        0x32
#define HID_USAGE_AXIS_RX       0x33
#define HID_USAGE_AXIS_RY       0x34
#define HID_USAGE_AXIS_RZ       0x35
#define HID_USAGE_HAT_SWITCH    0x39
#define HID_USAGE_GAMEPAD       0x05
#define HID_USAGE_JOYSTICK      0x04

#define PS_RAW_EXTENDED_DATA_PS4 42
#define PS_RAW_EXTENDED_DATA_PS5 39
#define PS_EXTENDED_OFFSET_PICKUP     0
#define PS_EXTENDED_OFFSET_WHAMMY     1
#define PS_EXTENDED_OFFSET_TILT       2
#define PS_EXTENDED_OFFSET_RED_PAD    0
#define PS_EXTENDED_OFFSET_BLUE_PAD   1
#define PS_EXTENDED_OFFSET_YELLOW_PAD 2
#define PS_EXTENDED_OFFSET_GREEN_PAD  3
#define PS_EXTENDED_OFFSET_YELLOW_CYM 4
#define PS_EXTENDED_OFFSET_BLUE_CYM   5
#define PS_EXTENDED_OFFSET_GREEN_CYM  6

#define HID_USAGE_PS3_CAPABILITIES 0x2621
#define HID_USAGE_PS4_CAPABILITIES 0x2721
#define HID_USAGE_PS5_CAPABILITIES 0x2821

#pragma pack(push, 1)
struct Report {
	uint8_t reportId;
};

struct ButtonsReport {
	bool has_hat_switch;
	int16_t x;
	int16_t y;
	int16_t z;
	int16_t rz;
	int16_t rx;
	int16_t ry;
	uint8_t y_button;
	uint8_t b_button;
	uint8_t a_button;
	uint8_t x_button;
	uint8_t hatSwitch;

	// for controllers without a hat switch
	uint8_t dpad_left;
	uint8_t dpad_right;
	uint8_t dpad_up;
	uint8_t dpad_down;

	uint8_t r3;
	uint8_t l3;
	uint8_t start;
	uint8_t back;
	uint8_t r2;
	uint8_t l2;
	uint8_t r1;
	uint8_t l1;
	uint8_t xbox;
};
#pragma pack(pop)