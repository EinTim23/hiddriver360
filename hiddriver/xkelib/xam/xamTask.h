#ifndef __XAMTASK_H
#define __XAMTASK_H

typedef HRESULT (WINAPI *PXAMTASKPROC)(IN PVOID pvParam);

typedef PVOID                           HXAMTASK, *PHXAMTASK;
typedef PVOID                           HXAMTASKQUEUE, *PHXAMTASKQUEUE;
typedef PVOID                           HXAMTASKOBJ, *PHXAMTASKOBJ; // Generic handle, either XAMTASK or XAMTASKQUEUE

// Type of task
#define XAMPROPERTY_TYPE_WAIT           0x00000001      // waits on a kernel handle and calls taskproc when signaled
#define XAMPROPERTY_TYPE_DEDICATED      0x00000002      // dedicated thread, will do work and exit
#define XAMPROPERTY_TYPE_POOLED         0x00000004      // reuse some kind of thread pool
#define XAMPROPERTY_TYPE_PERIODIC       0x00000008      // reuse some kind of thread pool, run at regular intervals
#define XAMPROPERTY_TYPE_UI             0x00000010
#define _XAMPROPERTY_TYPE_MASK_         0x0000001F

// When this task wakes up and does processing
#define XAMPROPERTY_WAKEUP_IO           0x00000100      // burst of CPU usage but mostly waiting on I/O
#define XAMPROPERTY_WAKEUP_NETWORK      0x00000200      // burst of CPU usage but mostly waiting on N/W
#define XAMPROPERTY_WAKEUP_OFTENWAIT    0x00000400      // calling WaitFor*Object* often
#define _XAMPROPERTY_WAKEUP_MASK_       0x00000700

// Task CPU usage
#define XAMPROPERTY_CPUUSAGE_LO         0x00001000      // e.g.: waiting and processing notifications
#define XAMPROPERTY_CPUUSAGE_HI         0x00002000      // e.g.: CODEC, encrypting, animations?
#define _XAMPROPERTY_CPUUSAGE_MASK_     0x00003000

// Duration
#define XAMPROPERTY_DURATION_VERYSHORT  0x00010000
#define XAMPROPERTY_DURATION_SHORT      0x00020000
#define XAMPROPERTY_DURATION_LONG       0x00040000
//                                      0x00080000      // Taken below
#define _XAMPROPERTY_DURATION_MASK_     0x00070000

// Priority
#define XAMPROPERTY_PRI_LO              0x00100000
#define XAMPROPERTY_PRI_NORMAL          0x00200000
#define XAMPROPERTY_PRI_HI              0x00400000
#define XAMPROPERTY_PRI_BACKGROUND      0x00800000
#define XAMPROPERTY_PRI_FOREGROUND      0x00080000
#define _XAMPROPERTY_PRI_LOHIMASK_      0x00700000
#define _XAMPROPERTY_PRI_XGROUNDMASK_   0x00880000
#define _XAMPROPERTY_PRI_MASK_          0x00F80000

// Misc
#define XAMPROPERTY_MISC_FIREONCE       0x01000000
#define XAMPROPERTY_MISC_ONTITLEBEHALF  0x02000000
#define XAMPROPERTY_MISC_ONSYSTEMBEHALF 0x04000000
#define XAMPROPERTY_MISC_WAITONHANDLE   0x08000000      // By default, we expect a waitable kernel object (e.g. KEVENT)
#define XAMPROPERTY_MISC_SERIALQUEUE    0x10000000      // hxamtaskqueue needs to be set
#define XAMPROPERTY_MISC_FORCE_GROUND   0x20000000      // forces _XAMPROPERTY_PRI_XGROUNDMASK_ to be followed.  Do not use this except in extreme cases!

#define _XAMPROPERTY_MISC_BEHALFMASK_   0x06000000
#define _XAMPROPERTY_MISC_MASK_         0x1F000000

#define XAMTASKMODIFY_NONE              0x00000000
#define XAMTASKMODIFY_TASKPROC          0x00000001
#define XAMTASKMODIFY_PVPARAM           0x00000002
#define XAMTASKMODIFY_ATTRIBUTES        0x00000004

typedef struct _XAMTASKATTRIBUTES
{
	DWORD                               dwProperties;
	union
	{
		DWORD                           dwPeriod;
		PVOID                           pvWaitable;
		HANDLE                          hWaitable;
		HXAMTASKQUEUE                   hxamtaskqueue;
	}typ;
} XAMTASKATTRIBUTES, *PXAMTASKATTRIBUTES;

#endif // __XAMTASK_H
