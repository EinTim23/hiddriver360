#include <xtl.h>
#include <xkelib.h>
#include <string>
#include <fstream>
#include <time.h>
#include <sys/stat.h>
#include <fstream>
#include <sstream>
#include <vector>
#include "Detours.h"
#include "hid_parser.h"  

Detour HidAddDeviceDetour;
Detour HidRemoveDeviceDetour;
Detour XamInputSetStateDetour;
Detour XamInputGetCapabilitiesDetour;
Detour XInputdReadStateDetour;

uint16_t swap_endianness_16(uint16_t val) {
	return (val >> 8) | (val << 8);
}

BOOL IsTrayOpen() {
	BYTE Input[0x10] = { 0 }, Output[0x10] = { 0 };
	Input[0] = 0xA;
	HalSendSMCMessage(Input, Output);
	return (Output[1] == 0x60);
}

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
	BYTE  bLength;
	BYTE  bDescriptorType;
	WORD  bcdHID;
	BYTE  bCountryCode;
	BYTE  bNumDescriptors;
	BYTE  bDescriptorType2;
	WORD  wDescriptorLength;
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
							   // Bits 3..0: Endpoint number (1–15)
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

#define HID_USAGE_AXIS_X        0x30
#define HID_USAGE_AXIS_Y        0x31
#define HID_USAGE_AXIS_Z        0x32
#define HID_USAGE_AXIS_RX       0x33
#define HID_USAGE_AXIS_RY       0x34
#define HID_USAGE_AXIS_RZ       0x35
#define HID_USAGE_HAT_SWITCH    0x39
#define HID_USAGE_GAMEPAD       0x05
#define HID_USAGE_JOYSTICK      0x04

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
	uint8_t triangle;
	uint8_t circle;
	uint8_t cross;
	uint8_t square;
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

struct UsbTrb {
	DWORD endpoint;
	DWORD callback;
	DWORD savedEndpoint;
	BYTE  padding[4];
	BYTE  flags;
	BYTE  controllerIndex;   // written by UsbdQueueAsyncTransfer
	BYTE  pad2;
	BYTE  endpointIndex;     // written by UsbdQueueAsyncTransfer
	void* buffer;
	DWORD length;
};

struct UsbPacket {
	BYTE  bmRequestType;
	BYTE  bRequest;
	WORD  wValue;
	WORD  wIndex;
	WORD  wLength;
};

struct UsbControlTrb {
	UsbTrb          trb;          
	BYTE            pad[4];       
	UsbPacket  packet;  
};

struct deviceHandle;
struct __declspec(align(2)) HidControllerExtension
{
	deviceHandle* deviceHandle;
	UsbTrb interruptTrb;
	BYTE gap20[4];
	UsbControlTrb controlTrb;
	BYTE gap4C[4];
	DWORD cleanupHandler;
	BYTE gap54[24];
	DWORD queue;
	BYTE alwaysOne;
	BYTE alwaysOneTwo;
	BYTE unknownFlag;
	BYTE alwaysZero;
	BYTE cleanupDone;
	BYTE initTransferPending;
	BYTE alwaysZeroTwo;
	unsigned __int8 deviceType;
	BYTE alwaysZeroThree;
	BYTE alwaysZeroFour;
};

struct deviceHandle {
	HidControllerExtension* driver;
};

typedef struct _XINPUT_CAPABILITIESEX
{
	BYTE                                Type;
	BYTE                                SubType;
	WORD                                Flags;
	XINPUT_GAMEPAD                      Gamepad;
	XINPUT_VIBRATION                    Vibration;
	DWORD unk1;
	DWORD unk2;
	DWORD unk3;
} XINPUT_CAPABILITIES_EX, * PXINPUT_CAPABILITIES_EX;

enum InitState
{
	INIT_SET_CONFIGURATION,
	INIT_GET_HID_DESCRIPTOR,
	INIT_GET_REPORT_DESCRIPTOR,
	INIT_DONE,
	INIT_FAILED
};

InitState g_InitState;
#define USB_ENDPOINT_TYPE_CONTROL     0x00
#define USB_ENDPOINT_TYPE_ISOCHRONOUS 0x01
#define USB_ENDPOINT_TYPE_BULK        0x02
#define USB_ENDPOINT_TYPE_INTERRUPT   0x03
#define USB_DIRECTION_IN  1
#define USB_DIRECTION_OUT 0

const uint16_t NINTENDO_VENDOR_ID = 0x057E;
const uint16_t SWITCH_PRO_PRODUCT_ID = 0x2009;

const unsigned char nintendo_handshake[2] = { 0x80, 0x02 };
const unsigned char switch_baudrate[2] = { 0x80, 0x03 };
const unsigned char hid_only_mode[2] = { 0x80, 0x04 };

static bool NeedsNintendoHandshake(uint16_t vid, uint16_t pid) {
	if (vid != NINTENDO_VENDOR_ID) return false;
	return pid == SWITCH_PRO_PRODUCT_ID;
}

typedef usb_device_descriptor* (*usb_device_descriptor_func_t)(deviceHandle* handle);
typedef usb_interface_descriptor* (*usb_interface_descriptor_func_t)(deviceHandle* handle);
typedef int(*usb_add_device_complete_func_t)(deviceHandle* handle, int status_code);
typedef int(*usb_get_device_speed_func_t)(deviceHandle* handle);
typedef int(*usb_queue_async_transfer_func_t)(deviceHandle* handle, void* endpoint);
typedef NTSTATUS(*usb_queue_close_endpoint_func_t)(deviceHandle* handle, void* endpoint);
typedef NTSTATUS(*usb_remove_device_complete_func_t)(deviceHandle* handle);
typedef NTSTATUS(*usb_close_default_endpoint_func_t)(deviceHandle* handle, DWORD* endpoint);
typedef NTSTATUS(*usb_open_default_endpoint_func_t)(deviceHandle* handle, DWORD* endpoint);
typedef NTSTATUS(*usb_open_endpoint_func_t)(deviceHandle* handle, int transfertype, int endpointAddress, int maxPacketLength, int interval, DWORD* endpoint);
typedef usb_endpoint_descriptor* (*usb_endpoint_descriptor_func_t)(deviceHandle* handle, int index, int transfertype, int direction);
typedef int(*xam_user_bind_device_callback_func_t)(unsigned int controllerId, unsigned int context, unsigned __int8 category, bool disconnect, unsigned __int8* userIndex);
typedef int(*usbd_powerdown_notification_func_t)();
typedef void(*mm_free_physical_memory_func_t)(DWORD type, DWORD address);

