#ifndef __KERNEL_DEFINES_H
#define __KERNEL_DEFINES_H

#pragma comment(lib, "xkelib")

#include "keFileInfo.h"
#include "keXexInfo.h"
#include "kePrivateStructs.h"
#include "keXeCrypt.h"
#include "keXeKeys.h"
#include "keXConfig.h"
#include "keSmc.h"
#include "keNand.h"

#define OBJ_SYS_STRING	"\\System??\\%s"
#define OBJ_USR_STRING	"\\??\\%s"
#define OBJ_SYS_PATH	"\\System??\\"
#define OBJ_USR_PATH	"\\??\\"

// common modules
#define MODULE_KERNEL 	"xboxkrnl.exe"
#define MODULE_XAM  	"xam.xex"
#define MODULE_DASH 	"dash.xex"
#define MODULE_HUD  	"hud.xex"
#define MODULE_LAUNCH 	"launch.xex"
#define MODULE_XBDM 	"xbdm.xex"

// some handy intrinsics and macros that are left out...
#define __isync()		__emit(0x4C00012C)

#define doSync(addr) \
	do { \
	__dcbst(0, addr); \
	__sync(); \
	__isync(); \
	} while (0)

#define doLightSync(addr) \
	do { \
	__dcbst(0, addr); \
	__sync(); \
	} while (0)

// debug serial port register
#define DBG_SERIAL_RCV		(DWORD volatile*)0x7FEA1010
#define DBG_SERIAL_XMIT		(DWORD volatile*)0x7FEA1014
#define DBG_SERIAL_STS		(DWORD volatile*)0x7FEA1018
#define DBG_SERIAL_CNTRL	(DWORD volatile*)0x7FEA101C

#define FILE_DIRECTORY_FILE                     0x00000001 
#define FILE_WRITE_THROUGH                      0x00000002 
#define FILE_SEQUENTIAL_ONLY                    0x00000004 
#define FILE_NO_INTERMEDIATE_BUFFERING          0x00000008 

#define FILE_SYNCHRONOUS_IO_ALERT               0x00000010 
#define FILE_SYNCHRONOUS_IO_NONALERT            0x00000020 
#define FILE_NON_DIRECTORY_FILE                 0x00000040 
#define FILE_CREATE_TREE_CONNECTION             0x00000080 

#define FILE_COMPLETE_IF_OPLOCKED               0x00000100 
#define FILE_NO_EA_KNOWLEDGE                    0x00000200 
//UNUSED                                        0x00000400 
#define FILE_RANDOM_ACCESS                      0x00000800 

#define FILE_DELETE_ON_CLOSE                    0x00001000 
#define FILE_OPEN_BY_FILE_ID                    0x00002000 
#define FILE_OPEN_FOR_BACKUP_INTENT             0x00004000 
#define FILE_NO_COMPRESSION                     0x00008000 

// Valid values for object Attributes field
#define OBJ_INHERIT             0x00000002L
#define OBJ_PERMANENT           0x00000010L
#define OBJ_EXCLUSIVE           0x00000020L
#define OBJ_CASE_INSENSITIVE    0x00000040L
#define OBJ_OPENIF              0x00000080L
#define OBJ_OPENLINK            0x00000100L
#define OBJ_VALID_ATTRIBUTES    0x000001F2L

// Directory Stuff
#define DIRECTORY_QUERY                 (0x0001)
#define DIRECTORY_TRAVERSE              (0x0002)
#define DIRECTORY_CREATE_OBJECT         (0x0004)
#define DIRECTORY_CREATE_SUBDIRECTORY   (0x0008)
#define DIRECTORY_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED | 0xF)

#define SYMBOLIC_LINK_QUERY (0x0001)

// object type strings
#define OBJ_TYP_SYMBLINK	0x626d7953 // Symb
#define OBJ_TYP_DIRECTORY	0x65726944 // Dire
#define OBJ_TYP_DEVICE		0x69766544 // Devi
#define OBJ_TYP_EVENT		0x76657645 // Evev
#define OBJ_TYP_DEBUG		0x63706d64 // dmpc

// flags for xex load image dwModuleTypeFlags
#define XEX_LOADIMG_FLAG_TITLE_PROCESS   0x00000001
#define XEX_LOADIMG_FLAG_TITLE_IMPORTS   0x00000002
#define XEX_LOADIMG_FLAG_DEBUGGER        0x00000004
#define XEX_LOADIMG_FLAG_DLL             0x00000008
#define XEX_LOADIMG_FLAG_PATCH           0x00000010
#define XEX_LOADIMG_FLAG_PATCH_FULL      0x00000020
#define XEX_LOADIMG_FLAG_PATCH_DELTA     0x00000040
#define XEX_LOADIMG_FLAG_BOUND_PATH      0x40000000
#define XEX_LOADIMG_FLAG_SILENT_LOAD     0x80000000
#define XEX_LOADIMG_TYPE_TITLE 			(XEX_LOADIMG_FLAG_TITLE_PROCESS)
#define XEX_LOADIMG_TYPE_TITLE_DLL 		(XEX_LOADIMG_FLAG_TITLE_PROCESS | XEX_LOADIMG_FLAG_DLL)
#define XEX_LOADIMG_TYPE_SYSTEM_APP 	(XEX_LOADIMG_FLAG_DLL)
#define XEX_LOADIMG_TYPE_SYSTEM_DLL 	(XEX_LOADIMG_FLAG_DLL | XEX_LOADIMG_FLAG_TITLE_IMPORTS)

// flags used by ExCreateThread
#define EX_CREATE_FLAG_SUSPENDED	0x1 // thread created suspended
#define EX_CREATE_FLAG_SYSTEM		0x2 // create a system thread
//??? #define EX_CREATE_FLAG_HIDDEN		0x4 // hides the thread from debugger thread list???
#define EX_CREATE_FLAG_TITLE_EXEC	0x100 // title execution thread

