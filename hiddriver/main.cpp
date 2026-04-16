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
#include "usb.h"
#include "mapping.h"

Detour HidAddDeviceDetour;
Detour HidRemoveDeviceDetour;
Detour XamInputSetStateDetour;
Detour XamInputGetCapabilitiesDetour;
Detour XInputdReadStateDetour;
#define XNOTIFYUI_CUSTOM (XNOTIFYQUEUEUI_TYPE)80
uint16_t swap_endianness_16(uint16_t val) {
	return (val >> 8) | (val << 8);
}

BOOL IsTrayOpen() {
	BYTE Input[0x10] = { 0 }, Output[0x10] = { 0 };
	Input[0] = 0xA;
	HalSendSMCMessage(Input, Output);
	return (Output[1] == 0x60);
}

// This console likes to kill non system threads on title switches
HANDLE MakeThread(LPTHREAD_START_ROUTINE Address, PVOID arg) {
	HANDLE Handle = 0;
	ExCreateThread(&Handle, 0, 0, XapiThreadStartup, Address, arg, (EX_CREATE_FLAG_SUSPENDED | EX_CREATE_FLAG_SYSTEM | 0x18000424));
	XSetThreadProcessor(Handle, 4);
	SetThreadPriority(Handle, THREAD_PRIORITY_NORMAL);
	ResumeThread(Handle);
	return Handle;
}

void XNotifyUI(XNOTIFYQUEUEUI_TYPE Type, PWCHAR String) { XNotifyQueueUI(Type, XUSER_INDEX_ANY, XNOTIFYUI_PRIORITY_DEFAULT, String, 0); }

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
	INIT_GET_PS_CAPABILITIES_DESCRIPTOR,
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

uint16_t clamp_u16(uint16_t val, uint16_t lo, uint16_t hi) {
	if (val < lo) return lo;
	if (val > hi) return hi;
	return val;
}

// Nintendo specific start
const uint16_t NINTENDO_VENDOR_ID = 0x057E;
const uint16_t SWITCH_PRO_PRODUCT_ID = 0x2009;

const unsigned char nintendo_handshake[2] = { 0x80, 0x02 };
const unsigned char hid_only_mode[2] = { 0x80, 0x04 };

#pragma pack(push, 1)
struct switch_pro_input_report {
	uint8_t  timer;
	uint8_t  battery_conn;   // upper nibble = battery, lower = connection type
	uint8_t  buttons_right;  // Y X B A, R_SR, R_SL, R, ZR
	uint8_t  buttons_mid;    // minus, plus, R_stick, L_stick, home, capture
	uint8_t  buttons_left;   // dpad down/up/right/left, L_SR, L_SL, L, ZL
	uint8_t  left_stick[3];  // 12-bit packed: lx in bits [11:0], ly in bits [23:12]
	uint8_t  right_stick[3]; // same packing for rx, ry
	uint8_t  vibrator;
	uint8_t  imu[48];        // 3 � (accel xyz + gyro xyz), each int16_t
};
#pragma pack(pop)

// buttons1
// Face buttons
#define SWITCH_BTN_Y        (1 << 0)
#define SWITCH_BTN_X        (1 << 1)
#define SWITCH_BTN_B        (1 << 2)
#define SWITCH_BTN_A        (1 << 3)

// Right shoulder cluster
#define SWITCH_BTN_R        (1 << 6)
#define SWITCH_BTN_ZR       (1 << 7)

// System buttons
#define SWITCH_BTN_MINUS    (1 << 8)
#define SWITCH_BTN_PLUS     (1 << 9)

// Sticks
#define SWITCH_BTN_R_STICK  (1 << 10)
#define SWITCH_BTN_L_STICK  (1 << 11)

// System
#define SWITCH_BTN_HOME     (1 << 12)
#define SWITCH_BTN_CAPTURE  (1 << 13)

// buttons2

#define SWITCH_DPAD_DOWN    (1 << 0)
#define SWITCH_DPAD_UP      (1 << 1)
#define SWITCH_DPAD_RIGHT   (1 << 2)
#define SWITCH_DPAD_LEFT    (1 << 3)

#define SWITCH_BTN_L        (1 << 6)
#define SWITCH_BTN_ZL       (1 << 7)

static uint16_t STICK_MIN = 500;
static uint16_t STICK_MAX = 3500;
static uint16_t STICK_CENTER = 2000;

int16_t normalize_stick(uint16_t raw) {
	raw = clamp_u16(raw, STICK_MIN, STICK_MAX);
	if (raw >= STICK_CENTER) {
		return (int16_t)((int32_t)(raw - STICK_CENTER) * 32767 / (STICK_CENTER - STICK_MIN));
	}
	else {
		return (int16_t)((int32_t)(STICK_CENTER - raw) * -32768 / (STICK_CENTER - STICK_MIN));
	}
};

bool NeedsNintendoHandshake(uint16_t vid, uint16_t pid) {
	if (vid != NINTENDO_VENDOR_ID) return false;
	return pid == SWITCH_PRO_PRODUCT_ID;
}


// nintendo specific end