usb_device_descriptor_func_t UsbdGetDeviceDescriptor = nullptr;
usb_interface_descriptor_func_t UsbdGetInterfaceDescriptor = nullptr;
usb_endpoint_descriptor_func_t UsbdGetEndpointDescriptor = nullptr;
usb_add_device_complete_func_t UsbdAddDeviceComplete = nullptr;
usb_open_default_endpoint_func_t UsbdOpenDefaultEndpoint = nullptr;
usb_open_endpoint_func_t UsbdOpenEndpoint = nullptr;
usb_get_device_speed_func_t UsbdGetDeviceSpeed = nullptr;
usb_queue_async_transfer_func_t UsbdQueueAsyncTransfer = nullptr;
usb_queue_close_endpoint_func_t UsbdQueueCloseEndpoint = nullptr;
usb_close_default_endpoint_func_t UsbdQueueCloseDefaultEndpoint = nullptr;
usb_remove_device_complete_func_t UsbdRemoveDeviceComplete = nullptr;
xam_user_bind_device_callback_func_t XamUserBindDeviceCallback = nullptr;
usbd_powerdown_notification_func_t UsbdPowerDownNotification = nullptr;
usbd_powerdown_notification_func_t UsbdDriverEntry = nullptr;
mm_free_physical_memory_func_t MmFreePhysicalMemory = nullptr;

enum NINTENDO_HANDSHAKE_STATE {
	INITIAL,
	HANDSHAKE,
	DONE
};
struct Controller {
	deviceHandle* deviceHandle;
	HidControllerExtension* controllerDriver;
	ButtonsReport currentState;
	uint8_t userIndex;
	uint32_t deviceContext;
	uint16_t vendorId;
	uint16_t productId;
	uint32_t packetNumber;
	HID_ReportInfo_t* reportInfo;
	uint8_t reportId;
	void* reportData;

	// for nintendo specific handshake
	NINTENDO_HANDSHAKE_STATE nintendo_handshake_state;
	UsbTrb interruptTrb;
} __declspec(align(4));

Controller connectedControllers[4];
Controller c;
usb_hid_descriptor hidDescriptorBuffer;
int globalIndex = -1;
void* reportDescriptorBuffer;
int interruptHandler(DWORD deviceHandle, int32_t a2);

// Keep every IN report item; the driver uses all axes, the hat, and buttons.
bool CALLBACK_HIDParser_FilterHIDReportItem(HID_ReportItem_t* const CurrentItem) {
	return (CurrentItem->ItemType == HID_REPORT_ITEM_In);
}

static HID_ReportItem_t* FindItemByUsage(
	HID_ReportInfo_t* info,
	uint16_t usagePage,
	uint16_t usage,
	uint8_t  reportId)
{
	for (HID_ReportItem_t* item = info->FirstReportItem; item; item = item->Next)
	{
		if (item->ItemType != HID_REPORT_ITEM_In)
			continue;
		if (item->Attributes.Usage.Page != usagePage)
			continue;
		if (item->Attributes.Usage.Usage != usage)
			continue;
		// When the device uses report IDs, only match the right report.
		if (info->UsingReportIDs && item->ReportID != reportId)
			continue;
		return item;
	}
	return nullptr;
}

static HID_ReportItem_t* FindButtonItem(
	HID_ReportInfo_t* info,
	uint8_t buttonIdx,
	uint8_t reportId)
{
	// HID button usages are 1-based
	return FindItemByUsage(info, HID_USAGE_PAGE_BUTTON, buttonIdx + 1, reportId);
}

// Find the report ID that carries gamepad information
// Returns 0 when the device doesn't use report IDs.
static uint8_t FindGamepadReportId(HID_ReportInfo_t* info)
{
	if (!info->UsingReportIDs)
		return 0;

	for (HID_ReportItem_t* item = info->FirstReportItem; item; item = item->Next) {
		if (item->ItemType != HID_REPORT_ITEM_In)
			continue;
		if (item->Attributes.Usage.Page != HID_USAGE_PAGE_GENERIC_DESKTOP)
			continue;
		uint16_t u = item->Attributes.Usage.Usage;
		if (u >= HID_USAGE_AXIS_X && u <= HID_USAGE_AXIS_RZ)
			return item->ReportID;
	}
	return 0;
}

void SendControlRequest(
	deviceHandle* deviceHandle,
	UsbControlTrb* controlTrb,
	uint8_t bmRequestType,
	uint8_t bRequest,
	uint16_t wValue,
	uint16_t wIndex,
	uint16_t wLength,
	void* data,
	DWORD completionCallback)
{
	controlTrb->packet.bmRequestType = bmRequestType;
	controlTrb->packet.bRequest = bRequest;
	controlTrb->packet.wValue = swap_endianness_16(wValue);
	controlTrb->packet.wIndex = swap_endianness_16(wIndex);
	controlTrb->packet.wLength = swap_endianness_16(wLength);
	controlTrb->trb.buffer = data;
	controlTrb->trb.length = wLength;
	controlTrb->trb.flags = 1;
	controlTrb->trb.callback = completionCallback;
	controlTrb->trb.savedEndpoint = controlTrb->trb.endpoint;
	UsbdQueueAsyncTransfer(deviceHandle, controlTrb);
}

