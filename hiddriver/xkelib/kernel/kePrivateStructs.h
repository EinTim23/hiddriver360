#ifndef __KEPRIVATESTRUCTS_H
#define __KEPRIVATESTRUCTS_H

/* **** FIXME ****
_OBJECT_HANDLE_TABLE
_KTHREAD
*/

/* HalReadArgonEeprom read on a falcon jtag at 14719
ArgonEepromErrorBitfield: ret 0x0 read 0x7	: 00 00 00 00 04 00 00
ArgonEepromV12P0		: ret 0x0 read 0x8	: BB 1B B1 13 00 4A 5D 01
ArgonEepromAll			: ret 0x0 read 0x10	: 0F 00 00 00 00 04 00 00 BB 1B B1 13 00 4A 5D 01 */
typedef enum {
	ArgonEepromErrorBitfield = 0x0, // buf size 0xC
	ArgonEepromV12P0 = 0x1, // buf size 8
	ArgonEepromAll = 0x2,
} ARGON_EEPROM_FIELD;

typedef enum {
	DriverQuiesceRundown = 0x0,
	DriverQuiesceStartup = 0x1,
	DriverShutdown = 0x2,
} _KDRIVER_NOTIFICATION_TYPE;

// for KeGetCurrentProcessType()
typedef enum  {
	PROC_IDLE = 0,
	PROC_USER = 1,
	PROC_SYSTEM = 2,
	PROC_DEBUG = 3,
} _KPROC_TYPE;

typedef _KPROC_TYPE		KPROCESSOR_MODE;

typedef enum {
	Executive = 0x0,
	DelayExecution = 0x1,
	Suspended = 0x2,
	UserRequest = 0x3,
	WrQueue = 0x4,
	WrFsCacheIn = 0x5,
	WrFsCacheOut = 0x6,
	WrReadWriteLock = 0x7,
	WrCriticalSection = 0x8,
	MaximumWaitReason = 0x9,
} KWAIT_REASON;

//NTSTATUS
//XxxDispatchRead(
//				IN PDEVICE_OBJECT  DeviceObject,
//				IN PIRP  Irp
//				);

// function location in array of u32's
typedef enum {
	DEVICEOB_Start = 0,
	DEVICEOB_Del,
	DEVICEOB_Dismount,
	DEVICEOB_FCreate,
	DEVICEOB_FClose,
	DEVICEOB_FRead,
	DEVICEOB_FWrite,
	DEVICEOB_FQuery,
	DEVICEOB_FSet,
	DEVICEOB_FFlush,
	DEVICEOB_FQueryVol,
	DEVICEOB_FDirCntrl,
	DEVICEOB_FDevCntrl,
	DEVICEOB_FCleanup,
} DEVICEOB_FUNS;

// INCOMPLETE!! DEVICE_TYPE
#define FILE_DEVICE_CD_ROM              0x00000002 // SataCdRomDriverObject
#define FILE_DEVICE_CD_ROM_FILE_SYSTEM  0x00000003 // OdfxDriverObject
#define FILE_DEVICE_DISK                0x00000007 // SataDiskDriverObject
#define FILE_DEVICE_DISK_FILE_SYSTEM    0x00000008 // nomnil FatwDriverObject FatxDriverObject RawxDriverObject HfspDriverObject
#define FILE_DEVICE_VIRTUAL_DISK        0x00000024
#define FILE_DEVICE_MASS_STORAGE        0x0000002d // MassIoDriverObject
#define FILE_DEVICE_MASS1				0x0000003a // MassIoDriverObject
#define FILE_DEVICE_MASS2				0x0000003e // MassIoDriverObject
#define FILE_DEVICE_SFCX				0x0000003c // SfcxDriverObject
#define FILE_DEVICE_UNK1				0x0000003d
#define FILE_DEVICE_UNK2				0x00000040
#define FILE_DEVICE_UNK3				0x00000041 // SfcxMuDriverObject
#define FILE_DEVICE_UNK4				0x00000042
#define FILE_DEVICE_UNK5				0x00000043
#define FILE_DEVICE_MASS3				0x00000044 // MassIoDriverObject
#define FILE_DEVICE_MASS4				0x00000045 // MassIoDriverObject
#define FILE_DEVICE_UNK6				0x00000046
#define FILE_DEVICE_DEV_AUTH			0x00000047 // DevAuthIoDriverObject

// for 32 bit version splitting/creating
#pragma pack(push, 1)
typedef union _XBOX32VER {
	struct {
	unsigned char Major : 4;
	unsigned char Minor : 4;
	unsigned short Build;
	unsigned char Qfe;
	};
	DWORD asULong;
} XBOX32VER, *PXBOX32VER;
#pragma pack(pop)