typedef enum _POOL_TYPE {
	PoolTypeThread = 0x0,
	PoolTypeTitle = 0x1,
	PoolTypeSystem = 0x2,
	PoolTypeTitleDebug = 0x3,
	PoolTypeSystemDebug = 0x4,
} POOL_TYPE;

typedef enum { // effects on jtag noted in comment
	HalHaltRoutine = 0x0, // hard poweroff (used by hotplug HDD) HalpPowerDownSystemNow
	HalRebootRoutine = 0x1, // hard reset (video error) HalpRebootSystem
	HalKdRebootRoutine = 0x2, // hard reset (used for dumpwritedump/frozen processor) HalpRebootSystem
	HalFatalErrorRebootRoutine = 0x3, // hard reset HalpRebootSystem
	HalResetSMCRoutine = 0x4, // power off (hard) HalpRebootSystem
	HalPowerDownRoutine = 0x5, // power off (nice) HalpPowerDownSystem
	HalRebootQuiesceRoutine = 0x6, // E79 (lost settings) HalpRebootQuiesceSystem
	HalForceShutdownRoutine = 0x7, // frozen console HalpRequestPowerDownDpc
	HalPowerCycleQuiesceRoutine = 0x8,
	HalMaximumRoutine = 0x9,
} FIRMWARE_REENTRY;

/* description about xex exe headers in memory */
typedef struct _XBOX_HARDWARE_INFO {
	DWORD Flags;
	unsigned char NumberOfProcessors;
	unsigned char PCIBridgeRevisionID;
	unsigned char Reserved[6];
	unsigned short BldrMagic;
	unsigned short BldrFlags;
} XBOX_HARDWARE_INFO, *PXBOX_HARDWARE_INFO;

typedef struct _EX_TITLE_TERMINATE_REGISTRATION
{
	void* NotificationRoutine; // function pointer
	u32 Priority; // xam uses 0x7C800000 for early and 0x0 for late
	LIST_ENTRY ListEntry; // already defined in winnt.h
} EX_TITLE_TERMINATE_REGISTRATION, *PEX_TITLE_TERMINATE_REGISTRATION;

typedef struct _EX_THREAD_REGISTRATION {
	void* NotificationRoutine;
	DWORD Priority;
	LIST_ENTRY ListEntry;
} EX_THREAD_REGISTRATION, *PEX_THREAD_REGISTRATION;

typedef struct _VD_NOTIFICATION_REGISTRATION { 
	PVOID NotificationRoutine; // 0x0 sz:0x4 void(*)()
	long Priority; // 0x4 sz:0x4
	LIST_ENTRY ListEntry; // 0x8 sz:0x8
} VD_NOTIFICATION_REGISTRATION, *PVD_NOTIFICATION_REGISTRATION; // size 16
C_ASSERT(sizeof(VD_NOTIFICATION_REGISTRATION) == 0x10);

typedef void (*pfnThreadNotificationRoutine)(PEX_THREAD_REGISTRATION ThisRegistration, PKTHREAD Thread, BOOL Creating);
typedef int (*LP_VD_NOTIFICATION_CALLBACK)(const PVD_NOTIFICATION_REGISTRATION vdReg, DWORD type, void * buf);

typedef struct _XBOX_KRNL_VERSION{
	WORD Major; // for 360 this is always 2
	WORD Minor; // usually 0
	WORD Build; // current version, for example 9199
	WORD Qfe;
} XBOX_KRNL_VERSION, *PXBOX_KRNL_VERSION;

typedef struct _KTIME_STAMP_BUNDLE { 
	LARGE_INTEGER InterruptTime; // 0
	LARGE_INTEGER SystemTime; // 8
	DWORD TickCount; // 10
} KTIME_STAMP_BUNDLE, *PKTIME_STAMP_BUNDLE;

typedef struct _LZX_DECOMPRESS {
	LONG WindowSize;
	LONG CpuType;
} LZX_DECOMPRESS, *PLZX_DECOMPRESS;