void SendInterruptRequest(
	deviceHandle* deviceHandle,
	UsbTrb* interruptTrb,
	void* data,
	uint32_t length,
	DWORD completionCallback)
{
	interruptTrb->buffer = data;
	interruptTrb->length = length;
	interruptTrb->flags = 1;
	interruptTrb->callback = completionCallback;
	interruptTrb->savedEndpoint = interruptTrb->endpoint;
	UsbdQueueAsyncTransfer(deviceHandle, interruptTrb);
}

int32_t setConfigurationComplete(DWORD deviceHandle, int32_t status) {
	HidControllerExtension* controllerDriver = (HidControllerExtension*)((BYTE*)deviceHandle - 36);
	DbgPrint("EINTIM: Control transfer completed.\n");

	if (g_InitState == InitState::INIT_SET_CONFIGURATION) {
		g_InitState = InitState::INIT_GET_HID_DESCRIPTOR;
		DbgPrint("EINTIM: Init Stage 1. SET_CONFIGURATION completed successfully\r\n");
		SendControlRequest(
			controllerDriver->deviceHandle,
			&controllerDriver->controlTrb,
			0x81,
			0x06,
			0x2100,
			0x0000,
			sizeof(usb_hid_descriptor),
			&hidDescriptorBuffer,
			(DWORD)setConfigurationComplete);
	}
	else if (g_InitState == InitState::INIT_GET_HID_DESCRIPTOR) {
		hidDescriptorBuffer.wDescriptorLength = swap_endianness_16(hidDescriptorBuffer.wDescriptorLength);
		DbgPrint("EINTIM: Init Stage 2. Get hid descriptor: %x:%x completed successfully\r\n",
			hidDescriptorBuffer.bLength, hidDescriptorBuffer.wDescriptorLength);
		g_InitState = InitState::INIT_GET_REPORT_DESCRIPTOR;

		reportDescriptorBuffer = calloc(1, hidDescriptorBuffer.wDescriptorLength);

		SendControlRequest(
			controllerDriver->deviceHandle,
			&controllerDriver->controlTrb,
			0x81,
			0x06,
			0x2200,
			0x0000,
			hidDescriptorBuffer.wDescriptorLength,
			reportDescriptorBuffer,
			(DWORD)setConfigurationComplete);
	}
	else if (g_InitState == InitState::INIT_GET_REPORT_DESCRIPTOR) {
		DbgPrint("EINTIM: Init Stage 3. INIT_GET_REPORT_DESCRIPTOR completed successfully %x\r\n",
			*(DWORD*)reportDescriptorBuffer);

		// Parse HID descriptor
		HID_ReportInfo_t* reportInfo = nullptr;
		uint8_t parseResult = USB_ProcessHIDReport(
			(const uint8_t*)reportDescriptorBuffer,
			hidDescriptorBuffer.wDescriptorLength,
			&reportInfo);

		if (parseResult != HID_PARSE_Successful || !reportInfo) {
			DbgPrint("EINTIM: Failed to parse HID descriptor: error %d\r\n", parseResult);
			g_InitState = InitState::INIT_FAILED;
			free(reportDescriptorBuffer);
			return -1;
		}

		g_InitState = InitState::INIT_DONE;

		c.reportInfo = reportInfo;
		c.reportId = FindGamepadReportId(reportInfo);

		DbgPrint("EINTIM: Parsed descriptor. UsingReportIDs: %d, Report ID: %d\r\n",
			(int)reportInfo->UsingReportIDs, c.reportId);

		free(reportDescriptorBuffer);

		usb_endpoint_descriptor* endpoint_descriptor = UsbdGetEndpointDescriptor(
			controllerDriver->deviceHandle, 0, USB_ENDPOINT_TYPE_INTERRUPT, USB_DIRECTION_IN);

		status = UsbdOpenEndpoint(
			controllerDriver->deviceHandle,
			3,
			endpoint_descriptor->bEndpointAddress,
			swap_endianness_16(endpoint_descriptor->wMaxPacketSize) & 0x7FF,
			endpoint_descriptor->bInterval,
			(DWORD*)&controllerDriver->interruptTrb);

		if (NT_ERROR(status)) {
			DbgPrint("EINTIM: Failed to open interrupt endpoint %x!\n", status);
			return status;
		}

		uint16_t pktSize = swap_endianness_16(endpoint_descriptor->wMaxPacketSize) & 0x7FF;
		c.reportData = malloc(pktSize * 2);
		memset(c.reportData, 0, pktSize * 2);

		controllerDriver->interruptTrb.savedEndpoint = controllerDriver->interruptTrb.endpoint; 
		controllerDriver->interruptTrb.length = pktSize;
		controllerDriver->interruptTrb.callback = (DWORD)interruptHandler;
		controllerDriver->interruptTrb.buffer = c.reportData;

		c.controllerDriver = controllerDriver;

		uint8_t  userIndex = -1;
		uint32_t context = 0x0000000010000005 + globalIndex;
		XamUserBindDeviceCallback(0xa7553952 + globalIndex, context, 0, false, &userIndex);
		c.userIndex = userIndex;
		c.deviceContext = context;
		connectedControllers[globalIndex] = c;

		DbgPrint("EINTIM: Registered virtual controller inside XAM with index: %d.\n", userIndex);

		return UsbdQueueAsyncTransfer(controllerDriver->deviceHandle, &controllerDriver->interruptTrb);
	}

	return 0;
}

uint8_t NormalizeHat(int32_t v) {
	if (v >= 0 && v <= 7)
		return (uint8_t)v;

	if (v == 0xFF || v > 7)
		return HatSwitch::HAT_NEUTRAL;

	return HatSwitch::HAT_NEUTRAL;
}