typedef ULONG_PTR KIPI_BROADCAST_WORKER (
	__in ULONG_PTR Argument
	);
typedef KIPI_BROADCAST_WORKER *PKIPI_BROADCAST_WORKER;

//typedef struct _DISPLAY_INFO{
//	WORD timing1; // 0x0
//	WORD timing2; // 0x2
//	BYTE colorspace; // 0x4
//	BYTE colorformat; // 0x5
//	BYTE padb_1[2]; // 0x6 pad
//	DWORD pitch; // 0x8
//	DWORD format; // 0xC
//	DWORD offsetx; // 0x10
//	DWORD offsety; // 0x14
//	DWORD sw; // 0x18
//	DWORD sh; // 0x1c
//	DWORD dwUnk1;
//	BYTE baUnk1[12];
//	DWORD dwUnk2;
//	BYTE baUnk2[12];
//	WORD waUnk1[6]; // last two may be important
//	DWORD dwAsFloat; // is a float?
//	DWORD dwUnk3;
//	BYTE padb_2[2]; //pad
//	WORD wUnk1;	
//} DISPLAY_INFO, *PDISPLAY_INFO; // total size 0x58 bytes
//C_ASSERT(sizeof(DISPLAY_INFO) == 88);
typedef struct _D3DPRIVATE_RECT {
	LONG x1, y1;
	LONG x2, y2;
} D3DPRIVATE_RECT, *PD3DPRIVATE_RECT;

typedef struct _D3DFILTER_PARAMETERS {
	FLOAT Nyquist;
	FLOAT FlickerFilter;
	FLOAT Beta;
} D3DFILTER_PARAMETERS, *PD3DFILTER_PARAMETERS;

typedef struct _D3DPRIVATE_SCALER_PARAMETERS {
	D3DPRIVATE_RECT		 ScalerSourceRect;
	LONG				 ScaledOutputWidth;
	LONG				 ScaledOutputHeight;
	DWORD				 VirticalFilterType;
	D3DFILTER_PARAMETERS VerticalFilterParameters;
	DWORD				 HorizontalFilterType;
	D3DFILTER_PARAMETERS HorizontalFilterParameters;
} D3DPRIVATE_SCALER_PARAMETERS, *PD3DPRIVATE_SCALER_PARAMETERS;

typedef struct _DISPLAY_INFORMATION {
	WORD						 FrontBufferWidth;
	WORD						 FrontBufferHeight;
	BYTE						 FrontBufferColorFormat;
	BYTE						 FrontBufferPixelFormat;
	D3DPRIVATE_SCALER_PARAMETERS ScalerParameters;
	WORD						 DisplayWindowOverscanLeft;
	WORD						 DisplayWindowOverscanTop;
	WORD						 DisplayWindowOverscanRight;
	WORD						 DisplayWindowOverscanBottom;
	WORD						 DisplayWidth;
	WORD						 DisplayHeight;
	FLOAT						 DisplayRefreshRate;
	DWORD						 DisplayInterlaced;
	BYTE						 DisplayColorFormat;
	WORD						 ActualDisplayWidth;
} DISPLAY_INFO, *PDISPLAY_INFO;
C_ASSERT(sizeof(DISPLAY_INFO) == 88);

// *********** PACKING STARTS HERE **************//
#pragma pack(push, 1)
// *********** PACKING STARTS HERE **************//

typedef struct _MM_STATISTICS{
	DWORD Length;
	DWORD TotalPhysicalPages;
	DWORD KernelPages;
	DWORD TitleAvailablePages;
	DWORD TitleTotalVirtualMemoryBytes;
	DWORD TitleReservedVirtualMemoryBytes;
	DWORD TitlePhysicalPages;
	DWORD TitlePoolPages;
	DWORD TitleStackPages;
	DWORD TitleImagePages;
	DWORD TitleHeapPages;
	DWORD TitleVirtualPages;
	DWORD TitlePageTablePages;
	DWORD TitleCachePages;
	DWORD SystemAvailablePages;
	DWORD SystemTotalVirtualMemoryBytes;
	DWORD SystemReservedVirtualMemoryBytes;
	DWORD SystemPhysicalPages;
	DWORD SystemPoolPages;
	DWORD SystemStackPages;
	DWORD SystemImagePages;
	DWORD SystemHeapPages;
	DWORD SystemVirtualPages;
	DWORD SystemPageTablePages;
	DWORD SystemCachePages;
	DWORD HighestPhysicalPage;
} MM_STATISTICS, *PMMSTATISTICS; // 104
C_ASSERT(sizeof(MM_STATISTICS) == 104);