#ifdef __cplusplus
extern "C" {
#endif

	NTSYSAPI
	EXPORTNUM(3)
	VOID
	NTAPI
	DbgPrint(
		IN		const char* s,
		...
	);

	// http://msdn.microsoft.com/en-us/library/ff544520%28v=vs.85%29.aspx
	// pool functions return NULL if there is insufficient memory
	// uses POOL_TYPE NonPagedPool
	// uses tag 'enoN' (None)
	NTSYSAPI
	EXPORTNUM(9)
	PVOID
	NTAPI
	ExAllocatePool(
		IN  	DWORD NumberOfBytes
	);

	// uses POOL_TYPE NonPagedPool
	NTSYSAPI
	EXPORTNUM(10)
	PVOID
	NTAPI
	ExAllocatePoolWithTag(
		IN  DWORD NumberOfBytes,
		IN  DWORD Tag
	);

	NTSYSAPI
	EXPORTNUM(11)
	PVOID
	NTAPI
	ExAllocatePoolTypeWithTag(
		IN  DWORD NumberOfBytes,
		IN  DWORD Tag,
		IN  POOL_TYPE PoolType
	);

	EXPORTNUM(12)
	extern PDWORD ExConsoleGameRegion;

	NTSYSAPI
	EXPORTNUM(13)
	DWORD
	NTAPI
	ExCreateThread(
		IN		PHANDLE pHandle,
		IN		DWORD dwStackSize,
		IN		LPDWORD lpThreadId,
		IN		PVOID apiThreadStartup,
		IN		LPTHREAD_START_ROUTINE lpStartAddress,
		IN		LPVOID lpParameter,
		IN		DWORD dwCreationFlagsMod
	);

	NTSYSAPI
	EXPORTNUM(15)
	VOID
	NTAPI
	ExFreePool(
		IN PVOID  pPool
    );

	NTSYSAPI
	EXPORTNUM(16)
	NTSTATUS
	NTAPI
	ExGetXConfigSetting(
		IN		WORD dwCategory,
		IN		WORD dwSetting,
		OUT		PVOID pBuffer,
		IN		WORD cbBuffer,
		OUT		PWORD szSetting
	);
	
	NTSYSAPI
	EXPORTNUM(20)
	VOID
	NTAPI
	ExRegisterThreadNotification(
		IN OUT	PEX_THREAD_REGISTRATION threadNotification,
		IN		BOOL Create
	);

	NTSYSAPI
	EXPORTNUM(21)
	VOID
	NTAPI
	ExRegisterTitleTerminateNotification(
		IN OUT	PEX_TITLE_TERMINATE_REGISTRATION pTermStruct,
		IN		BOOL bCreate // true create, false destroy existing
	);

	NTSYSAPI
	EXPORTNUM(24)
	NTSTATUS
	NTAPI
	ExSetXConfigSetting(
		IN		WORD dwCategory,
		IN		WORD dwSetting,
		IN		PVOID pBuffer,
		IN		WORD szSetting
	);

	NTSYSAPI
	EXPORTNUM(25)
	VOID
	NTAPI
	ExTerminateThread(
		IN DWORD  exitCode
    );

	NTSYSAPI
	EXPORTNUM(36)
	VOID
	NTAPI
	HalOpenCloseODDTray(
		IN		DWORD setTray
	);

	NTSYSAPI
	EXPORTNUM(40)
	VOID
	NTAPI
	HalReturnToFirmware(
		IN		FIRMWARE_REENTRY powerDownMode
	);

	// buffers are 0x10 bytes in length, on cmd that recv no response send NULL for recv
	NTSYSAPI
	EXPORTNUM(41)
	VOID
	NTAPI
	HalSendSMCMessage(
		IN		LPVOID pCommandBuffer,
		OUT		LPVOID pRecvBuffer
	);

	NTSYSAPI
	EXPORTNUM(55)
	NTSTATUS
	NTAPI
	IoCreateDevice(
		IN		PDRIVER_OBJECT DriverObject,
		IN		DWORD DeviceExtensionSize, // ??
		IN		PSTRING DeviceName, // opt
		IN		DWORD DeviceType,
		IN		DWORD DeviceCharacteristics,
	//	IN		BOOL Exclusive,
		OUT		PDEVICE_OBJECT *DeviceObject
	);

	// returns STATUS_ACCESS_DENIED if not called by a system thread
	NTSYSAPI
	EXPORTNUM(66)
	BOOL
	NTAPI
	ExSetBetaFeaturesEnabled(
		IN		DWORD dwBetaFeature
	);

	NTSYSAPI
	EXPORTNUM(77)
	VOID
	NTAPI
	KeAcquireSpinLockAtRaisedIrql(
		IN OUT	PDWORD spinVar
	);

	NTSYSAPI
	EXPORTNUM(86)
	VOID
	NTAPI
	KeContextFromKframes(
		IN 		PKTRAP_FRAME pktf,
		OUT 	PCONTEXT pctx
	);

	NTSYSAPI
	EXPORTNUM(90)
	NTSTATUS
	NTAPI
	KeDelayExecutionThread(
		IN		KPROCESSOR_MODE WaitMode,
		IN		BOOL Alertable,
		IN		PLARGE_INTEGER Interval
	);

	NTSYSAPI
	EXPORTNUM(95)
	VOID
	NTAPI
	KeEnterCriticalRegion(
		VOID
	);

	// untested
	NTSYSAPI
	EXPORTNUM(97)
	VOID
	NTAPI
	KeFlushCacheRange(
		IN		PVOID address,
		IN		DWORD size
	);

	NTSYSAPI
	EXPORTNUM(102)
	UCHAR // enum _KPROC_TYPE
	NTAPI
	KeGetCurrentProcessType(
		VOID
	);

	NTSYSAPI
	EXPORTNUM(124)
	PDWORD
	NTAPI
	KeIpiGenericCall(
		IN  	PKIPI_BROADCAST_WORKER BroadcastFunction,
		IN  	PDWORD Context
	);

	NTSYSAPI
	EXPORTNUM(125)
	VOID
	NTAPI
	KeLeaveCriticalRegion(
		VOID
	);
	
	NTSYSAPI
	EXPORTNUM(132)
	NTSTATUS
	NTAPI
	KeQuerySystemTime(
		OUT		PFILETIME CurrentTime // LARGE_INTEGER
	);
	
	NTSYSAPI
	EXPORTNUM(134)
	VOID
	NTAPI
	KeRegisterDriverNotification(
		IN OUT	PKDRIVER_NOTIFICATION_REGISTRATION pDriverNotification,
		IN		DWORD dwNotificationType // _KDRIVER_NOTIFICATION_TYPE
	);

	NTSYSAPI
	EXPORTNUM(137)
	VOID
	NTAPI
	KeReleaseSpinLockFromRaisedIrql(
		IN OUT	PDWORD spinVar
	);

	NTSYSAPI
	EXPORTNUM(146)
	VOID
	NTAPI
	KeResumeThread(
		IN	PKTHREAD thread
	);

	NTSYSAPI
	EXPORTNUM(154)
	VOID
	NTAPI
	KeSetCurrentProcessType(
		IN		DWORD ProcessType
	);

	NTSYSAPI
	EXPORTNUM(157)
	HRESULT
	NTAPI
	KeSetEvent(
		IN 		HANDLE  Event,
		IN		DWORD  Increment,
		IN		BOOL  Wait
	);

	NTSYSAPI
	EXPORTNUM(168)
	BYTE
	NTAPI
	KeStallExecutionProcessor(
		IN		DWORD period
	);

	NTSYSAPI
	EXPORTNUM(169)
	VOID
	NTAPI
	KeSuspendThread(
		IN	PKTHREAD thread
	);

	EXPORTNUM(173)
	extern PKTIME_STAMP_BUNDLE KeTimeStampBundle;

	NTSYSAPI
	EXPORTNUM(176)
	NTSTATUS 
	NTAPI
	KeWaitForSingleObject(
		IN		PVOID Object,
		IN		KWAIT_REASON WaitReason,
		IN		KPROCESSOR_MODE WaitMode,
		IN		BOOL Alertable,
		IN		PLARGE_INTEGER Timeout OPTIONAL
	);

	/* spinlock example:
	DWORD g_spinvar;
	{
		BYTE irql;
		irql = KfAcquireSpinLock(&g_spinvar);
		... some code
		KfReleaseSpinLock(&g_spinvar, irql);
	}*/
	NTSYSAPI
	EXPORTNUM(177)
	BYTE
	NTAPI
	KfAcquireSpinLock(
		IN OUT	PDWORD spinVar
	);

	NTSYSAPI
	EXPORTNUM(178)
	BYTE
	NTAPI
	KfRaiseIrql(
		IN		BYTE irql
	);
	
	NTSYSAPI
	EXPORTNUM(179)
	VOID
	NTAPI
	KfLowerIrql(
		IN		BYTE irql
	);

	NTSYSAPI
	EXPORTNUM(180)
	VOID
	NTAPI
	KfReleaseSpinLock(
		IN OUT	PDWORD spinVar,
		IN		BYTE oldIrql
	);

	NTSYSAPI
	EXPORTNUM(182)
	HRESULT
	NTAPI
	LDICreateDecompression(
		IN		PVOID pcbDataBlockMax,
		IN		PLZX_DECOMPRESS pvConfiguration,
		IN		PVOID pfn_dec_malloc, // void * (__cdecl *)(unsigned long) dec_malloc,
		IN		PVOID pfn_dec_free, // void (__cdecl *)(void *) dec_free,
		IN		PVOID dec_memory,
		IN		PVOID pcbSrcBufferMin,
		OUT		PHANDLE pmdhHandle
	);

	NTSYSAPI
	EXPORTNUM(183)
	HRESULT
	NTAPI
	LDIDecompress(
		IN		HANDLE hmd,
		IN		PVOID pbSrc,
		IN		DWORD cbSrc,
		IN OUT	PVOID pbDst,
		IN OUT	PDWORD pcbResult
	);
	
	NTSYSAPI
	EXPORTNUM(184)
	HRESULT
	NTAPI
	LDIDestroyDecompression(
		IN		HANDLE hmd
	);

	//  - INCOMPLETE!!
	NTSYSAPI
	EXPORTNUM(185)
	PVOID
	NTAPI
	MmAllocatePhysicalMemory(
		IN		DWORD type, // 0 (2 for system?)
		IN		DWORD size, // 1
		IN		DWORD accessFlags //0x20000004 - gives 1 64k phy alloc
	);

	NTSYSAPI
	EXPORTNUM(190)
	PVOID
	NTAPI
	MmGetPhysicalAddress(
		IN		PVOID Address
	);

	NTSYSAPI
	EXPORTNUM(191)
	BOOL
	NTAPI
	MmIsAddressValid(
		IN		PVOID Address
	);

	NTSYSAPI
	EXPORTNUM(196)
	DWORD
	NTAPI
	MmQueryAddressProtect(
		IN		PVOID Address
	);

	NTSYSAPI
	EXPORTNUM(197)
	DWORD
	NTAPI
	MmQueryAllocationSize(
		IN		PVOID Address
	);

	NTSYSAPI
	EXPORTNUM(198)
	DWORD
	NTAPI
	MmQueryStatistics(
		OUT		PMMSTATISTICS pMmStat
	);

	NTSYSAPI
	EXPORTNUM(199)
	DWORD
	NTAPI
	MmSetAddressProtect(
		IN		PVOID Address,
		IN		DWORD Size,
		IN		DWORD Type
	);// PAGE_READWRITE

	NTSYSAPI
	EXPORTNUM(204)
	PVOID
	NTAPI
	NtAllocateVirtualMemory(
		IN OUT 	LPVOID* lpAddress,
		IN		SIZE_T* dwSize,
		IN		DWORD flAllocationType,
		IN		DWORD flProtect,
		IN		DWORD dwMemoryRegionType
	);

	NTSYSAPI
	EXPORTNUM(207)
	NTSTATUS
	NTAPI
	NtClose(
		IN		HANDLE Handle
	);

	NTSYSAPI
	EXPORTNUM(210)
	NTSTATUS
	NTAPI
	NtCreateFile(
		OUT		PHANDLE FileHandle,
		IN		ACCESS_MASK DesiredAccess,
		IN		POBJECT_ATTRIBUTES ObjectAttributes,
		OUT		PIO_STATUS_BLOCK IoStatusBlock,
		IN		PLARGE_INTEGER AllocationSize OPTIONAL,
		IN		DWORD FileAttributes,
		IN		DWORD ShareAccess,
		IN		DWORD CreateDisposition,
		IN		DWORD CreateOptions
	);

	//  UNTESTED
	NTSYSAPI
	EXPORTNUM(216)
	NTSTATUS
	NTAPI
	NtDeleteFile(
		IN 		POBJECT_ATTRIBUTES ObjectAttributes
	);

	NTSYSAPI
	EXPORTNUM(217)
	NTSTATUS
	NTAPI
	NtDeviceIoControlFile(
		IN		HANDLE FileHandle,
		IN		HANDLE Event OPTIONAL,
		IN		PIO_APC_ROUTINE ApcRoutine OPTIONAL,
		IN		PVOID ApcContext OPTIONAL,
		OUT		PIO_STATUS_BLOCK IoStatusBlock,
		IN		DWORD IoControlCode,
		IN		PVOID InputBuffer OPTIONAL,
		IN		DWORD InputBufferLength,
		OUT		PVOID OutputBuffer OPTIONAL,
		IN		DWORD OutputBufferLength
	);

	NTSYSAPI
	EXPORTNUM(222)
	NTSTATUS
	NTAPI
	NtOpenDirectoryObject(
		OUT		PHANDLE DirectoryHandle,
		IN		POBJECT_ATTRIBUTES ObjectAttributes
	);

	NTSYSAPI
	EXPORTNUM(223)
	NTSTATUS
	NTAPI
	NtOpenFile(
		OUT		PHANDLE FileHandle,
		IN		ACCESS_MASK DesiredAccess,
		IN		POBJECT_ATTRIBUTES ObjectAttributes,
		OUT		PIO_STATUS_BLOCK IoStatusBlock,
		IN		DWORD ShareAccess,
		IN		DWORD OpenOptions
	);

	NTSYSAPI
	EXPORTNUM(224)
	NTSTATUS
	NTAPI
	NtOpenSymbolicLinkObject(
		OUT		PHANDLE LinkHandle,
		IN		POBJECT_ATTRIBUTES ObjectAttributes
	);

	// UNTESTED!!
	NTSYSAPI
	EXPORTNUM(227)
	NTSTATUS
	NTAPI
	NtQueueApcThread(
		IN		HANDLE ThreadHandle,
		IN		PIO_APC_ROUTINE ApcRoutine,
		IN		PVOID ApcRoutineContext OPTIONAL,
		IN		PIO_STATUS_BLOCK ApcStatusBlock OPTIONAL,
		IN		DWORD ApcReserved OPTIONAL
	);

	NTSYSAPI 
	EXPORTNUM(228)
	NTSTATUS
	NTAPI
	NtQueryDirectoryFile(
		IN		HANDLE FileHandle,
		IN		HANDLE Event OPTIONAL,
		IN		PIO_APC_ROUTINE ApcRoutine OPTIONAL,
		IN		PVOID ApcContext OPTIONAL,
		OUT		PIO_STATUS_BLOCK IoStatusBlock,
		OUT		PVOID FileInformation,
		IN		DWORD Length,
		IN		PSTRING FileMask OPTIONAL,
		IN		DWORD Unk // usually 0
	);

	NTSYSAPI
	EXPORTNUM(229)
	NTSTATUS
	NTAPI
	NtQueryDirectoryObject(
		IN		HANDLE DirectoryHandle,
		OUT		PVOID Buffer OPTIONAL,
		IN		DWORD Length,
		IN		BOOL RestartScan,//__in       BOOL  ReturnSingleEntry,
		IN OUT	PDWORD Context,
		OUT		PDWORD ReturnLength OPTIONAL
	);

	NTSYSAPI
	EXPORTNUM(231)
	NTSTATUS
	NTAPI
	NtQueryFullAttributesFile(
		IN		POBJECT_ATTRIBUTES ObjectAttributes,
		OUT		PFILE_NETWORK_OPEN_INFORMATION Attributes
	);

	NTSYSAPI
	EXPORTNUM(232)
	NTSTATUS
	NTAPI
	NtQueryInformationFile(
		IN		HANDLE FileHandle,
		OUT		PIO_STATUS_BLOCK IoStatusBlock,
		OUT		PVOID FileInformation,
		IN		DWORD Length,
		IN		FILE_INFORMATION_CLASS FileInformationClass
	); 

	NTSYSAPI
	EXPORTNUM(236)
	NTSTATUS
	NTAPI
	NtQuerySymbolicLinkObject(
		IN		HANDLE LinkHandle,
		IN OUT	PSTRING LinkTarget,
		OUT		PULONG ReturnedLength OPTIONAL
	);

	NTSYSAPI
	EXPORTNUM(239)
	NTSTATUS
	NTAPI
	NtQueryVolumeInformationFile(
		IN 		HANDLE FileHandle,
		OUT 	PIO_STATUS_BLOCK IoStatusBlock,
		OUT 	PVOID FileSystemInformation,
		IN 		DWORD Length,
		IN 		FSINFOCLASS FileSystemInformationClass
	);

	//  UNTESTED
	NTSYSAPI
	EXPORTNUM(240)
	NTSTATUS
	NTAPI
	NtReadFile(
		IN		HANDLE FileHandle,
		IN		HANDLE Event OPTIONAL,
		IN		PIO_APC_ROUTINE ApcRoutine OPTIONAL,
		IN		PVOID ApcContext OPTIONAL,
		OUT		PIO_STATUS_BLOCK IoStatusBlock,
		OUT		PVOID Buffer,
		IN		DWORD Length,
		IN		PLARGE_INTEGER ByteOffset OPTIONAL
	);

	//  UNTESTED
	NTSYSAPI
	EXPORTNUM(241)
	NTSTATUS
	NTAPI
	NtReadFileScatter(
		IN 		HANDLE FileHandle,
		IN 		HANDLE Event OPTIONAL,
		IN 		PIO_APC_ROUTINE ApcRoutine OPTIONAL,
		IN 		PVOID ApcContext OPTIONAL,
		OUT 	PIO_STATUS_BLOCK IoStatusBlock,
		IN 		FILE_SEGMENT_ELEMENT SegmentArray,
		IN 		DWORD Length,
		IN 		PLARGE_INTEGER ByteOffset,
		IN 		PDWORD Key OPTIONAL
	); 

	NTSYSAPI
	EXPORTNUM(247)
	NTSTATUS
	NTAPI
	NtSetInformationFile(
		IN		HANDLE FileHandle,
		OUT		PIO_STATUS_BLOCK IoStatusBlock,
		IN		PVOID FileInformation,
		IN		DWORD Length,
		IN		FILE_INFORMATION_CLASS FileInformationClass
	);

	NTSYSAPI
	EXPORTNUM(249)
	NTSTATUS
	NTAPI
	NtSetSystemTime(
		IN		PFILETIME SystemTime,  // LARGE_INTEGER
		OUT		PFILETIME PreviousTime // LARGE_INTEGER
	);

	NTSYSAPI
	EXPORTNUM(255)
	NTSTATUS
	NTAPI
	NtWriteFile(
		IN		HANDLE FileHandle,
		IN		HANDLE Event OPTIONAL,
		IN		PIO_APC_ROUTINE ApcRoutine OPTIONAL,
		IN		PVOID ApcContext OPTIONAL,
		OUT		PIO_STATUS_BLOCK IoStatusBlock,
		IN		PVOID Buffer,
		IN		DWORD Length,
		IN		PLARGE_INTEGER ByteOffset OPTIONAL
	);

	NTSYSAPI
	EXPORTNUM(259)
	HRESULT
	NTAPI
	ObCreateSymbolicLink(
		IN		PSTRING SymbolicLinkName,
		IN		PSTRING DeviceName
	);

	NTSYSAPI
	EXPORTNUM(260)
	HRESULT
	NTAPI
	ObDeleteSymbolicLink(
		IN		PSTRING SymbolicLinkName
	);

	NTSYSAPI
	EXPORTNUM(261)
	VOID
	NTAPI
	ObDereferenceObject(
		IN		PVOID Object
	);

	NTSYSAPI
	EXPORTNUM(266)
	HRESULT
	NTAPI
	ObLookupAnyThreadByThreadId(
		IN		DWORD dwThreadId, 
		OUT		PKTHREAD *pthr
	);

	NTSYSAPI
	EXPORTNUM(271)
	VOID
	NTAPI
	ObReferenceObject(
		IN		PVOID Object
	);

	NTSYSAPI
	EXPORTNUM(283)
	int
	NTAPI
	RtlCompareMemoryUlong(
		IN   	void* Source,
		IN   	DWORD Length,
		IN   	DWORD Pattern
	);

	NTSYSAPI
	EXPORTNUM(298)
	VOID
	NTAPI
	RtlGetStackLimits(
		IN OUT	PDWORD LimitA,
		IN OUT	PDWORD LimitB
	);

	NTSYSAPI
	EXPORTNUM(299)
	PVOID
	NTAPI
	RtlImageXexHeaderField(
		IN		PVOID XexHeaderBase,
		IN		DWORD ImageField
	);

	NTSYSAPI
	EXPORTNUM(300)
	VOID
	NTAPI
	RtlInitAnsiString(
		IN OUT	PANSI_STRING DestinationString,
		IN		PCSZ  SourceString
	);

	NTSYSAPI
	EXPORTNUM(305)
	PVOID
	NTAPI
	RtlLookupFunctionEntry(
		IN		PVOID pvOffset
	);

	NTSYSAPI
	EXPORTNUM(305)
	DWORD
	NTAPI
	RtlSprintf(
		IN		CHAR* Buffer, 
		IN		const CHAR* Format, 
		...
		);

	EXPORTNUM(342)
	extern PXBOX_HARDWARE_INFO XboxHardwareInfo;

	EXPORTNUM(343)
	extern PXBOX_KRNL_VERSION XboxKrnlBaseVersion;

	EXPORTNUM(344)
	extern PXBOX_KRNL_VERSION XboxKrnlVersion;

	//dword pointer in kernel space to current title exe handle
	// PLDR_DATA_TABLE_ENTRY ent = (PLDR_DATA_TABLE_ENTRY)(*XexExecutableModuleHandle);
	//extern PHANDLE XexExecutableModuleHandle;
	EXPORTNUM(403)
	extern PLDR_DATA_TABLE_ENTRY* XexExecutableModuleHandle;

	NTSYSAPI
	EXPORTNUM(404)
	BOOL
	NTAPI
	XexCheckExecutablePrivilege(
		IN		DWORD priviledge
	);

	// ie XexGetModuleHandle("xam.xex", &hand), returns 0 on success
	NTSYSAPI
	EXPORTNUM(405)
	NTSTATUS
	NTAPI
	XexGetModuleHandle(
		IN		PSZ moduleName,
		IN OUT	PHANDLE hand
	); 

	// ie XexGetProcedureAddress(hand ,0x50, &addr) returns 0 on success
	NTSYSAPI
	EXPORTNUM(407)
	NTSTATUS
	NTAPI
	XexGetProcedureAddress(
		IN		HANDLE hand,
		IN		DWORD dwOrdinal,
		IN		PVOID pvAddress
	);

	NTSYSAPI
	EXPORTNUM(408)
	NTSTATUS
	NTAPI
	XexLoadExecutable(
		IN		PCHAR szXexName,
		IN OUT	PHANDLE pHandle,
		IN		DWORD dwModuleTypeFlags,
		IN		DWORD dwMinimumVersion
	);

	NTSYSAPI
	EXPORTNUM(409)
	NTSTATUS
	NTAPI
	XexLoadImage(
		IN		LPCSTR szXexName,
		IN		DWORD dwModuleTypeFlags,
		IN		DWORD dwMinimumVersion,
		IN OUT	PHANDLE pHandle
	);

	NTSYSAPI
	EXPORTNUM(410)
	NTSTATUS
	NTAPI
	XexLoadImageFromMemory(
		IN		PVOID pvXexBuffer,
		IN		DWORD dwSize,
		IN		LPCSTR szXexName,
		IN		DWORD dwModuleTypeFlags,
		IN		DWORD dwMinimumVersion,
		IN OUT	PHANDLE pHandle
	);

	NTSYSAPI
	EXPORTNUM(412)
	PVOID // returns pointer to NT header
	NTAPI
	XexPcToFileHeader(
		IN		PVOID address,
		OUT		PLDR_DATA_TABLE_ENTRY* ldatOut // puts pointer to LDR_DATA_TABLE_ENTRY in the address pointer here
	);

	NTSYSAPI
	EXPORTNUM(416)
	HRESULT
	NTAPI
	XexStartExecutable(
		IN		FARPROC TitleProcessInitThreadProc
	);

	NTSYSAPI
	EXPORTNUM(417)
	VOID
	NTAPI
	XexUnloadImage(
		IN		HANDLE moduleHandle
	);

	NTSYSAPI
	EXPORTNUM(418)
	VOID
	NTAPI
	XexUnloadImageAndExitThread(
		IN		HANDLE moduleHandle,
		IN		HANDLE threadHandle
	);

	NTSYSAPI
	EXPORTNUM(420)
	NTSTATUS
	NTAPI
	XexVerifyImageHeaders(
		IN		PIMAGE_XEX_HEADER XexHeader,
		IN		DWORD SizeOfHeaders
	);

	NTSYSAPI
	EXPORTNUM(424)
	PVOID
	NTAPI
	RtlImageDirectoryEntryToData(
		IN		PVOID Base,
		IN		BOOL MappedAsImage,
		IN		WORD DirectoryEntry,
		IN OUT	PDWORD size
	);

	EXPORTNUM(430)
	extern PCHAR ExLoadedCommandLine; // max size 0x200 ie: "default.xex"
	
	EXPORTNUM(431)
	extern PCHAR ExLoadedImageName; // max size 0x100 ie: "\Device\Mass0\xexloader_testing\default.xex"

	NTSYSAPI
	EXPORTNUM(434)
	VOID
	NTAPI
	VdDisplayFatalError(
		IN		DWORD dwErrorCode
	);

	NTSYSAPI
	EXPORTNUM(442)
	VOID
	NTAPI
	VdGetCurrentDisplayInformation(
		IN OUT	PDISPLAY_INFO dispInfo
	);

	EXPORTNUM(446)
	extern D3DDevice* VdGlobalDevice;

	EXPORTNUM(447)
	extern D3DDevice* VdGlobalXamDevice;

	NTSYSAPI
	EXPORTNUM(461)
	PVOID // seems to return RTL_CRITICAL_SECTION VdpNotificationRegistrationListLock
	NTAPI
	VdRegisterGraphicsNotification(
		IN		LP_VD_NOTIFICATION_CALLBACK VdpXamNotificationCallback,
		IN		BOOL unk // seems to be insert/delete toggle
	);

	// when notification is called from kernel, PVD_NOTIFICATION_REGISTRATION is always NULL
	NTSYSAPI
	EXPORTNUM(462)
	VOID
	NTAPI
	VdRegisterXamGraphicsNotification(
		IN		LP_VD_NOTIFICATION_CALLBACK VdpXamNotificationCallback
	);

	NTSYSAPI
	EXPORTNUM(486)
	NTSTATUS
	NTAPI
	XInputdReadState(
		IN		DWORD DeviceContext,
		OUT		PDWORD pdwPacketNumber,
		OUT		PXINPUT_GAMEPAD pInputData,
		IN OUT	PBOOL pbUnknown OPTIONAL
	);

	// this item is 100% TODO!!
	// NTSYSAPI
	// EXPORTNUM(487)
	// NTSTATUS
	// NTAPI
	// XInputdWriteState(
		// IN		DWORD DeviceContext,
		// OUT		PDWORD pdwPacketNumber,
		// OUT		PXINPUT_GAMEPAD pInputData,
		// IN OUT	PBOOL pbUnknown
	// );

	NTSYSAPI
	EXPORTNUM(569)
	BOOL
	NTAPI
	ExIsBetaFeatureEnabled(
		IN		DWORD dwBetaFeature
	);

	NTSYSAPI
	EXPORTNUM(570)
	VOID
	NTAPI
	XeKeysGetFactoryChallenge(
		IN		PVOID buf
	);

	NTSYSAPI
	EXPORTNUM(571)
	DWORD
	NTAPI
	XeKeysSetFactoryResponse(
		IN		PVOID buf
	);

	NTSYSAPI
	EXPORTNUM(575)
	DWORD
	NTAPI
	XeKeysGetStatus(
		IN OUT	PDWORD sta
	);

	NTSYSAPI
	EXPORTNUM(576)
	NTSTATUS
	NTAPI
	XeKeysGeneratePrivateKey(
		IN		WORD KeyId
	);
	
	// returns the size of the given key number for use with get/set
	NTSYSAPI
	EXPORTNUM(577)
	WORD
	NTAPI
	XeKeysGetKeyProperties(
		IN		DWORD KeyId
	);

	NTSYSAPI
	EXPORTNUM(578)
	NTSTATUS
	NTAPI
	XeKeysSetKey(
		IN		WORD KeyId,
		IN		PVOID KeyBuffer,
		IN		WORD keyLength
	);

	NTSYSAPI
	EXPORTNUM(579)
	NTSTATUS
	NTAPI
	XeKeysGenerateRandomKey(
		IN		WORD KeyId,
		OUT		PVOID KeyBuffer
	);

	NTSYSAPI
	EXPORTNUM(580)
	NTSTATUS
	NTAPI
	XeKeysGetKey(
		IN		WORD KeyId,
		OUT		PVOID KeyBuffer,
		IN OUT	PDWORD keyLength
	);

	NTSYSAPI
	EXPORTNUM(582)
	DWORD
	NTAPI
	XeKeysGetConsoleID(
		OUT		PBYTE databuffer OPTIONAL, // optional, will copy the console ID to this if it's sent
		OUT		char* szBuffer OPTIONAL // optional, will sprintf the data to this if it's sent
	);

	NTSYSAPI
	EXPORTNUM(583)
	NTSTATUS
	NTAPI
	XeKeysGetConsoleType(
		IN OUT	PDWORD ConsoleType
	);

	NTSYSAPI
	EXPORTNUM(585)
	BOOL
	NTAPI
	XeKeysHmacSha(
		IN		XEKEY_INDEX keySel,
		IN		const PBYTE pbInp1,
		IN		DWORD cbInp1,
		IN		const PBYTE pbInp2,
		IN		DWORD cbInp2,
		IN		const PBYTE pbInp3,
		IN		DWORD cbInp3,
		IN OUT	PBYTE pbOut,
		IN		DWORD cbOut
	);

	NTSYSAPI
	EXPORTNUM(596)
	BOOL
	NTAPI
	XeKeysObfuscate(
		IN		XEKEY_OBFUSCATE keySel,
		IN		const PBYTE pbInp1,
		IN		DWORD cbInp1,
		IN OUT	PBYTE pbOut,
		IN		PDWORD cbOut
	);

	NTSYSAPI
	EXPORTNUM(597)
	BOOL
	NTAPI
	XeKeysUnObfuscate(
		IN		XEKEY_OBFUSCATE keySel,
		IN		const PBYTE pbInp1,
		IN		DWORD cbInp1,
		IN OUT	PBYTE pbOut,
		IN		PDWORD cbOut
	);

	NTSYSAPI
	EXPORTNUM(599)
	BOOL
	NTAPI
	XeKeysConsoleSignatureVerification(
		IN		PBYTE pbHash, // 0x14 byte hash
		IN		XE_CONSOLE_SIGNATURE sig,
		IN OUT	PDWORD localCertCompareResult // result of comparing the cert to the console cert
	);

	NTSYSAPI
	EXPORTNUM(607)
	DWORD
	NTAPI
	XeKeysExecute(
		IN		PVOID pvPhyBuffer, // must be 0x80 byte aligned
		IN		DWORD len, // code block must be > 0x120 and no larger than 0x10000
		IN OUT	PVOID arg1, // args depend on the code block sent
		IN OUT	PVOID arg2,
		IN OUT	PVOID arg3,
		IN OUT	PVOID arg4
	);

	NTSYSAPI
	EXPORTNUM(609)
	VOID
	NTAPI
	XInputdPowerDownDevice(
		IN		DWORD flag // or the user index with 0x10000000
	);

	NTSYSAPI
	EXPORTNUM(611)
	VOID
	NTAPI
	AniTerminateAnimation(
		VOID
	);
	
	NTSYSAPI
	EXPORTNUM(620)
	VOID
	NTAPI
	HalGetPowerUpCause(
		OUT		PBYTE 	reply
	);

	NTSYSAPI
	EXPORTNUM(650)
	NTSTATUS
	NTAPI
	NtAllocateEncryptedMemory(
		IN   	DWORD memoryRegionType, // 0= auto/thread type, 1= title, 2=system, 3=debug, 4=??
		IN  	SIZE_T Size,
		OUT		LPVOID* pBuffer
	);	

	NTSYSAPI
	EXPORTNUM(690)
	NTSTATUS
	NTAPI
	XeKeysGetMediaID(
		IN OUT	PBYTE pMediaID, // 0x10 bytes
		IN		BOOL fCheckHvAuth // if true checks if dvd auth data is valid first?
	);

	EXPORTNUM(731)
	extern PVOID* UsbdBootEnumerationDoneEvent;

	NTSYSAPI
	EXPORTNUM(738)
	HRESULT
	NTAPI
	LDIResetDecompression(
		IN		HANDLE hmd
	);

	NTSYSAPI
	EXPORTNUM(793)
	NTSTATUS
	NTAPI
	HalReadArgonEeprom(
		IN		ARGON_EEPROM_FIELD field,
		IN OUT	PVOID pvBuffer,
		IN		DWORD dwCbbuf,
		OUT		PWORD pwBufUsed OPTIONAL
	);

	// NTSYSAPI
	// EXPORTNUM(113)
	// VOID
	// NTAPI
	// KeInitializeInterrupt(
		// OUT		PKINTERRUPT InterruptObject,
		// IN		PVOID ServiceRoutine,
		// IN		PVOID ServiceContext,
		// IN		BYTE Irql,
		// IN		BYTE InterruptMode,
		// IN		BYTE bProcTargetNumber // KAFFINITY ProcessorEnableMask
	// );

	// NTSYSAPI
	// EXPORTNUM(45)
	// NTSTATUS
	// NTAPI
	// KeConnectInterrupt(
		// IN		PKINTERRUPT InterruptObject
	// );

	// NTSYSAPI
	// EXPORTNUM(111)
	// VOID
	// NTAPI
	// KeInitializeDpc(
		// OUT		PKDPC Dpc,
		// IN		PVOID DeferredRoutine,
		// IN		PVOID DeferredContext OPTIONAL
	// );


	// NTSYSAPI
	// EXPORTNUM(208)
	// NTSTATUS
	// NTAPI
	// NtCreateDirectoryObject(
		// OUT		PHANDLE DirectoryHandle,
		// IN		POBJECT_ATTRIBUTES ObjectAttributes
	// );

	// NTSYSAPI
	// EXPORTNUM(272)
	// NTSTATUS
	// NTAPI
	// ObReferenceObjectByHandle(
		// IN		HANDLE Handle,
		// IN		POBJECT_TYPE ObjectType,
		// OUT		PVOID *Object
	// );

	// NTSYSAPI
	// EXPORTNUM(258)
	// NTSTATUS
	// NTAPI
	// ObCreateObject(
		// IN		POBJECT_TYPE ObjectType,
		// IN		POBJECT_ATTRIBUTES ObjectAttributes,
		// IN		DWORD ObjectBodySize,
		// OUT		PVOID *Object
	// );

	// NTSYSAPI
	// EXPORTNUM(264)
	// NTSTATUS
	// NTAPI
	// ObInsertObject(
		// IN		PVOID Object,
		// IN		POBJECT_ATTRIBUTES ObjectAttributes, // not sure, seems it's fed this from ObCreateObject
		// IN		DWORD AdditionalReferences, // not sure on this one, could be a bool
		// OUT		PHANDLE Handle
	// );

	// NTSYSAPI
	// EXPORTNUM(273)
	// NTSTATUS
	// NTAPI
	// ObReferenceObjectByName(
		// IN		PSTRING ObjectName,
		// IN		DWORD Attributes, // PACCESS_STATE AccessState, ACCESS_MASK DesiredAccess
		// IN		POBJECT_TYPE ObjectType,
		// IN		PVOID ParseContext OPTIONAL, // KPROCESSOR_MODE AccessMode
		// OUT		PVOID* Object
	// );
		 

#ifdef __cplusplus
}
#endif


#endif	//__KERNEL_DEFINES_H