HID_ReportItem_t* FindHatItem(HID_ReportInfo_t* info, uint8_t reportId) {
	for (HID_ReportItem_t* item = info->FirstReportItem; item; item = item->Next) {
		if (item->ItemType != HID_REPORT_ITEM_In)
			continue;

		if (item->Attributes.Usage.Page != HID_USAGE_PAGE_GENERIC_DESKTOP)
			continue;

		if (item->Attributes.Usage.Usage != HID_USAGE_HAT_SWITCH)
			continue;

		if (info->UsingReportIDs && item->ReportID != reportId)
			continue;

		int32_t min = item->Attributes.Logical.Minimum;
		int32_t max = item->Attributes.Logical.Maximum;

		if (max - min > 16) // hats are never huge ranges
			continue;

		return item;
	}

	return nullptr;
}

void HidFillButtonsReport(
	const uint8_t* payload,
	HID_ReportInfo_t* info,
	ButtonsReport* out,
	uint8_t           reportId)
{
	// Axes
	static const struct { uint16_t usage; int16_t ButtonsReport::* field; } kAxisMap[] = {
		{ HID_USAGE_AXIS_X,  &ButtonsReport::x  },
		{ HID_USAGE_AXIS_Y,  &ButtonsReport::y  },
		{ HID_USAGE_AXIS_Z,  &ButtonsReport::z  },
		{ HID_USAGE_AXIS_RX, &ButtonsReport::rx },
		{ HID_USAGE_AXIS_RY, &ButtonsReport::ry },
		{ HID_USAGE_AXIS_RZ, &ButtonsReport::rz },
	};

	for (uint8_t i = 0; i < sizeof(kAxisMap) / sizeof(kAxisMap[0]); i++) {
		HID_ReportItem_t* item = FindItemByUsage(info, HID_USAGE_PAGE_GENERIC_DESKTOP,
			kAxisMap[i].usage, reportId);

		if (!item || !USB_GetHIDReportItemInfo(reportId, payload, item))
			continue;

		int32_t logMin = (int32_t)item->Attributes.Logical.Minimum;
		int32_t logMax = (int32_t)item->Attributes.Logical.Maximum;
		int32_t raw = (int32_t)item->Value;

		if (logMax > logMin) {
			if (raw < logMin) raw = logMin;
			if (raw > logMax) raw = logMax;

			int64_t numerator = (int64_t)(raw - logMin) * 65535;
			int32_t denominator = (logMax - logMin);

			int32_t scaled = (int32_t)((numerator + denominator / 2) / denominator);
			out->*kAxisMap[i].field = (int16_t)(scaled - 32768);
		}
		else {
			out->*kAxisMap[i].field = (int16_t)raw;
		}
	}

	// Hat switch
	HID_ReportItem_t* hatItem = FindHatItem(info, reportId);
	if (hatItem && USB_GetHIDReportItemInfo(reportId, payload, hatItem)) {
		out->has_hat_switch = true;
		out->hatSwitch = NormalizeHat(hatItem->Value);
	} else {
		out->has_hat_switch = false;
	}

	// Buttons
	static const struct { uint8_t idx; uint8_t ButtonsReport::* field; } kButtonMap[] = {
		{ 0,  &ButtonsReport::cross    },
		{ 1,  &ButtonsReport::circle   },
		{ 2,  &ButtonsReport::square   },
		{ 3,  &ButtonsReport::triangle },
		{ 4,  &ButtonsReport::l1       },
		{ 5,  &ButtonsReport::r1       },
		{ 6,  &ButtonsReport::l2       },
		{ 7,  &ButtonsReport::r2       },
		{ 8,  &ButtonsReport::back     },
		{ 9,  &ButtonsReport::start    },
		{ 10, &ButtonsReport::l3       },
		{ 11, &ButtonsReport::r3       },
		{ 12, &ButtonsReport::xbox     },
	};

	for (uint8_t i = 0; i < sizeof(kButtonMap) / sizeof(kButtonMap[0]); i++) {
		HID_ReportItem_t* item = FindButtonItem(info, kButtonMap[i].idx, reportId);
		if (item && USB_GetHIDReportItemInfo(reportId, payload, item))
			out->*kButtonMap[i].field = (uint8_t)item->Value;
	}
}

int32_t noopCompleteHandler(DWORD deviceHandle, int32_t status) {
	return 0;
}