typedef struct _KINTERRUPT { 
	PVOID ServiceRoutine; // VOID(*)() 0x0 sz:0x4
	PVOID ServiceContext; // 0x4 sz:0x4
	DWORD SpinLock; // 0x8 sz:0x4
	DWORD ServiceCount; // 0xC sz:0x4
	BYTE BusIrql; // 0x10 sz:0x1
	BYTE Irql; // 0x11 sz:0x1
//	BYTE Connected : 1; // 0x12 bfo:0x7
//	BYTE Mode : 7; // 0x12 bfo:0x0
	BYTE ModeConnected;
	BYTE TargetNumber; // 0x13 sz:0x1
} KINTERRUPT, *PKINTERRUPT; // size 20
C_ASSERT(sizeof(KINTERRUPT) == 0x14);

typedef struct _KTRAP_FRAME { 
	STACK_FRAME_HEADER Header; // 0x0 sz:0x50
	QWORD Gpr0; // 0x50 sz:0x8
	QWORD Gpr1; // 0x58 sz:0x8
	QWORD Gpr2; // 0x60 sz:0x8
	QWORD Gpr3; // 0x68 sz:0x8
	QWORD Gpr4; // 0x70 sz:0x8
	QWORD Gpr5; // 0x78 sz:0x8
	QWORD Gpr6; // 0x80 sz:0x8
	QWORD Gpr7; // 0x88 sz:0x8
	QWORD Gpr8; // 0x90 sz:0x8
	QWORD Gpr9; // 0x98 sz:0x8
	QWORD Gpr10; // 0xA0 sz:0x8
	QWORD Gpr11; // 0xA8 sz:0x8
	QWORD Gpr12; // 0xB0 sz:0x8
	QWORD Gpr13; // 0xB8 sz:0x8
	QWORD Gpr14; // 0xC0 sz:0x8
	QWORD Gpr15; // 0xC8 sz:0x8
	QWORD Gpr16; // 0xD0 sz:0x8
	QWORD Gpr17; // 0xD8 sz:0x8
	QWORD Gpr18; // 0xE0 sz:0x8
	QWORD Gpr19; // 0xE8 sz:0x8
	QWORD Gpr20; // 0xF0 sz:0x8
	QWORD Gpr21; // 0xF8 sz:0x8
	QWORD Gpr22; // 0x100 sz:0x8
	QWORD Gpr23; // 0x108 sz:0x8
	QWORD Gpr24; // 0x110 sz:0x8
	QWORD Gpr25; // 0x118 sz:0x8
	QWORD Gpr26; // 0x120 sz:0x8
	QWORD Gpr27; // 0x128 sz:0x8
	QWORD Gpr28; // 0x130 sz:0x8
	QWORD Gpr29; // 0x138 sz:0x8
	QWORD Gpr30; // 0x140 sz:0x8
	QWORD Gpr31; // 0x148 sz:0x8
	union{
		struct{
			BYTE OldIrql;
			PKINTERRUPT InterruptObject;
			BYTE InterruptIrql;
		} asIrq;
		BYTE ExceptionRecord[0x50];
	} irqExc;
	QWORD Ctr; // 0x1A0 sz:0x8
	DWORD Lr; // 0x1A8 sz:0x4
	DWORD Iar; // 0x1AC sz:0x4
	DWORD Cr; // 0x1B0 sz:0x4
	DWORD Msr; // 0x1B4 sz:0x4
	DWORD Xer; // 0x1B8 sz:0x4
	DWORD UserModeControl; // 0x1BC sz:0x4
} KTRAP_FRAME, *PKTRAP_FRAME; // size 448
C_ASSERT(sizeof(KTRAP_FRAME) == 0x1C0);

typedef struct _KAPC{
	short Type;
	BYTE ApcMode;
	BYTE Inserted;
	struct _KTHREAD * Thread;
	LIST_ENTRY ApcListEntry;
	PVOID KernelRoutine; // function ptr
	PVOID RundownRoutine; // function ptr
	PVOID NormalRoutine; // function ptr
	PVOID NormalContext;
	PVOID SystemArgument1;
	PVOID SystemArgument2;
} KAPC, *PKAPC; // 40
C_ASSERT(sizeof(KAPC) == 40);

typedef struct _DISPATCHER_HEADER{
	BYTE Type;
	BYTE Absolute;
	BYTE ProcessType;
	BYTE Inserted;
	LONG SignalState;
	LIST_ENTRY WaitListHead;
} DISPATCHER_HEADER, *PDISPATCHER_HEADER; // 16
C_ASSERT(sizeof(DISPATCHER_HEADER) == 16);

typedef struct _KEVENT{
	DISPATCHER_HEADER Header;
} KEVENT, *PKEVENT; // 16
C_ASSERT(sizeof(KEVENT) == 16);

