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
	BYTE interfaceNumber;
	BYTE gap20[3];
	UsbControlTrb controlTrb;
	UsbTrb closeTrb;          // 0x4C: endpoint close request, the same slot the kernel HID driver uses
	BYTE gap68[4];
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
static_assert(offsetof(HidControllerExtension, controlTrb) == 0x24, "HID extension layout changed");
static_assert(offsetof(HidControllerExtension, closeTrb) == 0x4C, "HID extension layout changed");
static_assert(offsetof(HidControllerExtension, cleanupDone) == 0x74, "HID extension layout changed");

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
	INIT_GET_REPORT_DESCRIPTOR,
	INIT_DONE,
	INIT_FAILED
};

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
#define SWITCH_BTN_Y        (1 << 1)
#define SWITCH_BTN_X        (1 << 0)
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
const uint16_t DUALSENSE_PRODUCT_ID = 0x0CE6;
const uint16_t DUALSENSE_EDGE_PRODUCT_ID = 0x0DF2;
const unsigned char DS3_HANDSHAKE[4] = { 0x42, 0x0C, 0x00, 0x00 };
const uint8_t DUALSENSE_USB_OUTPUT_REPORT_ID = 0x02;
const uint8_t DUALSENSE_USB_OUTPUT_REPORT_SIZE = 63;
const uint8_t DUALSENSE_VALID_FLAG_HAPTICS_SELECT = 1 << 1;
const uint8_t DUALSENSE_VALID_FLAG_COMPATIBLE_VIBRATION = 1 << 0;

bool NeedsDualshock3Handshake(uint16_t vid, uint16_t pid) {
	if (vid != SONY_VENDOR_ID) return false;
	return pid == DS3_PRODUCT_ID;
}

bool IsDualSenseController(uint16_t vid, uint16_t pid) {
	return vid == SONY_VENDOR_ID &&
		(pid == DUALSENSE_PRODUCT_ID || pid == DUALSENSE_EDGE_PRODUCT_ID);
}

enum DS3_FEATURE_VALUE
{
	Ds3FeatureDeviceAddress = 0x03F2,
	Ds3FeatureStartDevice = 0x03F4,
	Ds3FeatureHostAddress = 0x03F5

};
// dualshock 3 specific end

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
	const HidDeviceMapping* map;

	// for nintendo specific handshake
	NINTENDO_HANDSHAKE_STATE nintendo_handshake_state;
	UsbTrb interruptTrb;

	// DualSense USB rumble output endpoint and coalesced asynchronous transfer state.
	UsbTrb rumbleTrb;
	BYTE rumbleReport[DUALSENSE_USB_OUTPUT_REPORT_SIZE];
	bool rumbleEndpointOpen;
	volatile LONG rumbleTransferBusy;
	volatile LONG rumblePending;
	volatile LONG rumbleLeftRequested;
	volatile LONG rumbleRightRequested;
	volatile LONG rumbleLeftLast;
	volatile LONG rumbleRightLast;

	// Enumeration state, kept per slot so two controllers can enumerate at once.
	InitState initState;
	usb_hid_descriptor hidDescriptor;
	void* reportDescriptor;
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
// One HID extension per slot. A slot stays reserved from HidAddDevice until its
// endpoints are closed and Usbd has been told the device is gone, so pending
// transfers never complete into freed memory.
HidControllerExtension g_controllerExtensions[4];
volatile LONG g_extensionInUse[4];

// Returns the slot that owns this HID extension, or -1 if it isn't one of ours.
int SlotFromExtension(HidControllerExtension* ext) {
	for (int i = 0; i < (sizeof(g_controllerExtensions) / sizeof(HidControllerExtension)); i++) {
		if (ext == &g_controllerExtensions[i])
			return i;
	}
	return -1;
}

static void PumpDualSenseRumble(Controller* controller);

int32_t DualSenseRumbleComplete(DWORD transferAddress, int32_t status) {
	Controller* controller = nullptr;
	for (int i = 0; i < (sizeof(connectedControllers) / sizeof(Controller)); i++) {
		if ((DWORD)&connectedControllers[i].rumbleTrb == transferAddress) {
			controller = &connectedControllers[i];
			break;
		}
	}

	if (!controller)
		return 0;

	InterlockedExchange(&controller->rumbleTransferBusy, 0);

	// Don't retry a failed transfer from here; the next input report retries it.
	if (status != 0) {
		DbgPrint("EINTIM: DualSense rumble transfer completed with status %x.\n", status);
		return 0;
	}

	PumpDualSenseRumble(controller);
	return 0;
}