int interruptHandler(DWORD deviceHandle, int32_t a2) {
	HidControllerExtension* driverExtension = (HidControllerExtension*)((deviceHandle - 4));
	Report* report = (Report*)driverExtension->interruptTrb.buffer;

	if (!driverExtension || !driverExtension->deviceHandle ||
		!driverExtension->deviceHandle->driver ||
		driverExtension->deviceHandle->driver->cleanupDone)
		return 0;

	int index = -1;
	for (int i = 0; i < (sizeof(connectedControllers) / sizeof(Controller)); i++) {
		if (connectedControllers[i].controllerDriver == driverExtension) {
			index = i;
			break;
		}
	}

	// this handshake works fine, but currently the actual input is borked because of a nintendo pro controller firmware bug
	// https://gbatemp.net/threads/reverse-engineering-the-switch-pro-controller-wired-mode.475226/
	/*
	"WARNING: The HID descriptor does not match the data in the controller payload at all. My guess is it's just the Bluetooth HID descriptor c/p over. Because of that, if you enable the controller on Windows by poking that enable interrupt packet with your favorite USB tool, Windows will go crazy trying to interpret the packets it gets. I now have this on-screen controller keyboard I don't know how to get rid of."
	*/
	// Will need to hardcode a special case for it later
	if (NeedsNintendoHandshake(connectedControllers[index].vendorId, connectedControllers[index].productId) && connectedControllers[index].nintendo_handshake_state != DONE) {
		DbgPrint("EINTIM: Gotta do nintendo handshake for this one!(fuck them)\r\n");
		if (connectedControllers[index].nintendo_handshake_state == INITIAL) {
			usb_endpoint_descriptor* endpoint_descriptor = UsbdGetEndpointDescriptor(
				driverExtension->deviceHandle, 0,
				USB_ENDPOINT_TYPE_INTERRUPT, USB_DIRECTION_OUT);

			if (!endpoint_descriptor) {
				DbgPrint("EINTIM: Failed to find output descriptor\r\n");
				return -1;
			}

			NTSTATUS status = UsbdOpenEndpoint(
				driverExtension->deviceHandle,
				3,
				endpoint_descriptor->bEndpointAddress,
				swap_endianness_16(endpoint_descriptor->wMaxPacketSize) & 0x7FF,
				endpoint_descriptor->bInterval,
				(DWORD*)&connectedControllers[index].interruptTrb);

			if (NT_ERROR(status)) {
				DbgPrint("EINTIM: Failed to open interrupt OUT endpoint %x!\n", status);
				return status;
			}
			connectedControllers[index].nintendo_handshake_state = HANDSHAKE;
			SendInterruptRequest(driverExtension->deviceHandle, &connectedControllers[index].interruptTrb, (void*)nintendo_handshake, sizeof(nintendo_handshake), (DWORD)noopCompleteHandler);
		}

		else if (connectedControllers[index].nintendo_handshake_state == HANDSHAKE) {
			connectedControllers[index].nintendo_handshake_state = DONE;
			SendInterruptRequest(driverExtension->deviceHandle, &connectedControllers[index].interruptTrb, (void*)hid_only_mode, sizeof(hid_only_mode), (DWORD)noopCompleteHandler);
		}
	}

	bool hasReportId = connectedControllers[index].reportInfo &&
		connectedControllers[index].reportInfo->UsingReportIDs; 
	if (report->reportId == connectedControllers[index].reportId || !hasReportId) {
		ButtonsReport buttonReport;
		memset(&buttonReport, 0, sizeof(ButtonsReport));

		const uint8_t* payload = (const uint8_t*)report;
		
		// Check if correct report ID, skip report ID byte for parsing if present
		if (hasReportId) {
			if (payload[0] != connectedControllers[index].reportId)
				return UsbdQueueAsyncTransfer(driverExtension->deviceHandle,
					&driverExtension->interruptTrb);

			payload++;
		}

		HidFillButtonsReport(
			payload,
			connectedControllers[index].reportInfo,  
			&buttonReport,
			connectedControllers[index].reportId);

		connectedControllers[index].currentState = buttonReport;
	}

	return UsbdQueueAsyncTransfer(driverExtension->deviceHandle, &driverExtension->interruptTrb);
}


int HidRemoveDeviceHook(deviceHandle* deviceHandle2) {
	bool found = false;
	int index = 0;
	for (int i = 0; i < (sizeof(connectedControllers) / sizeof(Controller)); i++) {
		if (connectedControllers[i].deviceHandle == deviceHandle2) {
			found = true;
			index = i;
			break;
		}
	}

	if (!found) {
		DbgPrint("EINTIM: Returning original for handle %p\n", deviceHandle2);
		return HidRemoveDeviceDetour.GetOriginal<decltype(&HidRemoveDeviceHook)>()(deviceHandle2);
	}

	DbgPrint("EINTIM: Removing controller with handle %p\n", deviceHandle2);

	if (!deviceHandle2->driver->cleanupDone) {
		deviceHandle2->driver->cleanupDone = 1;
		connectedControllers[index].controllerDriver = nullptr;

		// Free HID report info for this controller slot
		if (connectedControllers[index].reportInfo) {
			USB_FreeReportInfo(connectedControllers[index].reportInfo);
			connectedControllers[index].reportInfo = nullptr;
		}

		// Clean up is currently broken, so it leaks up to 1kb of memory on every controller reconnect
		/*
		UsbdQueueCloseEndpoint(deviceHandle2, &deviceHandle2->driver->interruptEndpoint);
		UsbdQueueCloseDefaultEndpoint(deviceHandle2, &deviceHandle2->driver->defaultEndpoint);
		UsbdRemoveDeviceComplete(deviceHandle2);
		delete deviceHandle2->driver;*/
		deviceHandle2->driver = nullptr;
		free(connectedControllers[index].reportData);
		DbgPrint("EINTIM: Removed controller with handle %p\n", deviceHandle2);
		XamUserBindDeviceCallback(0xa7553952 + index, 0x0000000010000005 + index, 0, true, 0);
		DbgPrint("EINTIM: Removed virtual controller from XAM.\n");
		return 0;
	}
}