typedef struct _KDPC{
	short Type;
	BYTE InsertedNumber;
	BYTE TargetNumber;
	LIST_ENTRY DpcListEntry;
	PVOID DeferredRoutine; // function *
	PVOID DeferredContext;
	PVOID SystemArgument1;
	PVOID SystemArgument2;
} KDPC, *PKDPC; // 28
C_ASSERT(sizeof(KDPC) == 28);

typedef struct _KWAIT_BLOCK{
	LIST_ENTRY WaitListEntry;
	struct _KTHREAD* Thread;
	PVOID Object;
	struct _KWAIT_BLOCK * NextWaitBlock;
	WORD WaitKey;
	WORD WaitType;
} KWAIT_BLOCK, *PKWAIT_BLOCK; // 24
C_ASSERT(sizeof(KWAIT_BLOCK) == 24);


typedef struct _KTIMER{
	DISPATCHER_HEADER Header;
	ULARGE_INTEGER DueTime;
	LIST_ENTRY TimerListEntry;
	PKDPC Dpc;
	LONG Period;
} KTIMER, *PKTIMER; // 40
C_ASSERT(sizeof(KTIMER) == 40);

typedef struct _KPROCESS{
	DWORD ThreadListLock;
	LIST_ENTRY ThreadListHead;
	LONG ThreadQuantum;
	DWORD DirectoryTableBase;
	DWORD ThreadCount;
	BYTE IdlePriorityClass;
	BYTE NormalPriorityClass;
	BYTE TimeCriticalPriorityClass;
	BYTE DisableQuantum;
	DWORD DefaultKernelStackSize;
	PVOID TlsStaticDataImage;
	DWORD SizeOfTlsStaticData;
	DWORD SizeOfTlsStaticDataImage;
	WORD SizeOfTlsSlots;
	BYTE Terminating;
	BYTE ProcessType;
	DWORD TlsSlotBitmap[8];
	DWORD FileObjectListLock;
	LIST_ENTRY FileObjectListHead;
	PVOID Win32DefaultHeapHandle;
} KPROCESS, *PKPROCESS; // 96
C_ASSERT(sizeof(KPROCESS) == 96);

typedef struct _KSEMAPHORE{
	DISPATCHER_HEADER Header;
	LONG Limit;
} KSEMAPHORE, *PKSEMAPHORE; // 20
C_ASSERT(sizeof(KSEMAPHORE) == 20);

typedef struct _KQUEUE{
	DISPATCHER_HEADER Header;
	LIST_ENTRY EntryListHead;
	DWORD CurrentCount;
	DWORD MaximumCount;
	LIST_ENTRY ThreadListHead;
} KQUEUE, *PKQUEUE; // 40
C_ASSERT(sizeof(KQUEUE) == 40);

// *********** PACKING ENDS HERE **************//
#pragma pack(pop)
// *********** PACKING ENDS HERE **************//

typedef struct _KPRCB { 
	struct _KTHREAD* CurrentThread; // 0x0 sz:0x4
	struct _KTHREAD* NextThread; // 0x4 sz:0x4
	struct _KTHREAD* IdleThread; // 0x8 sz:0x4
	BYTE Number; // 0xC sz:0x1
	DWORD SetMember; // 0x10 sz:0x4
	DWORD DpcTime; // 0x14 sz:0x4
	DWORD InterruptTime; // 0x18 sz:0x4
	DWORD InterruptCount; // 0x1C sz:0x4
	volatile DWORD IpiFrozen; // 0x20 sz:0x4
	volatile DWORD CurrentPacket[3]; // 0x24 sz:0xC
	DWORD TargetSet; // 0x30 sz:0x4
	PVOID WorkerRoutine; // 0x34 sz:0x4 Function Pointer
	struct _KPRCB* SignalDone; // 0x38 sz:0x4
	DWORD RequestSummary; // 0x3C sz:0x4
	DWORD DpcInterruptRequested; // 0x40 sz:0x4
	DWORD DpcLock; // 0x44 sz:0x4
	LIST_ENTRY DpcListHead; // 0x48 sz:0x8
	DWORD DpcRoutineActive; // 0x50 sz:0x4
	DWORD ReadyListLock; // 0x54 sz:0x4
	DWORD IdleThreadActive; // 0x58 sz:0x4
	SINGLE_LIST_ENTRY DeferredReadyListHead; // 0x5C sz:0x4
	DWORD ReadySummary; // 0x60 sz:0x4
	DWORD ReadySummaryMask; // 0x64 sz:0x4
	LIST_ENTRY DispatcherReadyListHead[32]; // 0x68 sz:0x100
	KDPC ThreadReaperDpc; // 0x168 sz:0x1C
	LIST_ENTRY ThreadReaperListHead; // 0x184 sz:0x8
	KDPC SwitchProcessorThreadDpc; // 0x18C sz:0x1C
} KPRCB, *PKPRCB; // size 424
C_ASSERT(sizeof(KPRCB) == 0x1A8);