// Caller must own rumbleTransferBusy.
static void QueuePendingDualSenseRumble(Controller* controller) {
	if (InterlockedExchange(&controller->rumblePending, 0)) {
		LONG left = InterlockedCompareExchange(&controller->rumbleLeftRequested, 0, 0);
		LONG right = InterlockedCompareExchange(&controller->rumbleRightRequested, 0, 0);

		memset(controller->rumbleReport, 0, sizeof(controller->rumbleReport));
		controller->rumbleReport[0] = DUALSENSE_USB_OUTPUT_REPORT_ID;
		controller->rumbleReport[1] = DUALSENSE_VALID_FLAG_HAPTICS_SELECT |
			DUALSENSE_VALID_FLAG_COMPATIBLE_VIBRATION;
		controller->rumbleReport[3] = (BYTE)right;
		controller->rumbleReport[4] = (BYTE)left;

		controller->rumbleTrb.flags = 1;
		controller->rumbleTrb.length = DUALSENSE_USB_OUTPUT_REPORT_SIZE;
		controller->rumbleTrb.callback = (DWORD)DualSenseRumbleComplete;
		controller->rumbleTrb.savedEndpoint = controller->rumbleTrb.endpoint;
		controller->rumbleTrb.buffer = controller->rumbleReport;

		NTSTATUS transferStatus = UsbdQueueAsyncTransfer(
			controller->deviceHandle, &controller->rumbleTrb);
		if (NT_ERROR(transferStatus)) {
			DbgPrint("EINTIM: Failed to queue DualSense rumble report: %x.\n", transferStatus);
			InterlockedExchange(&controller->rumblePending, 1);
			InterlockedExchange(&controller->rumbleTransferBusy, 0);
		}
		return;
	}

	InterlockedExchange(&controller->rumbleTransferBusy, 0);
}

// Sends the newest requested rumble values if no OUT transfer is in flight.
// Only call this from USB completion callbacks. Title threads calling
// XamInputSetState run on other hardware threads, and queueing transfers from
// there races the USB completion path inside Usbd.
static void PumpDualSenseRumble(Controller* controller) {
	if (!controller->rumbleEndpointOpen || !controller->deviceHandle)
		return;
	if (!InterlockedCompareExchange(&controller->rumblePending, 0, 0))
		return;
	if (InterlockedCompareExchange(&controller->rumbleTransferBusy, 1, 0) != 0)
		return;
	QueuePendingDualSenseRumble(controller);
}

// Runs on the title thread: only records the request. The DualSense input
// report completion picks it up and queues the USB transfer.
static void RequestDualSenseRumble(Controller* controller, XINPUT_VIBRATION* vibration) {
	if (!controller || !controller->rumbleEndpointOpen || !vibration)
		return;

	LONG left = vibration->wLeftMotorSpeed >> 8;
	LONG right = vibration->wRightMotorSpeed >> 8;
	LONG oldLeft = InterlockedExchange(&controller->rumbleLeftLast, left);
	LONG oldRight = InterlockedExchange(&controller->rumbleRightLast, right);
	if (oldLeft == left && oldRight == right &&
		!InterlockedCompareExchange(&controller->rumblePending, 0, 0))
		return;

	InterlockedExchange(&controller->rumbleLeftRequested, left);
	InterlockedExchange(&controller->rumbleRightRequested, right);
	InterlockedExchange(&controller->rumblePending, 1);
}
MappingState g_mappingState;

int interruptHandler(DWORD deviceHandle, int32_t a2);