int reportData = 0;
int HidAddDeviceHook(deviceHandle* deviceHandle) {
	DbgPrint("EINTIM: HID add device %p\n", deviceHandle);
	usb_device_descriptor* device_descriptor = UsbdGetDeviceDescriptor(deviceHandle);
	usb_interface_descriptor* interface_descriptor = UsbdGetInterfaceDescriptor(deviceHandle);

	uint16_t vendorId = swap_endianness_16(device_descriptor->idVendor);
	uint16_t productId = swap_endianness_16(device_descriptor->idProduct);

	int speed = UsbdGetDeviceSpeed(deviceHandle);
	bool isOhci = speed == 0;

	DbgPrint("EINTIM: IS USB1.0: %d\n", isOhci);
	DbgPrint("EINTIM: USB device descriptor Pointer: %p\n", device_descriptor);
	DbgPrint("EINTIM: HID device vendor id: %x, product id: %x\n", vendorId, productId);

	if (interface_descriptor->bInterfaceClass == 0x03 &&
		interface_descriptor->bInterfaceSubClass == 0 &&
		interface_descriptor->bInterfaceProtocol == 0) {
		DbgPrint("EINTIM: Controller detected. Initialising custom handler.\n");
		int index = -1;
		for (int i = 0; i < (sizeof(connectedControllers) / sizeof(Controller)); i++) {
			if (!connectedControllers[i].controllerDriver) {
				DbgPrint("Assigning controller to index %d\n", i);
				index = i;
				break;
			}
		}

		if (index == -1) {
			DbgPrint("EINTIM: No free index!\n");
			return HidAddDeviceDetour.GetOriginal<decltype(&HidAddDeviceHook)>()(deviceHandle);
		}
		globalIndex = index;

		c = Controller();
		memset(&c, 0, sizeof(Controller));
		c.packetNumber = 0;
		c.reportInfo = nullptr;   // will be filled in INIT_GET_REPORT_DESCRIPTOR
		c.vendorId = vendorId;
		c.productId = productId;
		c.nintendo_handshake_state = NINTENDO_HANDSHAKE_STATE::INITIAL;

		HidControllerExtension* controllerDriver = new HidControllerExtension();
		c.deviceHandle = deviceHandle;

		controllerDriver->deviceType = 0;
		deviceHandle->driver = controllerDriver;
		controllerDriver->deviceHandle = deviceHandle;
		controllerDriver->interruptTrb.flags = 1;

		UsbdAddDeviceComplete(deviceHandle, 0);

		NTSTATUS status = UsbdOpenDefaultEndpoint(deviceHandle, (DWORD*)&controllerDriver->controlTrb);
		if (NT_ERROR(status)) {
			DbgPrint("EINTIM: Failed to open control endpoint %x!\n", status);
			return status;
		}

		g_InitState = InitState::INIT_SET_CONFIGURATION;
		SendControlRequest(
			controllerDriver->deviceHandle,
			&controllerDriver->controlTrb,
			0x00,
			0x09,
			1, 0, 0,
			nullptr,
			(DWORD)setConfigurationComplete);

		return 0;
	}

	DbgPrint("EINTIM: Unrelated USB Device. Calling original...\n");
	return HidAddDeviceDetour.GetOriginal<decltype(&HidAddDeviceHook)>()(deviceHandle);
}

DWORD XamInputSetStateHook(DWORD user, DWORD flags, XINPUT_STATE* pInputState, BYTE bAmplitude, BYTE bFrequency, BYTE bOffset) {
	DWORD status = XamInputSetStateDetour.GetOriginal<decltype(&XamInputSetStateHook)>()(user, flags, pInputState, bAmplitude, bFrequency, bOffset);

	if ((user & 0xFF) == 0xFF)
		user = 0;

	if (status == ERROR_DEVICE_NOT_CONNECTED) {
		Controller* c = nullptr;
		for (int i = 0; i < (sizeof(connectedControllers) / sizeof(Controller)); i++) {
			if (connectedControllers[i].controllerDriver &&
				connectedControllers[i].userIndex == user) {
				c = &connectedControllers[i];
				break;
			}
		}
		if (!c)
			return status;
		return ERROR_SUCCESS;
	}

	return status;
}

DWORD XamInputGetCapabilitiesExHook(DWORD unk, DWORD user, DWORD flags, XINPUT_CAPABILITIES_EX* capabilities) {
	DWORD status = XamInputGetCapabilitiesDetour.GetOriginal<decltype(&XamInputGetCapabilitiesExHook)>()(unk, user, flags, capabilities);

	if ((user & 0xFF) == 0xFF)
		user = 0;

	if (!capabilities)
		return status;

	if (status == ERROR_DEVICE_NOT_CONNECTED) {
		Controller* c = nullptr;
		for (int i = 0; i < (sizeof(connectedControllers) / sizeof(Controller)); i++) {
			if (connectedControllers[i].controllerDriver &&
				connectedControllers[i].userIndex == user) {
				c = &connectedControllers[i];
				break;
			}
		}
		if (!c)
			return status;

		capabilities->Type = XINPUT_DEVTYPE_GAMEPAD;
		capabilities->SubType = XINPUT_DEVSUBTYPE_GAMEPAD;
		capabilities->Flags = 0;

		XINPUT_STATE state;
		memset(&state, 0, sizeof(XINPUT_STATE));
		XInputGetState(user, &state);
		capabilities->Gamepad = state.Gamepad;
		capabilities->Vibration.wLeftMotorSpeed = 0;
		capabilities->Vibration.wRightMotorSpeed = 0;
		return ERROR_SUCCESS;
	}
}

