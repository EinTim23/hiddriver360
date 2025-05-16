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
Detour HidAddDeviceDetour;
Detour HidRemoveDeviceDetour;
Detour XamInputGetStateDetour;

uint16_t swap_endianness_16(uint16_t val) {
    return (val >> 8) | (val << 8);
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


const uint16_t DUALSENSE_VENDOR_ID = 0x054c;
const uint16_t DUALSENSE_PRODUCT_ID = 0x0CE6;

struct Report {
    uint8_t reportId;
};

struct ButtonsReport : Report {
    uint8_t x;
    uint8_t y;
    uint8_t z;
    uint8_t rz;
    uint8_t rx;
    uint8_t ry;
    uint8_t vendorDefined_ff00_20;
    uint8_t triangle : 1; // this is byteswapped for ppc
    uint8_t circle : 1; //
    uint8_t cross : 1; //
    uint8_t square : 1; //
    uint8_t hatSwitch : 4; // byteswap end
    uint8_t r3 : 1; // byteswap start
    uint8_t l3 : 1; //
    uint8_t options : 1; //
    uint8_t create : 1; //
    uint8_t r2 : 1; //
    uint8_t l2 : 1; //
    uint8_t r1 : 1; //
    uint8_t l1 : 1; // bytswap end
    uint8_t pad : 5; // byteswap start
    uint8_t mute : 1; //
    uint8_t touchpad : 1; //
    uint8_t ps : 1; // byteswap end
};

struct deviceHandle;
struct __declspec(align(2)) HidControllerExtension
{
    deviceHandle* deviceHandle;
    DWORD interruptEndpoint;
    DWORD interruptHandler;
    DWORD dwordC;
    BYTE gap10[4];
    BYTE byte14;
    BYTE gap15[3];
    DWORD interruptData;
    DWORD packetSize;
    BYTE gap20[4];
    DWORD defaultEndpoint;
    DWORD keyboardCompletionHandler;
    DWORD defaultEndpoint2;
    BYTE gap30[4];
    BYTE byte34;
    BYTE gap35[3];
    DWORD dword38;
    DWORD dword3C;
    BYTE gap40[4];
    BYTE byte44;
    BYTE byte45;
    WORD word46;
    WORD interfaceNumber;
    WORD word4A;
    BYTE gap4C[4];
    DWORD cleanupHandler;
    BYTE gap54[24];
    DWORD queue;
    BYTE alwaysOne;
    BYTE alwaysOneTwo;
    BYTE unknownFlag;
    BYTE alwaysZero;
    BYTE cleanedUpDone;
    BYTE byte75;
    BYTE alwaysZeroTwo;
    unsigned __int8 deviceType;
    BYTE alwaysZeroThree;
    BYTE alwaysZeroFour;
};

struct deviceHandle // sizeof=0x4
{
    HidControllerExtension* driver;
};
#define USB_ENDPOINT_TYPE_CONTROL         0x00
#define USB_ENDPOINT_TYPE_ISOCHRONOUS     0x01
#define USB_ENDPOINT_TYPE_BULK            0x02
#define USB_ENDPOINT_TYPE_INTERRUPT       0x03

#define USB_DIRECTION_IN 1
#define USB_DIRECTION_OUT 0

typedef usb_device_descriptor* (*usb_device_descriptor_func_t)(deviceHandle* handle);
typedef usb_interface_descriptor* (*usb_interface_descriptor_func_t)(deviceHandle* handle);
typedef int (*usb_add_device_complete_func_t)(deviceHandle* handle, int status_code);
typedef int (*usb_get_device_speed_func_t)(deviceHandle* handle);
typedef int (*usb_queue_async_transfer_func_t)(deviceHandle* handle, void* endpoint);
typedef NTSTATUS(*usb_queue_close_endpoint_func_t)(deviceHandle* handle, void* endpoint);
typedef NTSTATUS(*usb_remove_device_complete_func_t)(deviceHandle* handle);
typedef NTSTATUS(*usb_close_default_endpoint_func_t)(deviceHandle* handle, DWORD* endpoint);
typedef NTSTATUS (*usb_open_default_endpoint_func_t)(deviceHandle* handle, DWORD* endpoint);
typedef NTSTATUS(*usb_open_endpoint_func_t)(deviceHandle* handle, int transfertype, int endpointAddress, int maxPacketLength, int interval, DWORD* endpoint);
typedef usb_endpoint_descriptor* (*usb_endpoint_descriptor_func_t)(deviceHandle* handle, int index, int transfertype, int direction);
typedef int (*xam_user_bind_device_callback_func_t)(unsigned int controllerId, unsigned int context, unsigned __int8 category, bool disconnect, unsigned __int8* userIndex);


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

struct Controller {
    deviceHandle* deviceHandle;
    HidControllerExtension* controllerDriver;
    ButtonsReport currentState;
    uint8_t userIndex;
    void* reportData;
} __declspec(align(4));

Controller connectedControllers[4];

int interruptHandler(DWORD deviceHandle, int32_t a2) {
    HidControllerExtension* driverExtension = (HidControllerExtension*)((deviceHandle - 4));
    Report* report = (Report*)driverExtension->interruptData;
    if (!driverExtension || !driverExtension->deviceHandle || !driverExtension->deviceHandle->driver || driverExtension->deviceHandle->driver->cleanedUpDone)
        return 0;

    if (report->reportId == 1) {
        ButtonsReport* buttonReport = (ButtonsReport*)report;
        
        for (int i = 0; i < (sizeof(connectedControllers) / sizeof(Controller)); i++) {
            if (connectedControllers[i].controllerDriver == driverExtension) {
                connectedControllers[i].currentState = *buttonReport;
                break;
            }
        }
    }

    return UsbdQueueAsyncTransfer(driverExtension->deviceHandle, &driverExtension->interruptEndpoint);
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

    if (!deviceHandle2->driver->cleanedUpDone) {
        deviceHandle2->driver->cleanedUpDone = 1;
        connectedControllers[index].controllerDriver = nullptr;
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
    usb_endpoint_descriptor* endpoint_descriptor = UsbdGetEndpointDescriptor(deviceHandle, 0, USB_ENDPOINT_TYPE_INTERRUPT, USB_DIRECTION_IN);

    uint16_t vendorId = swap_endianness_16(device_descriptor->idVendor);
    uint16_t productId = swap_endianness_16(device_descriptor->idProduct);

    int speed = UsbdGetDeviceSpeed(deviceHandle);
    bool isOhci = speed == 0;

    DbgPrint("EINTIM: IS USB1.0: %d\n", isOhci);
    DbgPrint("EINTIM: USB device descriptor Pointer: %p\n", device_descriptor);
    DbgPrint("EINTIM: USB interface descriptor Pointer: %p\n", interface_descriptor);
    DbgPrint("EINTIM: USB endpoint interrupt in descriptor Pointer: %p\n", endpoint_descriptor);
    DbgPrint("EINTIM: HID device vendor id: %x, product id: %x\n", vendorId, productId);

    if (vendorId == DUALSENSE_VENDOR_ID && productId == DUALSENSE_PRODUCT_ID) {
        DbgPrint("EINTIM: DualSense PS5 controller detected. Initialising custom handler.\n");
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
       
        Controller c = Controller();
        HidControllerExtension* controllerDriver = new HidControllerExtension();
        c.deviceHandle = deviceHandle;

        controllerDriver->deviceType = 0; // Hid device type, for controllers it's zero.
        controllerDriver->alwaysOne = 1;
        deviceHandle->driver = controllerDriver;
        controllerDriver->deviceHandle = deviceHandle;
        controllerDriver->alwaysZero = 0;
        controllerDriver->alwaysZeroTwo = 0;
        controllerDriver->alwaysOneTwo = 1;
        controllerDriver->unknownFlag = 0;       // should be zero if type is 0, otherwise 1
        controllerDriver->alwaysZeroThree = 0;
        controllerDriver->alwaysZeroFour = 0;

        controllerDriver->byte14 = 1;

        UsbdAddDeviceComplete(deviceHandle, 0);

        NTSTATUS status = UsbdOpenDefaultEndpoint(deviceHandle, &controllerDriver->defaultEndpoint);
        if (NT_ERROR(status)) {
            DbgPrint("EINTIM: Failed to open control endpoint %x!\n", status);
            return status;
        }

        status = UsbdOpenEndpoint(deviceHandle, 3, endpoint_descriptor->bEndpointAddress, swap_endianness_16(endpoint_descriptor->wMaxPacketSize) & 0x7FF, endpoint_descriptor->bInterval, &controllerDriver->interruptEndpoint);
        if (NT_ERROR(status)) {
            DbgPrint("EINTIM: Failed to open interrupt endpoint %x!\n", status);
            return status;
        }
        controllerDriver->dwordC = controllerDriver->interruptEndpoint;
        controllerDriver->packetSize = swap_endianness_16(endpoint_descriptor->wMaxPacketSize) & 0x7FF;
        controllerDriver->byte75 = 1;
        controllerDriver->byte44 = 33;
        controllerDriver->byte45 = 10;
        controllerDriver->word46 = 0;
        controllerDriver->interfaceNumber = swap_endianness_16(interface_descriptor->bInterfaceNumber);
        controllerDriver->word4A = 0;
        controllerDriver->dword38 = 0;
        controllerDriver->dword3C = 0;
        controllerDriver->byte34 = 1;

        controllerDriver->interruptHandler = (DWORD)interruptHandler;
        c.reportData = malloc((swap_endianness_16(endpoint_descriptor->wMaxPacketSize) & 0x7FF) * 2);
        memset(c.reportData, 0, (swap_endianness_16(endpoint_descriptor->wMaxPacketSize) & 0x7FF) * 2);
        controllerDriver->interruptData = (DWORD)c.reportData;
        
        c.controllerDriver = controllerDriver;

        uint8_t userIndex = -1;
        XamUserBindDeviceCallback(0xa7553952 + index, 0x0000000010000005 + index, 0, false, &userIndex);
        c.userIndex = userIndex;
        connectedControllers[index] = c;

        DbgPrint("EINTIM: Registered virtual controller inside XAM with index: %d.\n", userIndex);
        return UsbdQueueAsyncTransfer(deviceHandle, &controllerDriver->interruptEndpoint);
    }

    DbgPrint("EINTIM: Unrelated USB Device. Calling original...\n");
	return HidAddDeviceDetour.GetOriginal<decltype(&HidAddDeviceHook)>()(deviceHandle);
}


int16_t ConvertToFullRange(uint8_t input, bool invert_y = false) {
    if(!invert_y) 
        return static_cast<int16_t>((input - 128) * 256);
    else
        return static_cast<int16_t>((~(input) - 128) * 256);
}

DWORD XamInputGetStateHook(DWORD user, DWORD flags, XINPUT_STATE* input_state) {
    DWORD status = XamInputGetStateDetour.GetOriginal<decltype(&XamInputGetStateHook)>()(user, flags, input_state);

    if ((user & 0xFF) == 0xFF)
        user = 0;

    if (!input_state)
        return status;

    static DWORD lastPressTime = 0;  
    static const DWORD cooldownDuration = 1000; 

    if (status == ERROR_DEVICE_NOT_CONNECTED) {
        ButtonsReport b;
        Controller* c = nullptr;
        for (int i = 0; i < 4; i++) {
            if (connectedControllers[i].controllerDriver) {
                if (connectedControllers[i].userIndex == user) {
                    c = &connectedControllers[i];
                    b = connectedControllers[i].currentState;
                    break;
                }
            }
        }

        if (!c)
            return status;


        if (b.ps) {
            DWORD now = GetTickCount(); 
            if (now - lastPressTime >= cooldownDuration) {
                lastPressTime = now; 
                XamInputSendXenonButtonPress(user);  
            }
        }

        if (b.cross)
            input_state->Gamepad.wButtons |= XINPUT_GAMEPAD_A;

        if (b.circle)
            input_state->Gamepad.wButtons |= XINPUT_GAMEPAD_B;

        if (b.triangle)
            input_state->Gamepad.wButtons |= XINPUT_GAMEPAD_Y;

        if (b.square)
            input_state->Gamepad.wButtons |= XINPUT_GAMEPAD_X;

        if (b.options)
            input_state->Gamepad.wButtons |= XINPUT_GAMEPAD_START;

        if (b.mute)
            input_state->Gamepad.wButtons |= XINPUT_GAMEPAD_BACK;

        if (b.r3)
            input_state->Gamepad.wButtons |= XINPUT_GAMEPAD_RIGHT_THUMB;

        if (b.l3)
            input_state->Gamepad.wButtons |= XINPUT_GAMEPAD_LEFT_THUMB;

        if (b.l1)
            input_state->Gamepad.wButtons |= XINPUT_GAMEPAD_LEFT_SHOULDER;

        if (b.r1)
            input_state->Gamepad.wButtons |= XINPUT_GAMEPAD_RIGHT_SHOULDER;

        if (b.hatSwitch == 0)
            input_state->Gamepad.wButtons |= XINPUT_GAMEPAD_DPAD_UP;

        if (b.hatSwitch == 2)
            input_state->Gamepad.wButtons |= XINPUT_GAMEPAD_DPAD_RIGHT;

        if (b.hatSwitch == 4)
            input_state->Gamepad.wButtons |= XINPUT_GAMEPAD_DPAD_DOWN;

        if (b.hatSwitch == 6)
            input_state->Gamepad.wButtons |= XINPUT_GAMEPAD_DPAD_LEFT;

        input_state->Gamepad.sThumbRX = ConvertToFullRange(b.z);
        input_state->Gamepad.sThumbRY = ConvertToFullRange(b.rz, true);

        input_state->Gamepad.sThumbLX = ConvertToFullRange(b.x);
        input_state->Gamepad.sThumbLY = ConvertToFullRange(b.y, true);

        input_state->Gamepad.bLeftTrigger = b.rx;
        input_state->Gamepad.bRightTrigger = b.ry;

        return ERROR_SUCCESS;
    }
    return status;
}

void* XamInputGetState = nullptr;
bool isDevkit = true;

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

    XexGetProcedureAddress(xamHandle, 401, &XamInputGetState);

    if (isDevkit) {
        DbgPrint("EINTIM: Running in devkit mode\n");
        UsbdGetInterfaceDescriptor = (usb_interface_descriptor_func_t)0x8010D2D0; // 89 43 ? ? 3D 60 ? ? 89 2D ? ? 39 6B ? ? 55 4A FF 3A 2B 09 ? ? 7D 6A 58 2E ? ? ? ? ? ? ? ? 89 4D ? ? 2B 0A ? ? ? ? ? ? ? ? ? ? 81 4B ? ? 7F 03 50 40 ? ? ? ? ? ? ? ? A1 4B very bad direct signature. XREF sig: 89 63 ? ? 38 A1
        XamUserBindDeviceCallback = (xam_user_bind_device_callback_func_t)0x817A34B8; // 7C 8B 23 78 7C A4 2B 78 54 CA 06 3F
    }
    else {
        DbgPrint("EINTIM: Running in retail mode\n");
        UsbdGetInterfaceDescriptor = (usb_interface_descriptor_func_t)0x800D8500; // 89 43 ? ? 3D 60 ? ? 39 6B ? ? 55 4A FF 3A 7D 6A 58 2E A1 4B
        XamUserBindDeviceCallback = (xam_user_bind_device_callback_func_t)0x816D9060; // 7C 8B 23 78 7C A4 2B 78 54 CA 06 3F
    }

    return true;
}

BOOL APIENTRY DllMain(HANDLE Handle, DWORD Reason, PVOID Reserved)
{
	if (Reason == DLL_PROCESS_ATTACH)
	{
        if (XboxKrnlVersion->Build != 17559 && XboxKrnlVersion->Build != 17489) {
            DbgPrint("EINTIM: Only 17559 and 17489 dashboards are currently supported.\n");
            return FALSE;
        }

		DbgPrint("EINTIM: HELLO from xbox 360 HID controller driver\n");
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

        XamInputGetStateDetour = Detour(XamInputGetState, (void*)XamInputGetStateHook);
        XamInputGetStateDetour.Install();
		DbgPrint("EINTIM: Hooks installed\n");
	}
	return TRUE;
}