// dualshock 3 specific start
const uint16_t SONY_VENDOR_ID = 0x054C;
const uint16_t DS3_PRODUCT_ID = 0x0268;
const unsigned char DS3_HANDSHAKE[4] = { 0x42, 0x0C, 0x00, 0x00 };

bool NeedsDualshock3Handshake(uint16_t vid, uint16_t pid) {
	if (vid != SONY_VENDOR_ID) return false;
	return pid == DS3_PRODUCT_ID;
}

enum DS3_FEATURE_VALUE
{
	Ds3FeatureDeviceAddress = 0x03F2,
	Ds3FeatureStartDevice = 0x03F4,
	Ds3FeatureHostAddress = 0x03F5

};
// dualshock 3 specific end
enum PS_FEATURE_VALUE
{
	PSFeatureCapabilities = 0x0303
};

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
	uint16_t sonyUsage;
	uint8_t subType;
	uint16_t flags;
	uint32_t packetNumber;
	HID_ReportInfo_t* reportInfo;
	uint8_t reportId;
	void* reportData;
	const HidDeviceMapping* map;

	// for nintendo specific handshake
	NINTENDO_HANDSHAKE_STATE nintendo_handshake_state;
	UsbTrb interruptTrb;
} __declspec(align(4));

struct MappingState {
	volatile bool active;
	volatile uint8_t pressedButtonIdx;
	volatile int16_t axisValues[6];
	uint8_t reportId;
	HID_ReportInfo_t* reportInfo;
	int controllerIndex;
	uint8_t availableButtons[256];
	uint8_t availableButtonCount;
	volatile uint8_t previousPressedButtonIdx;
	volatile uint32_t holdCount;
} __declspec(align(4));

Controller connectedControllers[4];
Controller c;
usb_hid_descriptor hidDescriptorBuffer;
uint8_t psFeatureBuffer[48];
int globalIndex = -1;
void* reportDescriptorBuffer;
MappingState g_mappingState;

int interruptHandler(DWORD deviceHandle, int32_t a2);

// Keep every IN report item; the driver uses all axes, the hat, and buttons.
bool CALLBACK_HIDParser_FilterHIDReportItem(HID_ReportItem_t* const CurrentItem) {
	return (CurrentItem->ItemType == HID_REPORT_ITEM_In || CurrentItem->ItemType == HID_REPORT_ITEM_Feature);
}