// Keep every IN report item; the driver uses all axes, the hat, and buttons.
bool CALLBACK_HIDParser_FilterHIDReportItem(HID_ReportItem_t* const CurrentItem) {
	return (CurrentItem->ItemType == HID_REPORT_ITEM_In);
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
	HidControllerExtension* controllerDriver = (HidControllerExtension*)((BYTE*)deviceHandle -
		offsetof(HidControllerExtension, controlTrb));
	int slot = SlotFromExtension(controllerDriver);
	if (slot < 0 || controllerDriver->cleanupDone)
		return 0;

	Controller* ctl = &connectedControllers[slot];

	if (status != 0) {
		DbgPrint("EINTIM: Control transfer failed with status %x!\n", status);
		ctl->initState = INIT_FAILED;
		free(ctl->reportDescriptor);
		ctl->reportDescriptor = nullptr;
		return status;
	}

	if (ctl->initState == InitState::INIT_SET_CONFIGURATION) {
		// SET_CONFIGURATION just completed, now fetch the report descriptor
		DbgPrint("EINTIM: SET_CONFIGURATION completed. Requesting report descriptor.\n");

		// Prepare report descriptor buffer
		ctl->hidDescriptor.wDescriptorLength = swap_endianness_16(ctl->hidDescriptor.wDescriptorLength);
		DbgPrint("EINTIM: Report descriptor length: %d\n", ctl->hidDescriptor.wDescriptorLength);

		if (ctl->hidDescriptor.wDescriptorLength == 0) {
			DbgPrint("EINTIM: ERROR - HID descriptor length is 0!\n");
			ctl->initState = INIT_FAILED;
			return -1;
		}

		ctl->reportDescriptor = calloc(1, ctl->hidDescriptor.wDescriptorLength);
		if (!ctl->reportDescriptor) {
			DbgPrint("EINTIM: ERROR - Could not allocate the report descriptor buffer!\n");
			ctl->initState = INIT_FAILED;
			return -1;
		}

		ctl->initState = InitState::INIT_GET_REPORT_DESCRIPTOR;
		DbgPrint("EINTIM: Fetching report descriptor. Interface: %d, Length: %d\n",
			controllerDriver->interfaceNumber, ctl->hidDescriptor.wDescriptorLength);

		SendControlRequest(
			controllerDriver->deviceHandle,
			&controllerDriver->controlTrb,
			0x81,
			0x06,
			0x2200,
			controllerDriver->interfaceNumber,
			ctl->hidDescriptor.wDescriptorLength,
			ctl->reportDescriptor,
			(DWORD)setConfigurationComplete);
	}
	else if (ctl->initState == InitState::INIT_GET_REPORT_DESCRIPTOR) {
		// Report descriptor request completed
		DbgPrint("EINTIM: Report descriptor request completed successfully\n");

		HID_ReportInfo_t* reportInfo = nullptr;
		uint8_t parseResult = USB_ProcessHIDReport((const uint8_t*)ctl->reportDescriptor,
			ctl->hidDescriptor.wDescriptorLength,
			&reportInfo);
		free(ctl->reportDescriptor);
		ctl->reportDescriptor = nullptr;

		if (parseResult != HID_PARSE_Successful || !reportInfo) {
			DbgPrint("EINTIM: Failed to parse HID descriptor: error %d\r\n", parseResult);
			ctl->initState = InitState::INIT_FAILED;
			return -1;
		}

		ctl->initState = InitState::INIT_DONE;
		ctl->reportInfo = reportInfo;
		ctl->reportId = FindGamepadReportId(reportInfo);

		DbgPrint("EINTIM: Parsed descriptor. UsingReportIDs: %d, Report ID: %d\r\n",
			(int)reportInfo->UsingReportIDs, ctl->reportId);

		usb_endpoint_descriptor* endpoint_descriptor = UsbdGetEndpointDescriptor(
			controllerDriver->deviceHandle, 0, USB_ENDPOINT_TYPE_INTERRUPT, USB_DIRECTION_IN);
		if (!endpoint_descriptor) {
			DbgPrint("EINTIM: Controller has no interrupt IN endpoint!\n");
			return -1;
		}

		uint16_t pktSize = swap_endianness_16(endpoint_descriptor->wMaxPacketSize) & 0x7FF;
		ctl->reportData = malloc(pktSize * 2);
		if (!ctl->reportData) {
			DbgPrint("EINTIM: Could not allocate the input report buffer!\n");
			return -1;
		}
		memset(ctl->reportData, 0, pktSize * 2);

		status = UsbdOpenEndpoint(
			controllerDriver->deviceHandle,
			3,
			endpoint_descriptor->bEndpointAddress,
			pktSize,
			endpoint_descriptor->bInterval,
			(DWORD*)&controllerDriver->interruptTrb);

		if (NT_ERROR(status)) {
			DbgPrint("EINTIM: Failed to open interrupt endpoint %x!\n", status);
			return status;
		}

		controllerDriver->interruptTrb.savedEndpoint = controllerDriver->interruptTrb.endpoint;
		controllerDriver->interruptTrb.length = pktSize;
		controllerDriver->interruptTrb.callback = (DWORD)interruptHandler;
		controllerDriver->interruptTrb.buffer = ctl->reportData;

		if (IsDualSenseController(ctl->vendorId, ctl->productId)) {
			usb_endpoint_descriptor* outputDescriptor = UsbdGetEndpointDescriptor(
				controllerDriver->deviceHandle, 0,
				USB_ENDPOINT_TYPE_INTERRUPT, USB_DIRECTION_OUT);
			if (!outputDescriptor) {
				DbgPrint("EINTIM: DualSense has no interrupt OUT endpoint; rumble disabled.\n");
			}
			else {
				uint16_t outputPacketSize = swap_endianness_16(outputDescriptor->wMaxPacketSize) & 0x7FF;
				if (outputPacketSize < DUALSENSE_USB_OUTPUT_REPORT_SIZE) {
					DbgPrint("EINTIM: DualSense OUT endpoint packet size %d is too small; rumble disabled.\n",
						outputPacketSize);
				}
				else {
					// Opened straight into the slot: the TRB must stay at the
					// address Usbd saw when the endpoint was opened.
					memset(&ctl->rumbleTrb, 0, sizeof(ctl->rumbleTrb));
					NTSTATUS rumbleEndpointStatus = UsbdOpenEndpoint(
						controllerDriver->deviceHandle,
						USB_ENDPOINT_TYPE_INTERRUPT,
						outputDescriptor->bEndpointAddress,
						outputPacketSize,
						outputDescriptor->bInterval,
						(DWORD*)&ctl->rumbleTrb);
					if (NT_ERROR(rumbleEndpointStatus)) {
						DbgPrint("EINTIM: Failed to open DualSense OUT endpoint %x; rumble disabled.\n",
							rumbleEndpointStatus);
					}
					else {
						ctl->rumbleEndpointOpen = true;
						DbgPrint("EINTIM: DualSense rumble OUT endpoint opened at %02x.\n",
							outputDescriptor->bEndpointAddress);
					}
				}
			}
		}

		uint8_t  userIndex = -1;
		uint32_t context = 0x0000000010000005 + slot;
		XamUserBindDeviceCallback(0xa7553952 + slot, context, 0, false, &userIndex);
		ctl->userIndex = userIndex;
		ctl->deviceContext = context;

		// Publishing controllerDriver makes the slot visible to the XInput hooks,
		// so every other field has to be written first.
		__lwsync();
		ctl->controllerDriver = controllerDriver;

		DbgPrint("EINTIM: Registered virtual controller inside XAM with index: %d.\n", userIndex);

		if (NeedsDualshock3Handshake(ctl->vendorId, ctl->productId)) {
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

	// Like the kernel HID driver: skip the report of a failed transfer and resubmit.
	if (a2 < 0)
		return UsbdQueueAsyncTransfer(driverExtension->deviceHandle, &driverExtension->interruptTrb);

	int index = SlotFromExtension(driverExtension);
	if (index < 0)
		return 0;

	PumpDualSenseRumble(&connectedControllers[index]);

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


// Mirrors the kernel HID driver's removal path: close one endpoint at a time
// through the close request at 0x4C, continue from its completion, and report
// the removal to Usbd once nothing is open. Closing an endpoint completes its
// pending transfers first, so no callback can touch the slot after this ends.
int32_t HidRemoveCleanupStep(DWORD closeTrbAddress, int32_t status) {
	HidControllerExtension* ext = (HidControllerExtension*)(closeTrbAddress -
		offsetof(HidControllerExtension, closeTrb));
	deviceHandle* dev = ext->deviceHandle;
	UsbTrb* closeTrb = &ext->closeTrb;
	int index = SlotFromExtension(ext);
	Controller* controller = index >= 0 ? &connectedControllers[index] : nullptr;

	if (controller) {
		DWORD* extraEndpoints[] = {
			&controller->rumbleTrb.endpoint,
			&controller->interruptTrb.endpoint, // Switch Pro handshake OUT endpoint
		};
		for (int i = 0; i < (sizeof(extraEndpoints) / sizeof(extraEndpoints[0])); i++) {
			if (*extraEndpoints[i]) {
				closeTrb->savedEndpoint = *extraEndpoints[i];
				*extraEndpoints[i] = 0;
				UsbdQueueCloseEndpoint(dev, closeTrb);
				return 0;
			}
		}
	}

	if (ext->interruptTrb.endpoint) {
		closeTrb->savedEndpoint = ext->interruptTrb.endpoint;
		ext->interruptTrb.endpoint = 0;
		UsbdQueueCloseEndpoint(dev, closeTrb);
		return 0;
	}

	if (ext->controlTrb.trb.endpoint) {
		closeTrb->savedEndpoint = ext->controlTrb.trb.endpoint;
		ext->controlTrb.trb.endpoint = 0;
		UsbdQueueCloseDefaultEndpoint(dev, (DWORD*)closeTrb);
		return 0;
	}

	if (controller) {
		if (controller->reportInfo)
			USB_FreeReportInfo(controller->reportInfo);
		free(controller->reportData);
		free(controller->reportDescriptor);
		memset(controller, 0, sizeof(Controller));
	}

	dev->driver = nullptr;
	UsbdRemoveDeviceComplete(dev);
	DbgPrint("EINTIM: Removed controller with handle %p\n", dev);

	if (index >= 0)
		InterlockedExchange(&g_extensionInUse[index], 0);
	return 0;
}

int HidRemoveDeviceHook(deviceHandle* deviceHandle2) {
	HidControllerExtension* ext = deviceHandle2->driver;
	int index = SlotFromExtension(ext);

	if (index < 0) {
		DbgPrint("EINTIM: Returning original for handle %p\n", deviceHandle2);
		return HidRemoveDeviceDetour.GetOriginal<decltype(&HidRemoveDeviceHook)>()(deviceHandle2);
	}

	if (ext->cleanupDone)
		return 0;

	DbgPrint("EINTIM: Removing controller with handle %p\n", deviceHandle2);

	// interruptHandler and the rumble pump stop resubmitting once this is set.
	ext->cleanupDone = 1;

	// Check if this controller is currently in mapping process and stop it
	if (g_mappingState.active && g_mappingState.controllerIndex == index) {
		DbgPrint("EINTIM: Stopping mapping process for removed controller %d\n", index);
		g_mappingState.active = false;
		memset(&g_mappingState, 0, sizeof(MappingState));
		g_mappingState.pressedButtonIdx = 0xFF;
	}

	Controller* controller = &connectedControllers[index];
	controller->rumbleEndpointOpen = false;
	if (controller->controllerDriver == ext) {
		// Hide the slot from the XInput hooks and the mapping thread; its memory
		// is released once every endpoint has closed.
		controller->controllerDriver = nullptr;
		XamUserBindDeviceCallback(0xa7553952 + index, 0x0000000010000005 + index, 0, true, 0);
		DbgPrint("EINTIM: Removed virtual controller from XAM.\n");
	}

	ext->closeTrb.callback = (DWORD)HidRemoveCleanupStep;
	HidRemoveCleanupStep((DWORD)&ext->closeTrb, 0);
	return 0;
}

int reportData = 0;
int HidAddDeviceHook(deviceHandle* deviceHandle) {
	DbgPrint("EINTIM: HID add device %p\n", deviceHandle);
	usb_device_descriptor* device_descriptor = UsbdGetDeviceDescriptor(deviceHandle);
	usb_interface_descriptor* interface_descriptor = UsbdGetInterfaceDescriptor(deviceHandle);
	if (!device_descriptor || !interface_descriptor)
		return HidAddDeviceDetour.GetOriginal<decltype(&HidAddDeviceHook)>()(deviceHandle);

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
		
		// Extract HID descriptor from memory right after interface descriptor
		BYTE* hid_descriptor_ptr = ((BYTE*)interface_descriptor) + interface_descriptor->bLength;
		usb_hid_descriptor* hid_descriptor = (usb_hid_descriptor*)hid_descriptor_ptr;
		
		DbgPrint("EINTIM: Found HID descriptor at offset %d: type=%02x, length=%d\n",
			interface_descriptor->bLength, hid_descriptor->bDescriptorType, hid_descriptor->wDescriptorLength);
		
		if (hid_descriptor->bDescriptorType != 0x21) {
			DbgPrint("EINTIM: ERROR - Invalid HID descriptor type %02x!\n", hid_descriptor->bDescriptorType);
			return HidAddDeviceDetour.GetOriginal<decltype(&HidAddDeviceHook)>()(deviceHandle);
		}
		
		// A slot is free only after its previous device finished closing.
		int index = -1;
		for (int i = 0; i < (sizeof(connectedControllers) / sizeof(Controller)); i++) {
			if (InterlockedCompareExchange(&g_extensionInUse[i], 1, 0) == 0) {
				DbgPrint("Assigning controller to index %d\n", i);
				index = i;
				break;
			}
		}

		if (index == -1) {
			DbgPrint("EINTIM: No free index!\n");
			return HidAddDeviceDetour.GetOriginal<decltype(&HidAddDeviceHook)>()(deviceHandle);
		}
		// The slot stays invisible to the XInput hooks until controllerDriver is
		// set at the end of enumeration.
		Controller* ctl = &connectedControllers[index];
		memset(ctl, 0, sizeof(Controller));
		ctl->vendorId = vendorId;
		ctl->productId = productId;
		ctl->map = FindMapping(vendorId, productId);
		ctl->nintendo_handshake_state = NINTENDO_HANDSHAKE_STATE::INITIAL;

		HidControllerExtension* controllerDriver = &g_controllerExtensions[index];
		memset(controllerDriver, 0, sizeof(HidControllerExtension));
		ctl->deviceHandle = deviceHandle;
		controllerDriver->deviceType = 0;
		deviceHandle->driver = controllerDriver;
		controllerDriver->deviceHandle = deviceHandle;
		controllerDriver->interfaceNumber = interface_descriptor->bInterfaceNumber;
		DbgPrint("EINTIM: Storing interface number: %d\n", controllerDriver->interfaceNumber);
		controllerDriver->interruptTrb.flags = 1;

		// Keep the HID descriptor for the report descriptor request
		memcpy(&ctl->hidDescriptor, hid_descriptor, sizeof(usb_hid_descriptor));
		DbgPrint("EINTIM: Copied HID descriptor. wDescriptorLength: %d\n", ctl->hidDescriptor.wDescriptorLength);

		UsbdAddDeviceComplete(deviceHandle, 0);

		NTSTATUS status = UsbdOpenDefaultEndpoint(deviceHandle, (DWORD*)&controllerDriver->controlTrb);
		if (NT_ERROR(status)) {
			DbgPrint("EINTIM: Failed to open control endpoint %x!\n", status);
			return status;
		}

		// Set device configuration (required for proper USB enumeration)
		ctl->initState = InitState::INIT_SET_CONFIGURATION;
		DbgPrint("EINTIM: Sending SET_CONFIGURATION\n");
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

DWORD XamInputSetStateHook(DWORD user, DWORD flags, XINPUT_VIBRATION* vibration) {
	DWORD status = XamInputSetStateDetour.GetOriginal<decltype(&XamInputSetStateHook)>()(user, flags, vibration);

	if ((user & 0xFF) == 0xFF)
		user = 0;

	Controller* controller = nullptr;
	for (int i = 0; i < (sizeof(connectedControllers) / sizeof(Controller)); i++) {
		if (connectedControllers[i].controllerDriver &&
			connectedControllers[i].userIndex == user) {
			controller = &connectedControllers[i];
			break;
		}
	}
	if (!controller)
		return status;

	RequestDualSenseRumble(controller, vibration);
	if (status == ERROR_DEVICE_NOT_CONNECTED)
		return ERROR_SUCCESS;

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
		capabilities->Vibration.wLeftMotorSpeed = c->rumbleEndpointOpen ? 0xFFFF : 0;
		capabilities->Vibration.wRightMotorSpeed = c->rumbleEndpointOpen ? 0xFFFF : 0;
		return ERROR_SUCCESS;
	}

	return status;
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
uint16_t* XNotifyTypeLimitPtr = nullptr;
bool isDevkit = true;
DWORD UsbPhysicalPage = 0;
void* NotificationPatchPtr = nullptr;
void* HidAddDeviceAddress = nullptr;
void* HidRemoveDeviceAddress = nullptr;
bool g_SkipUsbDriverReset = false;

// The retail 17502 build is not covered by the hard-coded offsets below.
// Search only for known function signatures, and require each one to be unique
// before installing a hook. 0x80000000-0x81800000 covers the kernel/XAM
// address range used by the supported retail build.
void* FindUniqueSignature(
	const BYTE* signature,
	const char* mask,
	size_t signatureLength,
	const char* name) {
	const uintptr_t scanStart = 0x80000000;
	const uintptr_t scanEnd = 0x81800000;
	void* match = nullptr;
	unsigned int matchCount = 0;

	if (signatureLength == 0 || signatureLength > scanEnd - scanStart)
		return nullptr;

	for (uintptr_t pageStart = scanStart; pageStart < scanEnd; pageStart += 0x1000) {
		if (!MmIsAddressValid((void*)pageStart))
			continue;

		uintptr_t pageEnd = pageStart + 0x1000;
		if (pageEnd > scanEnd)
			pageEnd = scanEnd;
		if (pageEnd - pageStart < signatureLength)
			continue;

		for (uintptr_t address = pageStart;
			address <= pageEnd - signatureLength;
			address += sizeof(DWORD)) {
			const BYTE* candidate = (const BYTE*)address;
			size_t i = 0;
			for (; i < signatureLength; ++i) {
				if (mask[i] != '?' && candidate[i] != signature[i])
					break;
			}
			if (i == signatureLength) {
				match = (void*)address;
				if (++matchCount > 1)
					break;
			}
		}
		if (matchCount > 1)
			break;
	}

	if (matchCount != 1) {
		DbgPrint("EINTIM: Signature for %s matched %d times; refusing to hook.\n", name, matchCount);
		return nullptr;
	}

	DbgPrint("EINTIM: Resolved %s at %08X.\n", name, (DWORD)match);
	return match;
}

bool HasExpectedInstruction(uintptr_t address, DWORD expected) {
	return MmIsAddressValid((void*)address) && *(DWORD*)address == expected;
}

bool initFunctionPointers(bool is17502Retail) {
	// 17502 is a retail dashboard build. Avoid probing a 17559-specific address
	// to distinguish devkits on this older kernel.
	isDevkit = is17502Retail ? false : (*(uint32_t*)(0x8010D334) == 0x00000000);
	HANDLE kernelHandle = GetModuleHandleA("xboxkrnl.exe");

	if (!kernelHandle) {
		DbgPrint("EINTIM: COULDNT GET KERNEL HANDLE!\n");
		return false;
	}

	HANDLE xamHandle = GetModuleHandleA("xam.xex");
	if (!xamHandle) {
		DbgPrint("EINTIM: COULDNT GET XAM HANDLE!\n");
		return false;
	}

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
	XexGetProcedureAddress(xamHandle, 1183, &NotificationPatchPtr);
	if (!UsbdGetDeviceDescriptor || !UsbdGetEndpointDescriptor ||
		!UsbdAddDeviceComplete || !UsbdOpenDefaultEndpoint || !UsbdOpenEndpoint ||
		!UsbdGetDeviceSpeed || !UsbdQueueAsyncTransfer || !UsbdQueueCloseEndpoint ||
		!UsbdQueueCloseDefaultEndpoint || !UsbdRemoveDeviceComplete ||
		!XInputdReadStatePtr || !XamInputGetCapabilitiesEx || !XamInputSetState ||
		(!is17502Retail && !MmFreePhysicalMemory)) {
		DbgPrint("EINTIM: Required kernel/XAM exports were not resolved.\n");
		return false;
	}

	if (is17502Retail) {
		// These 17502 retail signatures were checked against the reconstructed
		// CF/CG kernel and XAM payloads; each signature occurs once in its image.
		static const BYTE hidAddSignature[] = {
			0x7D, 0x88, 0x02, 0xA6, 0, 0, 0, 0, 0x94, 0x21, 0, 0,
			0x7C, 0x7B, 0x1B, 0x78, 0, 0, 0, 0, 0x7C, 0x7F, 0x1B, 0x79
		};
		static const char hidAddMask[] = "xxxx????xx??xxxx????xxxx";
		static const BYTE hidRemoveSignature[] = {
			0x81, 0x63, 0, 0, 0x39, 0x40, 0, 0, 0x39, 0x20, 0, 0, 0x99, 0x4B
		};
		static const char hidRemoveMask[] = "xx??xx??xx??xx";
		static const BYTE interfaceDescriptorSignature[] = {
			0x89, 0x43, 0, 0, 0x3D, 0x60, 0, 0, 0x39, 0x6B, 0, 0,
			0x55, 0x4A, 0xFF, 0x3A, 0x7D, 0x6A,
			0x58, 0x2E, 0xA1, 0x4B
		};
		static const char interfaceDescriptorMask[] = "xx??xx??xx??xxxxxxxxxx";
		static const BYTE bindDeviceSignature[] = {
			0x7C, 0x8B, 0x23, 0x78, 0x7C, 0xA4, 0x2B, 0x78, 0x54, 0xCA, 0x06, 0x3F
		};
		static const char bindDeviceMask[] = "xxxxxxxxxxxx";
		static const BYTE notifyConfigSignature[] = {
			0x81, 0x4A, 0x00, 0xE0, 0x2B, 0x0A, 0x00, 0x2F,
			0x39, 0x40, 0x27, 0x10, 0x41, 0x9A, 0x00, 0x08,
			0x39, 0x40, 0x13, 0x88, 0x91, 0x4B, 0x00, 0x18,
			0x3D, 0x40, 0x81, 0x5F
		};
		static const char notifyConfigMask[] = "xxxxxxxxxxxxxxxxxxxxxxxxxxxx";

		HidAddDeviceAddress = FindUniqueSignature(
			hidAddSignature, hidAddMask, sizeof(hidAddSignature), "HidAddDevice");
		HidRemoveDeviceAddress = FindUniqueSignature(
			hidRemoveSignature, hidRemoveMask, sizeof(hidRemoveSignature), "HidRemoveDevice");
		UsbdGetInterfaceDescriptor = (usb_interface_descriptor_func_t)FindUniqueSignature(
			interfaceDescriptorSignature, interfaceDescriptorMask,
			sizeof(interfaceDescriptorSignature), "UsbdGetInterfaceDescriptor");
		XamUserBindDeviceCallback = (xam_user_bind_device_callback_func_t)FindUniqueSignature(
			bindDeviceSignature, bindDeviceMask, sizeof(bindDeviceSignature), "XamUserBindDeviceCallback");
		void* notifyConfig = FindUniqueSignature(
			notifyConfigSignature, notifyConfigMask, sizeof(notifyConfigSignature), "XAM notification config");
		if (notifyConfig) {
			// The matched PPC block has the type limit immediate at +6 and timer
			// immediate at +10. The 17502 XAM contains one exact match.
			XNotifyTypeLimitPtr = (uint16_t*)((BYTE*)notifyConfig + 6);
			XNotifyTimerPtr = (uint16_t*)((BYTE*)notifyConfig + 10);
		}

		if (!HidAddDeviceAddress || !HidRemoveDeviceAddress ||
			!UsbdGetInterfaceDescriptor || !XamUserBindDeviceCallback) {
			DbgPrint("EINTIM: Could not resolve all 17502 retail signatures.\n");
			return false;
		}

		// 17502's reset sites were recovered from the exact retail kernel image.
		// Verify every patch site first; if the running kernel differs, keep the
		// USB stack intact and fall back to reconnecting controllers after load.
		UsbdDriverEntry = (usbd_powerdown_notification_func_t)0x800D9D58;
		UsbdPowerDownNotification = (usbd_powerdown_notification_func_t)0x800DA018;
		UsbPhysicalPage = 0x801A8DF8;
		g_SkipUsbDriverReset = true;
		const bool resetSitesMatch = MmFreePhysicalMemory &&
			HasExpectedInstruction(0x800D9D58, 0x7D8802A6) &&
			HasExpectedInstruction(0x800D9D60, 0x9421FF70) &&
			HasExpectedInstruction(0x800D9DB4, 0x3F80801B) &&
			HasExpectedInstruction(0x800D9DB8, 0x93FC8DF8) &&
			HasExpectedInstruction(0x800DA018, 0x3D60800E) &&
			HasExpectedInstruction(0x800E1634, 0x40820018) &&
			HasExpectedInstruction(0x800DE930, 0x40820018) &&
			HasExpectedInstruction(0x800D9F50, 0x4BF8DB19) &&
			HasExpectedInstruction(0x800D9F40, 0x4BF97079) &&
			MmIsAddressValid((void*)UsbPhysicalPage);
		if (resetSitesMatch) {
			*(DWORD*)0x800E1634 = 0x48000018;
			*(DWORD*)0x800DE930 = 0x48000018;
			*(DWORD*)0x800D9F50 = 0x60000000;
			*(DWORD*)0x800D9F40 = 0x60000000;
			g_SkipUsbDriverReset = false;
			DbgPrint("EINTIM: Verified 17502 USB reset sites.\n");
		}
		else {
			DbgPrint("EINTIM: 17502 USB reset sites differ; keeping USB running. Reconnect controllers after load.\n");
		}
	}
	else if (isDevkit) {
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

		XNotifyTypeLimitPtr = (uint16_t*)0x8176A7C6; // Register custom notification type condition
		XNotifyTimerPtr = (uint16_t*)0x8176a7ca;
		HidAddDeviceAddress = (void*)0x8011AE38;
		HidRemoveDeviceAddress = (void*)0x8011ADF8;
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

		XNotifyTypeLimitPtr = (uint16_t*)0x816AB7A6; // Register custom notification type condition
		XNotifyTimerPtr = (uint16_t*)0x816ab7aa;
		HidAddDeviceAddress = (void*)0x800E4D68;
		HidRemoveDeviceAddress = (void*)0x800E4D28;
	}

	if (XNotifyTypeLimitPtr)
		*XNotifyTypeLimitPtr = 80;
	if (XNotifyTimerPtr)
		*XNotifyTimerPtr = 1500;

	// Patches notification handling to work without JRPC2, Thanks crow!
	// This guarded branch patch also matches the 17502 retail XAM ordinal 1183.
	if (NotificationPatchPtr && *(short*)((uintptr_t)(NotificationPatchPtr) + 48) == 0x409A) {
		*(short*)((uintptr_t)(NotificationPatchPtr) + 48) = 0x4800;
	}
	if (!UsbdGetInterfaceDescriptor || !XamUserBindDeviceCallback ||
		!HidAddDeviceAddress || !HidRemoveDeviceAddress) {
		DbgPrint("EINTIM: Required HID functions were not resolved.\n");
		return false;
	}
	if (!g_SkipUsbDriverReset &&
		(!UsbdPowerDownNotification || !UsbdDriverEntry || !UsbPhysicalPage)) {
		DbgPrint("EINTIM: Required USB reset functions were not resolved.\n");
		return false;
	}

	return true;
}

BOOL APIENTRY DllMain(HANDLE Handle, DWORD Reason, PVOID Reserved) {
	if (Reason == DLL_PROCESS_ATTACH) {
		const bool is17502Retail = XboxKrnlVersion->Build == 17502;
		if ((XboxKrnlVersion->Build != 17559 && XboxKrnlVersion->Build != 17489 && !is17502Retail) || IsTrayOpen()) {
			DbgPrint("EINTIM: Supported dashboards are 17559 retail, 17489 devkit, and experimental 17502 retail; the disk tray must be closed. Aborting launch...\n");
			return FALSE;
		}

		DbgPrint("EINTIM: HELLO from xbox 360 HID controller driver version 0.6 beta\n");
		if (!initFunctionPointers(is17502Retail))
			return FALSE;

		DbgPrint("EINTIM: Loading mappings!\r\n");
		if (!LoadMappingsFromFile("HDD:\\hiddriver.json")) {
			DbgPrint("EINTIM: Failed to load mappings(JSON either doesn't exist yet or syntax error)!\r\n");
		}

		HidAddDeviceDetour = Detour(HidAddDeviceAddress, (void*)HidAddDeviceHook);
		HidRemoveDeviceDetour = Detour(HidRemoveDeviceAddress, (void*)HidRemoveDeviceHook);

		HidAddDeviceDetour.Install();
		HidRemoveDeviceDetour.Install();

		XamInputGetCapabilitiesDetour = Detour(XamInputGetCapabilitiesEx, (void*)XamInputGetCapabilitiesExHook);
		XamInputSetStateDetour = Detour(XamInputSetState, (void*)XamInputSetStateHook);
		XInputdReadStateDetour = Detour(XInputdReadStatePtr, (void*)XInputdReadStateHook);

		XamInputSetStateDetour.Install();
		XamInputGetCapabilitiesDetour.Install();
		XInputdReadStateDetour.Install();
		DbgPrint("EINTIM: Hooks installed\n");

		if (!g_SkipUsbDriverReset) {
			// This is a dirty way of forcing the system to reenumerate USB devices so you don't need to replug the controllers
			DbgPrint("EINTIM: Resetting USB driver!\n");
			UsbdPowerDownNotification();

			// For some reason microsoft doesnt clean up this page by themselves in the shutdown notification, so ill do it for them, call me mr nice guy :)
			MmFreePhysicalMemory(0, *(DWORD*)UsbPhysicalPage);
			DbgPrint("EINTIM: USB driver shutdown complete.\n");

			UsbdDriverEntry();
			DbgPrint("EINTIM: USB driver reset complete.\n");
		}
		else {
			DbgPrint("EINTIM: 17502 mode keeps the USB stack running; reconnect controllers after loading the plugin.\n");
		}

		// Start mapping manager thread
		MakeThread((LPTHREAD_START_ROUTINE)MappingManagerThreadProc, nullptr);
	}
	return TRUE;
}