typedef struct _KTHREAD { 
	DISPATCHER_HEADER Header; // 0x0 sz:0x10
	LIST_ENTRY MutantListHead; // 0x10 sz:0x8
	KTIMER Timer; // 0x18 sz:0x28
	KWAIT_BLOCK TimerWaitBlock; // 0x40 sz:0x18
	DWORD KernelTime; // 0x58 sz:0x4
	PVOID StackBase; // 0x5C sz:0x4
	PVOID StackLimit; // 0x60 sz:0x4
	PVOID KernelStack; // 0x64 sz:0x4
	PVOID TlsData; // 0x68 sz:0x4
	BYTE State; // 0x6C sz:0x1
	BYTE Alerted[2]; // 0x6D sz:0x2
	BYTE Alertable; // 0x6F sz:0x1
	BYTE Priority; // 0x70 sz:0x1
	BYTE FpuExceptionEnable; // 0x71 sz:0x1
	BYTE CreateProcessType; // 0x72 sz:0x1
	BYTE CurrentProcessType; // 0x73 sz:0x1
	LIST_ENTRY ApcListHead[2]; // 0x74 sz:0x10
	PKPROCESS Process; // 0x84 sz:0x4
	BYTE KernelApcInProgress; // 0x88 sz:0x1
	BYTE KernelApcPending; // 0x89 sz:0x1
	BYTE UserApcPending; // 0x8A sz:0x1
	BYTE ApcQueueable; // 0x8B sz:0x1
	DWORD ApcQueueLock; // 0x8C sz:0x4
	DWORD ContextSwitches; // 0x90 sz:0x4
	LIST_ENTRY ReadyListEntry; // 0x94 sz:0x8
	union {
		struct {
			WORD MsrEnableMaskHigh; // 0x9C sz:0x2
			WORD MsrEnableMaskLow; // 0x9E sz:0x2
		} hilo;
		DWORD MsrEnableMask; // 0x9C sz:0x4
	} Msr;
	LONG WaitStatus; // 0xA0 sz:0x4
	BYTE WaitIrql; // 0xA4 sz:0x1
	BYTE WaitMode; // 0xA5 sz:0x1
	BYTE WaitNext; // 0xA6 sz:0x1
	BYTE WaitReason; // 0xA7 sz:0x1
	PKWAIT_BLOCK WaitBlockList; // 0xA8 sz:0x4
	DWORD Padding1; // 0xAC sz:0x4
	DWORD KernelApcDisable; // 0xB0 sz:0x4
	LONG Quantum; // 0xB4 sz:0x4
	char Saturation; // 0xB8 sz:0x1
	BYTE BasePriority; // 0xB9 sz:0x1
	BYTE PriorityDecrement; // 0xBA sz:0x1
	BYTE DisableBoost; // 0xBB sz:0x1
	char SuspendCount; // 0xBC sz:0x1
	BYTE Preempted; // 0xBD sz:0x1
	BYTE HasTerminated; // 0xBE sz:0x1
	BYTE CurrentProcessor; // 0xBF sz:0x1
	PKPRCB CurrentPrcb; // 0xC0 sz:0x4
	PKPRCB AffinityPrcb; // 0xC4 sz:0x4
	BYTE IdlePriorityClass; // 0xC8 sz:0x1
	BYTE NormalPriorityClass; // 0xC9 sz:0x1
	BYTE TimeCriticalPriorityClass; // 0xCA sz:0x1
	BYTE HasAsyncTerminated; // 0xCB sz:0x1
	DWORD ActiveTimerListLock; // 0xCC sz:0x4
	PVOID StackAllocatedBase; // 0xD0 sz:0x4
	KAPC SuspendApc; // 0xD4 sz:0x28
	KSEMAPHORE SuspendSemaphore; // 0xFC sz:0x14
	LIST_ENTRY ThreadListEntry; // 0x110 sz:0x8
	PKQUEUE Queue; // 0x118 sz:0x4
	LIST_ENTRY QueueListEntry; // 0x11C sz:0x8
	PVOID UserModeDispatcher; // 0x124 sz:0x4  Function Pointer
	PKTRAP_FRAME UserModeTrapFrame; // 0x128 sz:0x4
	LARGE_INTEGER CreateTime; // 0x130 sz:0x8
	LARGE_INTEGER ExitTime; // 0x138 sz:0x8
	LONG ExitStatus; // 0x140 sz:0x4
	LIST_ENTRY ActiveTimerListHead; // 0x144 sz:0x8
	PVOID ThreadId; // 0x14C sz:0x4
	PVOID StartAddress; // 0x150 sz:0x4 Function Pointer
	struct _LIST_ENTRY IrpList; // 0x154 sz:0x8
	PVOID DebugMonitorData; // 0x15C sz:0x4
	DWORD LastWin32ErrorCode; // 0x160 sz:0x4
	PVOID Win32CurrentFiber; // 0x164 sz:0x4
	DWORD Padding2; // 0x168 sz:0x4
	DWORD CreateOptions; // 0x16C sz:0x4
	float Vscr[4]; // 0x170 sz:0x10
	float Vr[4][128]; // 0x180 sz:0x800
	double Fpscr; // 0x980 sz:0x8
	double Fpr[32]; // 0x988 sz:0x100
	KAPC TerminateApc; // 0xA88 sz:0x28
} KTHREAD, *PKTHREAD; // size 2736