HID_ReportItem_t* FindItemByUsage(
	HID_ReportInfo_t* info,
	uint16_t usagePage,
	uint16_t usage,
	uint8_t  reportId) {
	for (HID_ReportItem_t* item = info->FirstReportItem; item; item = item->Next) {
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

HID_ReportItem_t* FindButtonItem(
	HID_ReportInfo_t* info,
	uint8_t buttonIdx,
	uint8_t reportId) {
	return FindItemByUsage(info, HID_USAGE_PAGE_BUTTON, buttonIdx + 1, reportId);
}

// Find the report ID that carries gamepad information
// Returns 0 when the device doesn't use report IDs.
uint8_t FindGamepadReportId(HID_ReportInfo_t* info) {
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

uint16_t FindSonyFeatureReport(HID_ReportInfo_t* info) {
	for (HID_ReportItem_t* item = info->FirstReportItem; item; item = item->Next) {
		if (item->Attributes.Usage.Page != HID_USAGE_PAGE_VENDOR)
			continue;
		uint16_t u = item->Attributes.Usage.Usage;
		if (u == HID_USAGE_PS3_CAPABILITIES || u == HID_USAGE_PS4_CAPABILITIES || u == HID_USAGE_PS5_CAPABILITIES)
			return u;
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
	DWORD completionCallback) {
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
	DWORD completionCallback) {
	interruptTrb->buffer = data;
	interruptTrb->length = length;
	interruptTrb->flags = 1;
	interruptTrb->callback = completionCallback;
	interruptTrb->savedEndpoint = interruptTrb->endpoint;
	UsbdQueueAsyncTransfer(deviceHandle, interruptTrb);
}

int32_t noopCompleteHandler(DWORD deviceHandle, int32_t status) {
	return 0;
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
		uint8_t parseResult = USB_ProcessHIDReport((const uint8_t*)reportDescriptorBuffer,
			hidDescriptorBuffer.wDescriptorLength,
			&reportInfo);

		if (parseResult != HID_PARSE_Successful || !reportInfo) {
			DbgPrint("EINTIM: Failed to parse HID descriptor: error %d\r\n", parseResult);
			g_InitState = InitState::INIT_FAILED;
			free(reportDescriptorBuffer);
			return -1;
		}


		c.reportInfo = reportInfo;
		c.reportId = FindGamepadReportId(reportInfo);
		c.sonyUsage = FindSonyFeatureReport(reportInfo);
		if (!c.map) {
			c.map = FindStaticSonyMapping(c.sonyUsage);
		}

		DbgPrint("EINTIM: Parsed descriptor. UsingReportIDs: %d, Report ID: %d, Sony Usage: %04x\r\n",
			(int)reportInfo->UsingReportIDs, c.reportId, c.sonyUsage);

		free(reportDescriptorBuffer);

		if (c.sonyUsage && c.sonyUsage != 0x2621) {
			DbgPrint("EINTIM: Sending PS4/5 capabilities request!\r\n");
			SendControlRequest(controllerDriver->deviceHandle,
				&controllerDriver->controlTrb,
				0x21,
				0x01, 
				PSFeatureCapabilities, 
				0, 
				sizeof(psFeatureBuffer), 
				(void*)psFeatureBuffer, 
				(DWORD)setConfigurationComplete);
			g_InitState = InitState::INIT_GET_PS_CAPABILITIES_DESCRIPTOR;
		} else {
			DbgPrint("EINTIM: parse done stage 1\r\n");
			g_InitState = InitState::INIT_DONE;
		}
	} else if (g_InitState == InitState::INIT_GET_PS_CAPABILITIES_DESCRIPTOR) {
		// parse the PS4/PS5 capabilities request and figure out the subtype
		switch (psFeatureBuffer[5]) {
			case 0x00:
				c.subType = XINPUT_DEVSUBTYPE_GAMEPAD;
				break;
			case 0x01:
				c.subType = XINPUT_DEVSUBTYPE_GUITAR;
				break;
			case 0x02:
				c.subType = XINPUT_DEVSUBTYPE_DRUM_KIT;
				break;
			case 0x04:
				c.subType = XINPUT_DEVSUBTYPE_DANCEPAD;
				break;
			case 0x06:
				c.subType = XINPUT_DEVSUBTYPE_WHEEL;
				break;
			case 0x07:
				c.subType = XINPUT_DEVSUBTYPE_ARCADE_STICK;
				break;
			case 0x08:
				c.subType = XINPUT_DEVSUBTYPE_FLIGHT_STICK;
				break;
			default:
				c.subType = XINPUT_DEVSUBTYPE_GAMEPAD;
				break;
		}
		DbgPrint("EINTIM: parse done stage 1\r\n");
		g_InitState = InitState::INIT_DONE;
	}

	if (g_InitState == InitState::INIT_DONE) {
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

		if (NeedsDualshock3Handshake(c.vendorId, c.productId)) {
			DbgPrint("EINTIM: Sending dualshock3 handshake!\r\n");
			SendControlRequest(controllerDriver->deviceHandle,
				&controllerDriver->controlTrb,
				0x21,
				0x09, 
				Ds3FeatureStartDevice, 
				0, 
				sizeof(DS3_HANDSHAKE), 
				(void*)DS3_HANDSHAKE, 
				(DWORD)noopCompleteHandler);
		}
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

void DiscoverAvailableButtons(HID_ReportInfo_t* info, uint8_t reportId,
                              uint8_t* outButtonIndices, uint8_t* outCount) {
	uint8_t count = 0;
	for (HID_ReportItem_t* item = info->FirstReportItem; item && count < 256; item = item->Next) {
		if (item->ItemType != HID_REPORT_ITEM_In)
			continue;
		if (item->Attributes.Usage.Page != HID_USAGE_PAGE_BUTTON)
			continue;
		if (info->UsingReportIDs && item->ReportID != reportId)
			continue;

		uint16_t usage = item->Attributes.Usage.Usage;
		if (usage >= 1 && usage <= 256) {
			uint8_t buttonIdx = usage - 1;
			outButtonIndices[count++] = buttonIdx;
		}
	}
	*outCount = count;
}

void DiscoverAvailableAxes(HID_ReportInfo_t* info, uint8_t reportId,
                           uint16_t* outAxisUsages, uint8_t* outCount) {
	uint8_t count = 0;
	for (HID_ReportItem_t* item = info->FirstReportItem; item && count < 6; item = item->Next) {
		if (item->ItemType != HID_REPORT_ITEM_In)
			continue;
		if (item->Attributes.Usage.Page != HID_USAGE_PAGE_GENERIC_DESKTOP)
			continue;
		if (info->UsingReportIDs && item->ReportID != reportId)
			continue;

		uint16_t usage = item->Attributes.Usage.Usage;
		if (usage >= HID_USAGE_AXIS_X && usage <= HID_USAGE_AXIS_RZ) {
			outAxisUsages[count++] = usage;
		}
	}
	*outCount = count;
}

void HidFillButtonsReport(
	const uint8_t* payload,
	HID_ReportInfo_t* info,
	ButtonsReport* out,
	uint8_t reportId,
	const HidDeviceMapping* map) {
	// Axes
	const auto* axisMap = map->axisMap;
	uint8_t axisCount = map->axisMapCount;

	for (uint8_t i = 0; i < axisCount; i++) {
		const auto& entry = axisMap[i];

		HID_ReportItem_t* item = FindItemByUsage(
			info,
			HID_USAGE_PAGE_GENERIC_DESKTOP,
			entry.usage,
			reportId
		);

		if (!item || !USB_GetHIDReportItemInfo(reportId, payload, item))
			continue;

		int32_t logMin = (int32_t)item->Attributes.Logical.Minimum;
		int32_t logMax = (int32_t)item->Attributes.Logical.Maximum;
		int32_t raw = (int32_t)item->Value;

		int32_t result = 0;

		if (logMax > logMin) {
			if (raw < logMin) raw = logMin;
			if (raw > logMax) raw = logMax;

			int64_t numerator = (int64_t)(raw - logMin) * 65535;
			int32_t denominator = (logMax - logMin);

			int32_t scaled = (int32_t)((numerator + denominator / 2) / denominator);
			result = scaled - 32768;
		}
		else {
			result = raw;
		}

		// apply inversion
		switch (entry.usage) {
		case HID_USAGE_AXIS_X:
			if (map->invert.invertX) result = -result;
			break;
		case HID_USAGE_AXIS_Y:
			if (map->invert.invertY) result = -result;
			break;
		case HID_USAGE_AXIS_Z:
			if (map->invert.invertZ) result = -result;
			break;
		case HID_USAGE_AXIS_RX:
			if (map->invert.invertRX) result = -result;
			break;
		case HID_USAGE_AXIS_RY:
			if (map->invert.invertRY) result = -result;
			break;
		case HID_USAGE_AXIS_RZ:
			if (map->invert.invertRZ) result = -result;
			break;
		}

		if (result > 32767) result = 32767; if (result < -32768) result = -32768;

		out->*entry.field = (int16_t)result;
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
	const auto* buttonMap = map->buttonMap;

	for (uint8_t i = 0; i < map->buttonMapCount; i++) {
		const auto& entry = buttonMap[i];

		HID_ReportItem_t* item = FindButtonItem(info, entry.idx, reportId);
		if (item && USB_GetHIDReportItemInfo(reportId, payload, item)) {
			out->*entry.field = (uint8_t)item->Value;
		}
	}
}

unsigned int __stdcall MappingThreadProc(void* param);
unsigned int __stdcall MappingManagerThreadProc(void* param){
	// This thread monitors for controllers needing mapping and spawns mapping threads
	while (true) {
		for (int i = 0; i < 4; i++) {
			// Check if controller exists, has reportInfo, but no mapping, and mapping not already in progress
			if (connectedControllers[i].controllerDriver &&
				connectedControllers[i].reportInfo &&
				!connectedControllers[i].map &&
				!g_mappingState.active) {

				DbgPrint("EINTIM: Starting mapping for controller %d (VID:%04x PID:%04x)\n",
					i, connectedControllers[i].vendorId, connectedControllers[i].productId);

				// Initialize mapping state
				memset(&g_mappingState, 0, sizeof(MappingState));
				g_mappingState.active = true;
				g_mappingState.reportId = connectedControllers[i].reportId;
				g_mappingState.reportInfo = connectedControllers[i].reportInfo;
				g_mappingState.controllerIndex = i;
				g_mappingState.pressedButtonIdx = 0xFF;

				HANDLE mappingThread = MakeThread((LPTHREAD_START_ROUTINE)MappingThreadProc, &connectedControllers[i]);
				if (mappingThread) {
					CloseHandle(mappingThread);
				} else {
					DbgPrint("EINTIM: Failed to create mapping thread!\n");
					g_mappingState.active = false;
				}
			}
		}
		Sleep(100); 
	}
	return 0;
}

unsigned int __stdcall MappingThreadProc(void* param) {
	XNotifyUI(XNOTIFYUI_CUSTOM, L"Unknown controller connected. Starting mapping process...");
	Controller* controller = (Controller*)param;
	HID_ReportInfo_t* info = controller->reportInfo;
	uint8_t reportId = controller->reportId;
	int controllerIndex = -1;

	for (int i = 0; i < 4; i++) {
		if (&connectedControllers[i] == controller) {
			controllerIndex = i;
			break;
		}
	}

	if (controllerIndex == -1)
		return -1;

	// Discover available buttons and axes
	uint8_t availableButtons[256] = {};
	uint8_t buttonCount = 0;
	DiscoverAvailableButtons(info, reportId, availableButtons, &buttonCount);

	uint16_t availableAxes[6] = {};
	uint8_t axisCount = 0;
	DiscoverAvailableAxes(info, reportId, availableAxes, &axisCount);

	// Store discovered buttons in mapping state
	g_mappingState.availableButtonCount = buttonCount;
	for (uint8_t i = 0; i < buttonCount; i++) {
		g_mappingState.availableButtons[i] = availableButtons[i];
	}

	std::vector<HidButtonMapEntry> mappedButtons;
	std::vector<HidAxisMapEntry> mappedAxes;
	HidAxisInvertFlags inverts = {0};

	// Invert vertical axes by default
	inverts.invertY = true;   // Left stick vertical
	inverts.invertRZ = true;  // Right stick vertical

	// Check for hat switch and analog triggers
	bool hasHatSwitch = FindHatItem(info, reportId) != nullptr;

	// If there are more than 4 axes (4 for dual analog sticks), the extra ones are analog triggers
	bool hasAnalogTriggers = axisCount > 4;

	// Track which HID usages we've mapped during this session
	uint16_t mappedUsages[6] = {};
	uint8_t mappedCount = 0;

	// Map buttons in predefined Xbox order
	const struct {
		uint8_t field_idx;
		const char* xbox_name;
		uint8_t ButtonsReport::* field;
	} xbox_buttons[] = {
		{0, "A", &ButtonsReport::a_button},
		{1, "B", &ButtonsReport::b_button},
		{2, "X", &ButtonsReport::x_button},
		{3, "Y", &ButtonsReport::y_button},
		{4, "LB", &ButtonsReport::l1},
		{5, "RB", &ButtonsReport::r1},
		{6, "Back", &ButtonsReport::back},
		{7, "Start", &ButtonsReport::start},
		{8, "Left Stick Click", &ButtonsReport::l3},
		{9, "Right Stick Click", &ButtonsReport::r3},
		{10, "Xbox", &ButtonsReport::xbox},
		{11, "LT", &ButtonsReport::l2},
		{12, "RT", &ButtonsReport::r2},
	};

	// Skip L2/R2 if we have analog triggers
	uint8_t buttonEndIdx = sizeof(xbox_buttons) / sizeof(xbox_buttons[0]);
	if (hasAnalogTriggers) {
		buttonEndIdx-=2;  // Only map up to RB, skip LT and RT
	}

	for (size_t i = 0; i < buttonEndIdx; i++) {
		if (!g_mappingState.active)
			break;

		static wchar_t msg[256];
		swprintf(msg, 256, L"Press %hs on controller (hold 3s to skip)", xbox_buttons[i].xbox_name);
		XNotifyUI(XNOTIFYUI_CUSTOM, msg);

		uint8_t previousButtonIdx = 0xFF;
		uint8_t foundButtonIdx = 0xFF;
		uint32_t pressWaitCount = 0;
		bool skipped = false;

		while (g_mappingState.active && foundButtonIdx == 0xFF && !skipped) {
			uint8_t currentButtonIdx = g_mappingState.pressedButtonIdx;

			if (previousButtonIdx == 0xFF && currentButtonIdx != 0xFF) {
				// Button just pressed
				pressWaitCount = 0;
				g_mappingState.holdCount = 0;
			} else if (previousButtonIdx != 0xFF && currentButtonIdx == 0xFF) {
				// Button just released
				if (pressWaitCount > 0) {
					foundButtonIdx = previousButtonIdx;
				}
				pressWaitCount = 0;
				g_mappingState.holdCount = 0;
			} else if (currentButtonIdx != 0xFF) {
				pressWaitCount++;
				g_mappingState.holdCount++;
				// 3 second hold = 60 iterations at 50ms each
				if (g_mappingState.holdCount >= 60) {
					skipped = true;
					XNotifyUI(XNOTIFYUI_CUSTOM, L"Mapping skipped");
					// Wait for button release
					while (g_mappingState.active && g_mappingState.pressedButtonIdx != 0xFF) {
						Sleep(50);
					}
				}
			}

			previousButtonIdx = currentButtonIdx;
			Sleep(50);
		}

		if (foundButtonIdx != 0xFF) {
			HidButtonMapEntry entry = {foundButtonIdx, xbox_buttons[i].field};
			mappedButtons.push_back(entry);
		}
	}

	// Map D-Pad buttons if no hat switch
	if (!hasHatSwitch && g_mappingState.active) {
		const struct {
			const char* dpad_name;
			uint8_t ButtonsReport::* field;
		} dpad_buttons[] = {
			{"D-Pad Left", &ButtonsReport::dpad_left},
			{"D-Pad Right", &ButtonsReport::dpad_right},
			{"D-Pad Up", &ButtonsReport::dpad_up},
			{"D-Pad Down", &ButtonsReport::dpad_down},
		};

		for (size_t i = 0; i < sizeof(dpad_buttons) / sizeof(dpad_buttons[0]); i++) {
			wchar_t msg[256];
			swprintf(msg, 256, L"Press %hs on controller (hold 3s to skip)", dpad_buttons[i].dpad_name);
			XNotifyUI(XNOTIFYUI_CUSTOM, msg);

			uint8_t previousButtonIdx = 0xFF;
			uint8_t foundButtonIdx = 0xFF;
			uint32_t pressWaitCount = 0;
			bool skipped = false;

			while (g_mappingState.active && foundButtonIdx == 0xFF && !skipped) {
				uint8_t currentButtonIdx = g_mappingState.pressedButtonIdx;

				if (previousButtonIdx == 0xFF && currentButtonIdx != 0xFF) {
					pressWaitCount = 0;
					g_mappingState.holdCount = 0;
				} else if (previousButtonIdx != 0xFF && currentButtonIdx == 0xFF) {
					if (pressWaitCount > 0) {
						foundButtonIdx = previousButtonIdx;
					}
					pressWaitCount = 0;
					g_mappingState.holdCount = 0;
				} else if (currentButtonIdx != 0xFF) {
					pressWaitCount++;
					g_mappingState.holdCount++;
					if (g_mappingState.holdCount >= 60) {
						skipped = true;
						XNotifyUI(XNOTIFYUI_CUSTOM, L"Mapping skipped");
						// Wait for button release
						while (g_mappingState.active && g_mappingState.pressedButtonIdx != 0xFF) {
							Sleep(50);
						}
					}
				}

				previousButtonIdx = currentButtonIdx;
				Sleep(50);
			}

			if (foundButtonIdx != 0xFF) {
				HidButtonMapEntry entry = {foundButtonIdx, dpad_buttons[i].field};
				mappedButtons.push_back(entry);
			}
		}
	}

	// use static axis mapping for now as for gamepads it should be the same for every gamepad
	HidAxisMapEntry entry;

	entry.usage = HID_USAGE_AXIS_X;
	entry.field = &ButtonsReport::x;
	mappedAxes.push_back(entry);

	entry.usage = HID_USAGE_AXIS_Y;
	entry.field = &ButtonsReport::y;
	mappedAxes.push_back(entry);

	entry.usage = HID_USAGE_AXIS_Z;
	entry.field = &ButtonsReport::z;
	mappedAxes.push_back(entry);

	entry.usage = HID_USAGE_AXIS_RX;
	entry.field = &ButtonsReport::rx;
	mappedAxes.push_back(entry);

	entry.usage = HID_USAGE_AXIS_RY;
	entry.field = &ButtonsReport::ry;
	mappedAxes.push_back(entry);

	entry.usage = HID_USAGE_AXIS_RZ;
	entry.field = &ButtonsReport::rz;
	mappedAxes.push_back(entry);

	// Build dynamic mapping
	std::unique_ptr<DynamicMappingData> dynamicData(new DynamicMappingData());
	dynamicData->axisEntries = mappedAxes;
	dynamicData->buttonEntries = mappedButtons;

	HidDeviceMapping newMapping = {0};
	newMapping.vendorId = controller->vendorId;
	newMapping.productId = controller->productId;
	newMapping.axisMapCount = (uint8_t)mappedAxes.size();
	newMapping.buttonMapCount = (uint8_t)mappedButtons.size();
	newMapping.invert = inverts;

	if (!mappedAxes.empty()) {
		newMapping.axisMap = dynamicData->axisEntries.data();
	}
	if (!mappedButtons.empty()) {
		newMapping.buttonMap = dynamicData->buttonEntries.data();
	}

	// Only save if mapping process wasn't interrupted
	if (!g_mappingState.active) {
		DbgPrint("EINTIM: Mapping was interrupted - not saving incomplete mapping\n");
		memset(&g_mappingState, 0, sizeof(MappingState));
		g_mappingState.pressedButtonIdx = 0xFF;
		return 0;
	}

	// Apply mapping to controller
	g_dynamicData.push_back(std::move(dynamicData));
	g_dynamicData.back()->axisEntries = mappedAxes;
	g_dynamicData.back()->buttonEntries = mappedButtons;

	HidDeviceMapping finalMapping = newMapping;
	finalMapping.axisMap = g_dynamicData.back()->axisEntries.data();
	finalMapping.buttonMap = g_dynamicData.back()->buttonEntries.data();

	g_dynamicMappings.push_back(finalMapping);
	connectedControllers[controllerIndex].map = &g_dynamicMappings.back();

	SaveMappingsToFile("HDD:\\hiddriver.json");

	XNotifyUI(XNOTIFYUI_CUSTOM, L"Mapping complete! Controller ready.");
	g_mappingState.active = false;

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

	if (NeedsNintendoHandshake(connectedControllers[index].vendorId, connectedControllers[index].productId) && connectedControllers[index].nintendo_handshake_state != DONE) {
		if (connectedControllers[index].nintendo_handshake_state == INITIAL) {
			DbgPrint("EINTIM: Gotta do nintendo handshake for this one!\r\n");
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

		// This special case is needed because:
		// https://gbatemp.net/threads/reverse-engineering-the-switch-pro-controller-wired-mode.475226/
		/*
		"WARNING: The HID descriptor does not match the data in the controller payload at all. My guess is it's just the Bluetooth HID descriptor c/p over. Because of that, if you enable the controller on Windows by poking that enable interrupt packet with your favorite USB tool, Windows will go crazy trying to interpret the packets it gets. I now have this on-screen controller keyboard I don't know how to get rid of."
		*/
		if (NeedsNintendoHandshake(connectedControllers[index].vendorId, connectedControllers[index].productId)) {
			switch_pro_input_report* switch_report = (switch_pro_input_report*)payload;
			
			uint16_t lx = switch_report->left_stick[0] | ((switch_report->left_stick[1] & 0x0F) << 8);
			uint16_t ly = (switch_report->left_stick[1] >> 4) | (switch_report->left_stick[2] << 4);
			uint16_t rx = switch_report->right_stick[0] | ((switch_report->right_stick[1] & 0x0F) << 8);
			uint16_t ry = (switch_report->right_stick[1] >> 4) | (switch_report->right_stick[2] << 4);

			// TODO: read the actual calibration values for normalization
			buttonReport.x = normalize_stick(lx);
			buttonReport.y = normalize_stick(ly);
			buttonReport.z = normalize_stick(rx);
			buttonReport.rz = normalize_stick(ry);

			uint16_t b1 = switch_report->buttons_right | ((uint16_t)switch_report->buttons_mid << 8);
			uint8_t  b2 = switch_report->buttons_left;

			buttonReport.a_button = (b1 & SWITCH_BTN_B) ? 1 : 0;
			buttonReport.b_button = (b1 & SWITCH_BTN_A) ? 1 : 0;
			buttonReport.x_button = (b1 & SWITCH_BTN_X) ? 1 : 0;
			buttonReport.y_button = (b1 & SWITCH_BTN_Y) ? 1 : 0;
			buttonReport.r1 = (b1 & SWITCH_BTN_R) ? 1 : 0;
			buttonReport.l1 = (b2 & SWITCH_BTN_L) ? 1 : 0;
			buttonReport.r2 = (b1 & SWITCH_BTN_ZR) ? 1 : 0;
			buttonReport.l2 = (b2 & SWITCH_BTN_ZL) ? 1 : 0;

			buttonReport.r3 = (b1 & SWITCH_BTN_R_STICK) ? 1 : 0;
			buttonReport.l3 = (b1 & SWITCH_BTN_L_STICK) ? 1 : 0;
			buttonReport.start = (b1 & SWITCH_BTN_PLUS) ? 1 : 0;
			buttonReport.back = (b1 & SWITCH_BTN_MINUS) ? 1 : 0;
			buttonReport.xbox = (b1 & SWITCH_BTN_HOME) ? 1 : 0;

			buttonReport.has_hat_switch = false;
			buttonReport.dpad_up = (b2 & SWITCH_DPAD_UP) ? 1 : 0;
			buttonReport.dpad_down = (b2 & SWITCH_DPAD_DOWN) ? 1 : 0;
			buttonReport.dpad_left = (b2 & SWITCH_DPAD_LEFT) ? 1 : 0;
			buttonReport.dpad_right = (b2 & SWITCH_DPAD_RIGHT) ? 1 : 0;
		}

		else if (connectedControllers[index].map) {
			HidFillButtonsReport(
				payload,
				connectedControllers[index].reportInfo,
				&buttonReport,
				connectedControllers[index].reportId,
				connectedControllers[index].map);
		}
		else if (g_mappingState.active && g_mappingState.controllerIndex == index) {
			// Collect raw button states during mapping - only check discovered buttons
			g_mappingState.pressedButtonIdx = 0xFF;
			uint8_t currentPressCount = 0;

			for (uint8_t i = 0; i < g_mappingState.availableButtonCount; i++) {
				uint8_t buttonIdx = g_mappingState.availableButtons[i];
				HID_ReportItem_t* item = FindButtonItem(connectedControllers[index].reportInfo, buttonIdx, connectedControllers[index].reportId);
				if (item && USB_GetHIDReportItemInfo(connectedControllers[index].reportId, payload, item)) {
					if (item->Value) {
						g_mappingState.pressedButtonIdx = buttonIdx;
						currentPressCount++;
					}
				}
			}

			// Clear if multiple buttons pressed (avoid accidental mappings)
			if (currentPressCount != 1) {
				g_mappingState.pressedButtonIdx = 0xFF;
			}

			// Collect raw axis states
			for (uint8_t i = 0; i < 6; i++) {
				static const uint16_t usages[] = {HID_USAGE_AXIS_X, HID_USAGE_AXIS_Y, HID_USAGE_AXIS_Z,
									  HID_USAGE_AXIS_RX, HID_USAGE_AXIS_RY, HID_USAGE_AXIS_RZ};
				HID_ReportItem_t* item = FindItemByUsage(connectedControllers[index].reportInfo,
					HID_USAGE_PAGE_GENERIC_DESKTOP, usages[i], connectedControllers[index].reportId);
				if (item && USB_GetHIDReportItemInfo(connectedControllers[index].reportId, payload, item)) {
					int32_t logMin = (int32_t)item->Attributes.Logical.Minimum;
					int32_t logMax = (int32_t)item->Attributes.Logical.Maximum;
					int32_t raw = (int32_t)item->Value;

					int16_t result = 0;
					if (logMax > logMin) {
						if (raw < logMin) raw = logMin;
						if (raw > logMax) raw = logMax;
						int64_t numerator = (int64_t)(raw - logMin) * 65535;
						int32_t denominator = (logMax - logMin);
						int32_t scaled = (int32_t)((numerator + denominator / 2) / denominator);
						result = (int16_t)(scaled - 32768);
					} else {
						result = (int16_t)raw;
					}
					g_mappingState.axisValues[i] = result;
				}
			}
		}

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

	// Check if this controller is currently in mapping process and stop it
	if (g_mappingState.active && g_mappingState.controllerIndex == index) {
		DbgPrint("EINTIM: Stopping mapping process for removed controller %d\n", index);
		g_mappingState.active = false;
		memset(&g_mappingState, 0, sizeof(MappingState));
		g_mappingState.pressedButtonIdx = 0xFF;
	}

	if (!deviceHandle2->driver->cleanupDone) {
		deviceHandle2->driver->cleanupDone = 1;
		connectedControllers[index].controllerDriver = nullptr;

		// Free HID report info for this controller slot
		if (connectedControllers[index].reportInfo) {
			USB_FreeReportInfo(connectedControllers[index].reportInfo);
			connectedControllers[index].reportInfo = nullptr;
		}
		// Clear mapping data
		connectedControllers[index].map = nullptr;
		memset(&connectedControllers[index], 0, sizeof(Controller));
		delete deviceHandle2->driver;
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
		c.sonyUsage = 0;
		c.subType = XINPUT_DEVSUBTYPE_GAMEPAD;
		c.flags = 0;
		c.map = FindMapping(vendorId, productId);
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
		capabilities->SubType = c->subType;
		capabilities->Flags = c->flags;

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

		if (b.a_button)    pInputData->wButtons |= XINPUT_GAMEPAD_A;
		if (b.b_button)   pInputData->wButtons |= XINPUT_GAMEPAD_B;
		if (b.y_button) pInputData->wButtons |= XINPUT_GAMEPAD_Y;
		if (b.x_button)   pInputData->wButtons |= XINPUT_GAMEPAD_X;
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
uint16_t* XNotifyTimerPtr = nullptr;
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

		// Remove two usb related bugchecks to allow reinitialisation of the usb driver
		*(DWORD*)0x80116298 = 0x48000018;
		*(DWORD*)0x801132A4 = 0x48000018;

		// DEVKIT only: Remove assertions(Microsoft did not think that we'd come and reset the usb driver, never let them know your next move typa shit)
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

		*(uint16_t*)0x8176A7C6 = 80; // Register custom notification type condition
		XNotifyTimerPtr = (uint16_t*)0x8176a7ca;
	}
	else {
		DbgPrint("EINTIM: Running in retail mode\n");
		UsbdGetInterfaceDescriptor = (usb_interface_descriptor_func_t)0x800D8500; // 89 43 ? ? 3D 60 ? ? 39 6B ? ? 55 4A FF 3A 7D 6A 58 2E A1 4B
		XamUserBindDeviceCallback = (xam_user_bind_device_callback_func_t)0x816D9060; // 7C 8B 23 78 7C A4 2B 78 54 CA 06 3F
		UsbdPowerDownNotification = (usbd_powerdown_notification_func_t)0x800D8FC8; // argument to last function call in UsbdDriverEntry
		UsbdDriverEntry = (usbd_powerdown_notification_func_t)0x800D8D08; // 7D 88 02 A6 ? ? ? ? 94 21 ? ? 3C 80 ? ? 38 A0 

		// Remove two usb related bugchecks to allow reinitialisation of the usb driver
		*(DWORD*)0x800E05E4 = 0x48000018;
		*(DWORD*)0x800DD8E0 = 0x48000018;

		// Prevent double registration of Usbd handlers because the console wont shutdown cleanly otherwise
		*(DWORD*)0x800D8F00 = 0x60000000;
		*(DWORD*)0x800D8EF0 = 0x60000000;
		UsbPhysicalPage = 0x801A8098;

		*(uint16_t*)0x816AB7A6 = 80; // Register custom notification type condition
		XNotifyTimerPtr = (uint16_t*)0x816ab7aa;
	}

	*XNotifyTimerPtr = 1500;
	return true;
}

BOOL APIENTRY DllMain(HANDLE Handle, DWORD Reason, PVOID Reserved) {
	if (Reason == DLL_PROCESS_ATTACH) {
		if ((XboxKrnlVersion->Build != 17559 && XboxKrnlVersion->Build != 17489) || IsTrayOpen()) {
			DbgPrint("EINTIM: Only 17559 and 17489 dashboards are currently supported or the disk tray is open. Aborting launch...\n");
			return FALSE;
		}

		DbgPrint("EINTIM: HELLO from xbox 360 HID controller driver version 0.6 beta\n");
		if (!initFunctionPointers())
			return FALSE;

		DbgPrint("EINTIM: Loading mappings!\r\n");
		if (!LoadMappingsFromFile("HDD:\\hiddriver.json")) {
			DbgPrint("EINTIM: Failed to load mappings(JSON either doesn't exist yet or syntax error)!\r\n");
		}

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
		DbgPrint("EINTIM: Hooks installed\n");

		// This is a dirty way of forcing the system to reenumerate USB devices so you don't need to replug the controllers
		DbgPrint("EINTIM: Resetting USB driver!\n");
		UsbdPowerDownNotification();

		// For some reason microsoft doesnt clean up this page by themselves in the shutdown notification, so ill do it for them, call me mr nice guy :)
		MmFreePhysicalMemory(0, *(DWORD*)UsbPhysicalPage);
		DbgPrint("EINTIM: USB driver shutdown complete.\n");

		UsbdDriverEntry();
		DbgPrint("EINTIM: USB driver reset complete.\n");

		// Start mapping manager thread
		MakeThread((LPTHREAD_START_ROUTINE)MappingManagerThreadProc, nullptr);
	}
	return TRUE;
}