NTSTATUS XInputdReadStateHook(DWORD dwDeviceContext, PDWORD pdwPacketNumber, PXINPUT_GAMEPAD pInputData, PBOOL unk) {
	if (dwDeviceContext >= 0x0000000010000005) {
		if (!pInputData)
			return ERROR_INVALID_PARAMETER;

		static DWORD lastPressTime = 0;
		static const DWORD cooldownDuration = 1000;

		ButtonsReport b;
		Controller* c = nullptr;
		for (int i = 0; i < (sizeof(connectedControllers) / sizeof(Controller)); i++) {
			if (connectedControllers[i].controllerDriver &&
				connectedControllers[i].deviceContext == dwDeviceContext) {
				c = &connectedControllers[i];
				b = connectedControllers[i].currentState;
				break;
			}
		}

		if (!c)
			return ERROR_INVALID_PARAMETER;

		if (b.xbox) {
			DWORD now = GetTickCount();
			if (now - lastPressTime >= cooldownDuration) {
				lastPressTime = now;
				XamInputSendXenonButtonPress(c->userIndex);
			}
		}

		if (b.cross)    pInputData->wButtons |= XINPUT_GAMEPAD_A;
		if (b.circle)   pInputData->wButtons |= XINPUT_GAMEPAD_B;
		if (b.triangle) pInputData->wButtons |= XINPUT_GAMEPAD_Y;
		if (b.square)   pInputData->wButtons |= XINPUT_GAMEPAD_X;
		if (b.start)    pInputData->wButtons |= XINPUT_GAMEPAD_START;
		if (b.back)     pInputData->wButtons |= XINPUT_GAMEPAD_BACK;
		if (b.r3)       pInputData->wButtons |= XINPUT_GAMEPAD_RIGHT_THUMB;
		if (b.l3)       pInputData->wButtons |= XINPUT_GAMEPAD_LEFT_THUMB;
		if (b.l1)       pInputData->wButtons |= XINPUT_GAMEPAD_LEFT_SHOULDER;
		if (b.r1)       pInputData->wButtons |= XINPUT_GAMEPAD_RIGHT_SHOULDER;

		if (b.has_hat_switch) {
			switch (b.hatSwitch) {
			case HatSwitch::HAT_UP:
				pInputData->wButtons |= XINPUT_GAMEPAD_DPAD_UP;
				break;
			case HatSwitch::HAT_UP_RIGHT:
				pInputData->wButtons |= XINPUT_GAMEPAD_DPAD_UP | XINPUT_GAMEPAD_DPAD_RIGHT;
				break;
			case HatSwitch::HAT_RIGHT:
				pInputData->wButtons |= XINPUT_GAMEPAD_DPAD_RIGHT;
				break;
			case HatSwitch::HAT_DOWN_RIGHT:
				pInputData->wButtons |= XINPUT_GAMEPAD_DPAD_DOWN | XINPUT_GAMEPAD_DPAD_RIGHT;
				break;
			case HatSwitch::HAT_DOWN:
				pInputData->wButtons |= XINPUT_GAMEPAD_DPAD_DOWN;
				break;
			case HatSwitch::HAT_DOWN_LEFT:
				pInputData->wButtons |= XINPUT_GAMEPAD_DPAD_DOWN | XINPUT_GAMEPAD_DPAD_LEFT;
				break;
			case HatSwitch::HAT_LEFT:
				pInputData->wButtons |= XINPUT_GAMEPAD_DPAD_LEFT;
				break;
			case HatSwitch::HAT_UP_LEFT:
				pInputData->wButtons |= XINPUT_GAMEPAD_DPAD_UP | XINPUT_GAMEPAD_DPAD_LEFT;
				break;
			case HatSwitch::HAT_NEUTRAL:
				break;
			}
		}
		else {
			if(b.dpad_left)
				pInputData->wButtons |= XINPUT_GAMEPAD_DPAD_LEFT;
			if(b.dpad_right)
				pInputData->wButtons |= XINPUT_GAMEPAD_DPAD_RIGHT;
			if(b.dpad_up)
				pInputData->wButtons |= XINPUT_GAMEPAD_DPAD_UP;
			if(b.dpad_down)
				pInputData->wButtons |= XINPUT_GAMEPAD_DPAD_DOWN;
		}
		
		pInputData->sThumbRX = b.z;
		pInputData->sThumbRY = b.rz;
		pInputData->sThumbLX = b.x;
		pInputData->sThumbLY = b.y;
		pInputData->bLeftTrigger = b.rx ? b.rx : (b.l2 ? 255 : 0);
		pInputData->bRightTrigger = b.ry ? b.ry : (b.r2 ? 255 : 0);

		if (pdwPacketNumber)
			*pdwPacketNumber = ++c->packetNumber;
		if (unk)
			*unk = FALSE;

		return STATUS_SUCCESS;
	}
	return XInputdReadStateDetour.GetOriginal<decltype(&XInputdReadStateHook)>()(dwDeviceContext, pdwPacketNumber, pInputData, unk);
}