// *********** PACKING STARTS HERE **************//
#pragma pack(push, 1)
// *********** PACKING STARTS HERE **************//

typedef struct _KDRIVER_NOTIFICATION_REGISTRATION { 
	PVOID NotificationRoutine; // 0x0 sz:0x4
	long Priority; // 0x4 sz:0x4
	LIST_ENTRY ListEntry; // 0x8 sz:0x8
} KDRIVER_NOTIFICATION_REGISTRATION, *PKDRIVER_NOTIFICATION_REGISTRATION; // size 16
C_ASSERT(sizeof(KDRIVER_NOTIFICATION_REGISTRATION) == 0x10);

//C_ASSERT(sizeof(KTHREAD) == 0xAB0);

//KPCR is %r13 (pointer)
//PKPCR __declspec(naked) GetThread_KPCR(VOID)
//{
//	asm{
//		mr		r3, r13
//		blr
//	}
//}

typedef struct _KPCR { 
	PVOID TlsData; // 0x0 sz:0x4
	union {
		struct {
			WORD MsrEnableMaskHigh; // 0x4 sz:0x2
			WORD MsrEnableMaskLow; // 0x6 sz:0x2
		} hilo;
		DWORD MsrEnableMask; // 0x4 sz:0x4
	} Msr;
	union {
		struct {
			BYTE DispatchInterrupt; // 0x8 sz:0x1
			BYTE ApcInterrupt; // 0x9 sz:0x1
		} hard;
		WORD SoftwareInterrupt; // 0x8 sz:0x2
	} Int;
	union {
		struct {
			BYTE DpcFpuStateSaved; // 0xA sz:0x1
			BYTE DpcVpuStateSaved; // 0xB sz:0x1
		} sep;
		WORD DpcFpuVpuStateSaved; // 0xA sz:0x2
	} Dpc;
	BYTE DpcCurrentProcessType; // 0xC sz:0x1
	BYTE QuantumEnd; // 0xD sz:0x1
	BYTE TimerRequest; // 0xE sz:0x1
	BYTE HvCr0Save; // 0xF sz:0x1
	PKTHREAD FpuOwnerThread; // 0x10 sz:0x4
	PKTHREAD VpuOwnerThread; // 0x14 sz:0x4
	BYTE CurrentIrql; // 0x18 sz:0x1
	BYTE BackgroundSchedulingActive; // 0x19 sz:0x1
	union {
		struct {
			BYTE StartBackgroundScheduling; // 0x1A sz:0x1
			BYTE StopBackgroundScheduling; // 0x1B sz:0x1
		} sep;
		WORD StartStopBackgroundScheduling; // 0x1A sz:0x2
	} Sched;
	DWORD TimerHand; // 0x1C sz:0x4
	union{
		struct{
			QWORD LrIarSave; // 0x20 sz:0x8
			QWORD CrMsrSave; // 0x28 sz:0x8
			QWORD Gpr13Save; // 0x30 sz:0x8
		} gpSave;
		struct{
			DWORD Gpr1Restore; // 0x20 sz:0x4
			DWORD IarRestore; // 0x24 sz:0x4
			DWORD CrRestore; // 0x28 sz:0x4
			DWORD MsrRestore; // 0x2C sz:0x4
		} gpRest;
	};
	QWORD HvGpr1Save; // 0x38 sz:0x8
	QWORD HvGpr3Save; // 0x40 sz:0x8
	QWORD HvGpr4Save; // 0x48 sz:0x8
	QWORD HvGpr5Save; // 0x50 sz:0x8
	DWORD UserModeControl; // 0x58 sz:0x4
	PVOID PanicStack; // 0x5C sz:0x4
	DWORD DarSave; // 0x60 sz:0x4
	DWORD DsisrSave; // 0x64 sz:0x4
	PVOID DbgLastDpcRoutine; // 0x68 sz:0x4
	DWORD OnInterruptStack; // 0x6C sz:0x4
	PVOID StackBase; // 0x70 sz:0x4
	PVOID StackLimit; // 0x74 sz:0x4
	PVOID InterruptStackBase; // 0x78 sz:0x4
	PVOID InterruptStackLimit; // 0x7C sz:0x4
	PVOID InterruptRoutine[0x20]; // 0x80 sz:0x80
	KPRCB PrcbData; // 0x100 sz:0x1A8
	PKPRCB Prcb; // 0x2A8 sz:0x4
	DWORD Unused; // 0x2AC sz:0x4
	PDWORD PixCurrent; // 0x2B0 sz:0x4
	PDWORD PixLimit; // 0x2B4 sz:0x4
	PDWORD ProfilerCurrent; // 0x2B8 sz:0x4
	PDWORD ProfilerLimit; // 0x2BC sz:0x4
	DWORD ProfilerFlags; // 0x2C0 sz:0x4
	QWORD Contention; // 0x2C8 sz:0x8
	PDWORD MonitorProfileData; // 0x2D0 sz:0x4
} KPCR, *PKPCR; // size 728
//C_ASSERT(sizeof(KPCR) == 0x2D8);

