// general things found in the xapilib.lib from the sdk, but not in the sdk headers
#ifndef __XAPIEXT_DEFINES_H
#define __XAPIEXT_DEFINES_H


#ifdef __cplusplus
extern "C" {
#endif

	VOID
	RtlSnprintf(
		CHAR * Buffer,
		DWORD MaxLength,
		LPCSTR Format,
		...
	);
	
	// untested
	VOID
	RtlSwprintf(
		WCHAR * Buffer,
		DWORD MaxLength,
		LPCWSTR Format,
		...
	);
	
	VOID
	RtlSnwprintf(
		WCHAR * Buffer,
		DWORD MaxLength,
		LPCWSTR Format,
		...
	);	

	// SDK library function
	VOID XapiThreadStartup(
		IN		VOID (__cdecl *StartRoutine)(VOID *),
		IN		PVOID StartContext,
		IN		DWORD dwExitCode
	);
	
	// SDK library function
	HANDLE NTAPI XapipCreateThread(
		IN		LPSECURITY_ATTRIBUTES lpThreadAttributes,
		IN		DWORD dwStackSize,
		IN		LPTHREAD_START_ROUTINE lpStartAddress,
		IN		LPVOID lpParameter,
		IN		DWORD dwCreationFlags,
		IN		DWORD dwThreadProcessor,
		IN		LPDWORD lpThreadId,
		IN OUT	PHANDLE pHandle
	);

	// SDK library function? sdk links it directly to XamLoaderGetGameInfo @427
	// UNTESTED!!!
	HRESULT
	XamGetGameInfo(
		LPCWSTR wszTitleName,
		DWORD dwTitleId,
		PBYTE* ppbUnk,
		QWORD* pqwUnk
	);
	// SDK library function?
	DWORD
	XContentCreateDeviceEnumerator(
		DWORD dwContentTypes,
		DWORD dwFlags,
		DWORD cItem,
		PDWORD pcbBuffer,
		PHANDLE phEnum
	);

	// disc format function for fatx found in xapilib.lib
	// ie "\\Device\\SomeDevice", use RtlInitAnsiString and modify ??((Length+0x10000) - 1)??
	NTSTATUS
	XapiFormatFATVolume(
		PANSI_STRING deviceName
	);
	
	NTSTATUS
	XapiFormatFATVolumeEx(
		PANSI_STRING deviceName,
		DWORD sectorSize
	);
	
	// disc format functions for fat32 found in xapilib.lib
	NTSTATUS
	XapiFormatFATWVolume(
		LPCSTR devicename
	);
	
	NTSTATUS
	XapiFormatFATWVolumeByHandle(
		HANDLE deviceHandle
	);

#ifdef __cplusplus
}
#endif

#endif // __XAPIEXT_DEFINES_H