void* XamInputSetState = nullptr;
void* XamInputGetCapabilitiesEx = nullptr;
void* XInputdReadStatePtr = nullptr;
bool isDevkit = true;
DWORD UsbPhysicalPage = 0;
bool initFunctionPointers() {
	isDevkit = *(uint32_t*)(0x8010D334) == 0x00000000;
	HANDLE kernelHandle = GetModuleHandleA("xboxkrnl.exe");

	if (!kernelHandle) {
		DbgPrint("EINTIM: COULDNT GET KERNEL HANDLE!\n");
		return false;
	}

	HANDLE xamHandle = GetModuleHandleA("xam.xex");

	XexGetProcedureAddress(kernelHandle, 759, &UsbdGetDeviceDescriptor);
	XexGetProcedureAddress(kernelHandle, 744, &UsbdGetEndpointDescriptor);
	XexGetProcedureAddress(kernelHandle, 740, &UsbdAddDeviceComplete);
	XexGetProcedureAddress(kernelHandle, 746, &UsbdOpenDefaultEndpoint);
	XexGetProcedureAddress(kernelHandle, 747, &UsbdOpenEndpoint);
	XexGetProcedureAddress(kernelHandle, 742, &UsbdGetDeviceSpeed);
	XexGetProcedureAddress(kernelHandle, 748, &UsbdQueueAsyncTransfer);
	XexGetProcedureAddress(kernelHandle, 750, &UsbdQueueCloseEndpoint);
	XexGetProcedureAddress(kernelHandle, 749, &UsbdQueueCloseDefaultEndpoint);
	XexGetProcedureAddress(kernelHandle, 751, &UsbdRemoveDeviceComplete);
	XexGetProcedureAddress(kernelHandle, 189, &MmFreePhysicalMemory);
	XexGetProcedureAddress(kernelHandle, 486, &XInputdReadStatePtr);

	XexGetProcedureAddress(xamHandle, 685, &XamInputGetCapabilitiesEx);
	XexGetProcedureAddress(xamHandle, 402, &XamInputSetState);

	if (isDevkit) {
		DbgPrint("EINTIM: Running in devkit mode\n");
		UsbdGetInterfaceDescriptor = (usb_interface_descriptor_func_t)0x8010D2D0; // 89 43 ? ? 3D 60 ? ? 89 2D ? ? 39 6B ? ? 55 4A FF 3A 2B 09 ? ? 7D 6A 58 2E ? ? ? ? ? ? ? ? 89 4D ? ? 2B 0A ? ? ? ? ? ? ? ? ? ? 81 4B ? ? 7F 03 50 40 ? ? ? ? ? ? ? ? A1 4B very bad direct signature. XREF sig: 89 63 ? ? 38 A1
		XamUserBindDeviceCallback = (xam_user_bind_device_callback_func_t)0x817A34B8; // 7C 8B 23 78 7C A4 2B 78 54 CA 06 3F
		UsbdPowerDownNotification = (usbd_powerdown_notification_func_t)0x8010E140; // argument to last function call in UsbdDriverEntry
		UsbdDriverEntry = (usbd_powerdown_notification_func_t)0x8010DE48; // 7D 88 02 A6 ? ? ? ? 94 21 ? ? 3C 80 ? ? 38 A0 

		//Remove two usb related bugchecks to allow reinitialisation of the usb driver
		*(DWORD*)0x80116298 = 0x48000018;
		*(DWORD*)0x801132A4 = 0x48000018;

		//DEVKIT only: Remove assertions(Microsoft did not think that we'd come and reset the usb driver, never let them know your next move typa shit)
		/*
		*(DWORD*)0x80096B84 = 0x60000000;
		*(DWORD*)0x80095F6C = 0x60000000;
		*(DWORD*)0x80116584 = 0x60000000;
		*(DWORD*)0x80116598 = 0x60000000;
		*/

		// Prevent double registration of Usbd handlers because the console wont shutdown cleanly otherwise
		* (DWORD*)0x8010E04C = 0x60000000;
		*(DWORD*)0x8010E05C = 0x60000000;
		UsbPhysicalPage = 0x8020A9B8;
	}
	else {
		DbgPrint("EINTIM: Running in retail mode\n");
		UsbdGetInterfaceDescriptor = (usb_interface_descriptor_func_t)0x800D8500; // 89 43 ? ? 3D 60 ? ? 39 6B ? ? 55 4A FF 3A 7D 6A 58 2E A1 4B
		XamUserBindDeviceCallback = (xam_user_bind_device_callback_func_t)0x816D9060; // 7C 8B 23 78 7C A4 2B 78 54 CA 06 3F
		UsbdPowerDownNotification = (usbd_powerdown_notification_func_t)0x800D8FC8; // argument to last function call in UsbdDriverEntry
		UsbdDriverEntry = (usbd_powerdown_notification_func_t)0x800D8D08; // 7D 88 02 A6 ? ? ? ? 94 21 ? ? 3C 80 ? ? 38 A0 

		//Remove two usb related bugchecks to allow reinitialisation of the usb driver
		*(DWORD*)0x800E05E4 = 0x48000018;
		*(DWORD*)0x800DD8E0 = 0x48000018;

		// Prevent double registration of Usbd handlers because the console wont shutdown cleanly otherwise
		*(DWORD*)0x800D8F00 = 0x60000000;
		*(DWORD*)0x800D8EF0 = 0x60000000;
		UsbPhysicalPage = 0x801A8098;
	}

	return true;
}

BOOL APIENTRY DllMain(HANDLE Handle, DWORD Reason, PVOID Reserved) {
	if (Reason == DLL_PROCESS_ATTACH) {
		if ((XboxKrnlVersion->Build != 17559 && XboxKrnlVersion->Build != 17489) || IsTrayOpen()) {
			DbgPrint("EINTIM: Only 17559 and 17489 dashboards are currently supported or the disk tray is open. Aborting launch...\n");
			return FALSE;
		}

		DbgPrint("EINTIM: HELLO from xbox 360 HID controller driver version 0.6 beta2\n");
		if (!initFunctionPointers())
			return FALSE;

		if (isDevkit) {
			HidAddDeviceDetour = Detour((void*)0x8011AE38, (void*)HidAddDeviceHook); // 7D 88 02 A6 ? ? ? ? 94 21 ? ? 7C 7C 1B 78 ? ? ? ? 7C 7F 1B 79
			HidRemoveDeviceDetour = Detour((void*)0x8011ADF8, (void*)HidRemoveDeviceHook); // 81 63 ? ? 39 40 ? ? 39 20 ? ? 99 4B
		}
		else {
			HidAddDeviceDetour = Detour((void*)0x800E4D68, (void*)HidAddDeviceHook); // 7D 88 02 A6 ? ? ? ? 94 21 ? ? 7C 7B 1B 78 ? ? ? ? 7C 7F 1B 79
			HidRemoveDeviceDetour = Detour((void*)0x800E4D28, (void*)HidRemoveDeviceHook); // 81 63 ? ? 39 40 ? ? 39 20 ? ? 99 4B
		}

		HidAddDeviceDetour.Install();
		HidRemoveDeviceDetour.Install();

		XamInputGetCapabilitiesDetour = Detour(XamInputGetCapabilitiesEx, (void*)XamInputGetCapabilitiesExHook);
		XamInputSetStateDetour = Detour(XamInputSetState, (void*)XamInputSetStateHook);
		XInputdReadStateDetour = Detour(XInputdReadStatePtr, (void*)XInputdReadStateHook);

		XamInputSetStateDetour.Install();
		XamInputGetCapabilitiesDetour.Install();
		XInputdReadStateDetour.Install();

		DbgPrint("EINTIM: Resetting USB driver!\n");
		UsbdPowerDownNotification();
		//For some reason microsoft doesnt clean up this page by themselves in the shutdown notification, so ill do it for them, call me mr nice guy :)
		MmFreePhysicalMemory(0, *(DWORD*)UsbPhysicalPage);
		DbgPrint("EINTIM: USB driver shutdown complete.\n");
		UsbdDriverEntry();
		DbgPrint("EINTIM: USB driver reset complete.\n");
		DbgPrint("EINTIM: Hooks installed\n");
	}
	return TRUE;
}