typedef union _KPCR_PAGE {
	struct{
		KPCR Pcr; // 0x0 sz:0x2D8
		QWORD IdleThreadData[ANYSIZE_ARRAY]; // 0x300 sz:0x0
	} AsSeparate;
	BYTE PcrAsUCHARs[4096]; // 0x0 sz:0x1000
} KPCR_PAGE, *PKPCR_PAGE; // size 4096
C_ASSERT(sizeof(KPCR_PAGE) == 0x1000);

typedef struct _KDEVICE_QUEUE{
	short Type;
	BYTE Padding;
	BYTE Busy;
	DWORD Lock;
	LIST_ENTRY DeviceListHead;
} KDEVICE_QUEUE, *PKDEVICE_QUEUE; // 16
C_ASSERT(sizeof(KDEVICE_QUEUE) == 16);

typedef struct _DRIVER_OBJECT{
	void  * DriverStartIo; // function pointers
	void  * DriverDeleteDevice;
	void  * DriverDismountVolume;
	void  * MajorFunction[11];
} DRIVER_OBJECT, *PDRIVER_OBJECT; //56
C_ASSERT(sizeof(DRIVER_OBJECT) == 56);

typedef struct _KDEVICE_QUEUE_ENTRY{
	LIST_ENTRY DeviceListEntry;
	DWORD SortKey;
	BYTE Inserted;
} KDEVICE_QUEUE_ENTRY, *PKDEVICE_QUEUE_ENTRY;

typedef struct _IO_COMPLETION_CONTEXT{
	void * Port;
	void * Key;
} IO_COMPLETION_CONTEXT, *PIO_COMPLETION_CONTEXT;
C_ASSERT(sizeof(IO_COMPLETION_CONTEXT) == 8);

typedef struct _DEVICE_OBJECT{
	short Type; // 0
	USHORT Size; // 2
	long ReferenceCount; // 4
	PDRIVER_OBJECT DriverObject; // 8
	struct _DEVICE_OBJECT * MountedOrSelfDevice; // 12
	void * CurrentIrp; // 16
	DWORD Flags; // 20
	void * DeviceExtension; // 24 
	BYTE DeviceType; // 28
	BYTE StartIoFlags; // 29
	char StackSize; // 30
	BYTE DeletePending; // 31
	DWORD SectorSize; // 32
	DWORD AlignmentRequirement; // 36
	KDEVICE_QUEUE DeviceQueue; // 40
	KEVENT DeviceLock; // 56
	DWORD StartIoCount; // 72
	DWORD StartIoKey; // 76
} DEVICE_OBJECT, *PDEVICE_OBJECT;// 80
C_ASSERT(sizeof(DEVICE_OBJECT) == 80);

typedef struct _FILE_OBJECT{
	short Type;
	BYTE Flags;
	BYTE Flags2;
	PDEVICE_OBJECT DeviceObject;
	void * FsContext;
	void * FsContext2;
	long FinalStatus;
	LARGE_INTEGER CurrentByteOffset;
	struct _FILE_OBJECT* RelatedFileObject;
	PIO_COMPLETION_CONTEXT CompletionContext;
	long LockCount;
	KEVENT Lock;
	KEVENT Event;
	LIST_ENTRY ProcessListEntry;
	LIST_ENTRY FileSystemListEntry;
	BYTE IoPriority;
	BYTE PoolPadding [15];
} FILE_OBJECT, *PFILE_OBJECT;

typedef struct _IO_STACK_LOCATION{
	BYTE MajorFunction; // 0
	BYTE MinorFunction; // 1
	BYTE Flags; // 2
	BYTE Control; // 3
	union{
		struct {
			DWORD DesiredAccess; // 4
			DWORD Options; // 8
			USHORT FileAttributes; // 12
			USHORT ShareAccess; // 14
			PSTRING RemainingName; // 16
		} Create;
		struct{
			DWORD Length;  // 4
			union{
				DWORD BufferOffset;
				void* CacheBuffer;
			};
			LARGE_INTEGER ByteOffset;
		} Read;
		struct{
			DWORD Length;  // 4
			union{
				DWORD BufferOffset;
				void* CacheBuffer;
			};
			LARGE_INTEGER ByteOffset;
		} Write;
		struct{
			DWORD Length;  // 4
			PSTRING FileName;
		} QueryDirectory;
		struct{
			DWORD Length; // 4
			FILE_INFORMATION_CLASS FileInformationClass;
		} QueryFile;
		struct{
			DWORD Length; // 4
			FILE_INFORMATION_CLASS FileInformationClass;
			PFILE_OBJECT FileObject;
		} SetFile;
		struct{
			DWORD Length; // 4
			FSINFOCLASS FsInformationClass;
		} QueryVolume;
		struct{
			DWORD Length; // 4
			FSINFOCLASS FsInformationClass;
		} SetVolume;
		struct{
			DWORD OutputBufferLength; // 4
			void* InputBuffer; // 8
			DWORD InputBufferLength; // 12
			DWORD IoControlCode; //16
		} DeviceIoControl;
		struct{
			DWORD Length; // 4
			PBYTE Buffer;
			DWORD SectorNumber;
			DWORD BufferOffset;
		} SectorIo;
		struct{
			void* Argument1; // 4
			void* Argument2;
			void* Argument3;
			void* Argument4;
		} Others;
	} parameters; // parameters 16
	PDEVICE_OBJECT DeviceObject; // 20
	PFILE_OBJECT FileObject;  // 24
	void* CompletionRoutine; // 28 function long(*)()
	void* Context; // 32
} IO_STACK_LOCATION, *PIO_STACK_LOCATION; // 36
C_ASSERT(sizeof(IO_STACK_LOCATION) == 36);


// *********** PACKING ENDS HERE **************//
#pragma pack(pop)
// *********** PACKING ENDS HERE **************//

typedef struct _OBJECT_HANDLE_TABLE { 
	LONG HandleCount; // 0x0 sz:0x4
	DWORD FirstFreeTableOffset; // 0x4 sz:0x4
	DWORD TableAllocation; // 0x8 sz:0x4
	PVOID** RootTable; // 0xC sz:0x4
	PVOID* BuiltinRootTable[8]; // 0x10 sz:0x20
	DWORD SpinLock; // 0x30 sz:0x4
	BYTE PoolType; // 0x34 sz:0x1
	BYTE Discriminant; // 0x35 sz:0x1
	BYTE Terminating; // 0x36 sz:0x1
} OBJECT_HANDLE_TABLE, *POBJECT_HANDLE_TABLE; // size 56
C_ASSERT(sizeof(OBJECT_HANDLE_TABLE) == 0x38);

typedef struct _IRP{
	SHORT Type; // 0
	USHORT Size; // 2
	DWORD Flags; // 4
	LIST_ENTRY ThreadListEntry; // 8
	IO_STATUS_BLOCK IoStatus; // 16
	char StackCount; // 24
	char CurrentLocation; // 25
	BYTE PendingReturned; // 26
	BYTE Cancel; // 27
	void * UserBuffer; // 28
	PIO_STATUS_BLOCK UserIosb; // 32
	PKEVENT UserEvent; // 36
	union{ // off 40
		struct{
			void * UserApcRoutine; // 40 function void(*)()
			void * UserApcContext; // 44
		} AsynchronousParameters;
		LARGE_INTEGER AllocationSize; // 40
	} Overlay;
	union { // off 48
		struct {
			union{
				KDEVICE_QUEUE_ENTRY DeviceQueueEntry; // 48
				LIST_ENTRY DeviceListEntry; // 48
				void * DriverContext[4]; // 48
			};
			DWORD LockedBufferLength; // 64
			PKTHREAD Thread; // 68
			LIST_ENTRY ListEntry; // 72
			union{
				PIO_STACK_LOCATION CurrentStackLocation; // 80
				DWORD PacketType; // 80
			};
			PFILE_OBJECT OriginalFileObject; // 84
		} Overlay;
		KAPC Apc; // 48
		void * CompletionKey; // 48
	} Tail;
	void * CancelRoutine; // 88 function void(*)()
} IRP, *PIRP; // 96 - actually 92? appears the LARGE_INTEGER isn't packed here
C_ASSERT(sizeof(IRP) == 96);


#endif // __KEPRIVATESTRUCTS_H
