#ifndef __XAMNET_DEFINES_H
#define __XAMNET_DEFINES_H

#define SO_MCASTMEMBER		0x0005 // dword (0xEFFFFFFA)
#define SO_MARKINSECURE		0x5801 // bool TRUE for insecure
#define SO_PRIVATE			0x5802 // bool TRUE for private
#define SO_GRANTINSECURE	0x5803 // bool TRUE for insecure
#define SO_NOT_SURE1		0x5804 // dword 4 or 0x58 to 0x5B4 ?set SACK errorlevel?
#define SO_EN_SACKERRL		0x5805 // dword ?4? enable SACK

// these are in the sdk
// #define XNET_OPTID_STARTUP_PARAMS				1
// #define XNET_OPTID_NIC_XMIT_BYTES				2
// #define XNET_OPTID_NIC_XMIT_FRAMES				3
// #define XNET_OPTID_NIC_RECV_BYTES				4
// #define XNET_OPTID_NIC_RECV_FRAMES				5
// #define XNET_OPTID_CALLER_XMIT_BYTES			6
// #define XNET_OPTID_CALLER_XMIT_FRAMES			7
// #define XNET_OPTID_CALLER_RECV_BYTES			8
// #define XNET_OPTID_CALLER_RECV_FRAMES			9
#define XNET_OPTID_XPLAT_CROSS_SUBNET		0xa // min 0x4, possibly BOOL
#define XNET_OPTID_TCP_COALESCE_MODE		0xb // BOOL* min 0x4

#define XNET_OPTID_NEUTERED					0x1389 // min 0x4
#define XNET_OPTID_UNK_138A					0x138a // min 0x10
#define XNET_OPTID_UNK_138B					0x138b // min 0x18
#define XNET_OPTID_XNKID_SECURITY			0x138c // IpGetXnkidSecurity XNPXNKIDSECURITY * min 0x14
#define XNET_OPTID_SOCK_SECURITY			0x138d // SockGetSocketSecurity XNPSOCKETSECURITY * min 0x10
#define XNET_OPTID_ONLINE_PRIVS				0x138e // min 0x4
#define XNET_OPTID_MIN_FLASH_FALL06			0x138f // BOOL* min 0x4
#define XNET_OPTID_PROMISCUOUS_MODE			0x1390 // min 0x4
#define XNET_OPTID_ARP_ENTRY				0x1391 // EnetGetArpEntry XNPARPENTRY * min 0x10
#define XNET_OPTID_ROUTE_ENTRY				0x1392 // RouteGetEntryOrTable XNPROUTEENTRY * min 0x18  CXnIp::RouteSetEntry
#define XNET_OPTID_DOMAIN_NAME				0x1393 // IpGetDomainName char * min none
#define XNET_OPTID_GREYLIST					0x1394 // IpSetGreyListAddrs XNPGREYLISTADDRESSENTRY * min 0x8
#define XNET_OPTID_LINK_STATUS_OVERRIDE		0x1395 // BaseGetLinkStatusOverride XNPLINKSTATUSOVERRIDE * min 0x8
#define XNET_OPTID_UNK_1396					0x1396 // min 0x2
#define XNET_OPTID_WIRELESS_FLAGS			0x1397 // IpGetWirelessStatusSummaryFlags DWORD * min 0x4
#define XNET_OPTID_PRIMARY_SG_NONCE			0x1398 // IpLogonGetPrimarySgNonce QWORD * min 0x8
#define XNET_OPTID_SEC_ASSOC_INFO			0x1399 // IpGetSecAssocInfo XnSecAssocInfo * min 0x78
// 0x2710-272f, 0x2AF8-0x2B5C, 2730, 2732, 2733, (optid&0xD0303FF)=0xD0303FF -> BaseNicGetOpt -> kernel NicGetOpt
#define XNET_OPTID_BOUND_NIC				0x2734 // BaseModifyBoundNicDevices DWORD* min 4
#define XNET_OPTID_DSCP_VALUE				0x2f01 // PBYTE min/max 1 

typedef enum {
	XNCALLER_INVALID = 0x0,
	XNCALLER_TITLE = 0x1,
	XNCALLER_SYSAPP = 0x2,
	XNCALLER_XBDM = 0x3,
	XNCALLER_TEST = 0x4,
	NUM_XNCALLER_TYPES = 0x4,
} XNCALLER_TYPE;

#pragma pack(push, 1)
typedef struct _XNetConfigParams
{ 
	UCHAR abHash[0x14]; // 0x0 sz:0x14
	UCHAR abConfounder[0x8]; // 0x14 sz:0x8
	USHORT wszName[0x18]; // 0x1C sz:0x30
	USHORT wFlags; // 0x4C sz:0x2
	UCHAR abEnet[0x6]; // 0x4E sz:0x6
	DWORD ina; // 0x54 sz:0x4
	DWORD inaMask; // 0x58 sz:0x4
	DWORD inaGateway; // 0x5C sz:0x4
	DWORD ainaDnsServers[0x2]; // 0x60 sz:0x8
	char achDhcpHostName[0x28]; // 0x68 sz:0x28
	char achPppoeUserName[0x40]; // 0x90 sz:0x40
	char achPppoePassword[0x40]; // 0xD0 sz:0x40
	char achPppoeServer[0x28]; // 0x110 sz:0x28
	char achPppoeService[0x28]; // 0x138 sz:0x28
	LARGE_INTEGER liTimeOfLease; // 0x160 sz:0x8
	DWORD dwSecsInLease; // 0x168 sz:0x4
	DWORD inaLease; // 0x16C sz:0x4
	DWORD inaMaskLease; // 0x170 sz:0x4
	DWORD inaServerLease; // 0x174 sz:0x4
	DWORD ainaGatewaysLease[0x4]; // 0x178 sz:0x10
	DWORD ainaDnsServersLease[0x4]; // 0x188 sz:0x10
	UCHAR abEnetUPnP[0x6]; // 0x198 sz:0x6
	USHORT wPortUPnP; // 0x19E sz:0x2
	UCHAR abEnetPppoeServer[0x6]; // 0x1A0 sz:0x6
	USHORT wPppoeSessionId; // 0x1A6 sz:0x2
	UCHAR abReserved[0x44]; // 0x1A8 sz:0x44
} XNetConfigParams, *PXNetConfigParams; // size 492
#pragma pack(pop)

// these are found in the status bytes on a successful (return 0) XNetGetOpt(XNET_OPTID_ARP_ENTRY)
#define XNPARP_LOOKUP_ENTRY_SUCCESS			1
#define XNPARP_LOOKUP_ENTRY_NOT_FOUND 		2

#pragma pack(push, 1)
typedef struct _XNPARPENTRY {
	DWORD status; // 0x0 sz 4
	IN_ADDR inIp; // 0x4 sz 4
	BYTE macAddr[6]; // 0x8 sz 6
	BYTE pad[2]; // 0xC sz 2
} XNPARPENTRY, *PXNPARPENTRY; // 0x10
#pragma pack(pop)
C_ASSERT(sizeof(XNPARPENTRY) == 0x10);

typedef struct _XNPROUTEENTRY {
	DWORD dwFlags; // 0x0 sz 4
	DWORD dwType; // 0x4 sz 4 - when setting use -1 for broadcast
	IN_ADDR iaDestAddr; // 0x8 sz 4
	IN_ADDR iaGateway; // 0xC sz 4
	IN_ADDR iaInterface; // 0x10 sz 4
	IN_ADDR iaUnk; // 0x14 sz 4
} XNPROUTEENTRY, *PXNPROUTEENTRY; // 0x18
C_ASSERT(sizeof(XNPROUTEENTRY) == 0x18);

typedef struct _XnSecAssocInfo {
   IN_ADDR inaddrSec; // 0x00-0x04
   XNCALLER_TYPE CallerType; // 0x04-0x08
   DWORD dwInfoFlags; // 0x08-0x0C
   DWORD dwReserved1; // 0x0C-0x10
   XNKID xnkid; // 0x10-0x18
   XNADDR xnaddr; // 0x18-0x3C
   DWORD dwServiceId; // 0x3C-0x40
   IN_ADDR inaddrRemote; // 0x40-0x44
   WORD wRemotePort; // 0x44-0x46
   BYTE bState; // 0x46-0x47
   BYTE bReserved2; // 0x47-0x48
   IN_ADDR inaddrNat; // 0x48-0x4C
   WORD wNatPort; // 0x4C-0x4E
   BYTE abReserved3[0x2A]; // 0x4E-0x78
} XnSecAssocInfo, *PXnSecAssocInfo; // 0x78
C_ASSERT(sizeof(XnSecAssocInfo) == 0x78);

typedef struct XNetCapBuffer { 
	DWORD cbCap; // 0x0 sz:0x4
	PBYTE pbCapHead; // 0x4 sz:0x4
	PBYTE pbCapTail; // 0x8 sz:0x4
	DWORD cbCapFree; // 0xC sz:0x4
} XNetCapBuffer, *PXNetCapBuffer; // size 16
C_ASSERT(sizeof(XNetCapBuffer) == 0x10);

typedef union _UPNP_VALUE {
	int   iValue; // 0x00-0x04
	CHAR* pszValue; // 0x00-0x04
} UPNP_VALUE, *PUPNP_VALUE;  // 0x04

typedef struct _UPNP_ACTION_OUTPUT_PARAMETER {
	CHAR* pszName; // 0x00-0x04
	enum  UPNP_VALUE_TYPE ValueType; // 0x04-0x08
	DWORD dwValueSize; // 0x08-0x0C
	UPNP_VALUE Value; // 0x0C-0x10
} UPNP_ACTION_OUTPUT_PARAMETER, *PUPNP_ACTION_OUTPUT_PARAMETER; // 0x10

typedef struct _UPNP_DEVICE {
	CHAR		szDeviceUuid[0x80];
	struct sockaddr_in saddrinHost;
	CHAR		szRelativePath[0x100];
	DWORD		dwLastUpdateTime;
} UPNP_DEVICE, *PUPNP_DEVICE;

typedef struct _UPNP_SERVICE_DESCRIPTION {
   CHAR szType[0x80];
   CHAR szId[0x81];
   WORD wDescriptionPort;
   CHAR szDescriptionURL[0x100];
   WORD wControlPort;
   CHAR szControlURL[0x100];
   WORD wEventPort;
   CHAR szEventURL[0x100];
   struct _UPNP_DEVICE_DESCRIPTION *  pOwningDevice;
   struct _UPNP_SERVICE_DESCRIPTION * pNextService;
} UPNP_SERVICE_DESCRIPTION, *PUPNP_SERVICE_DESCRIPTION;

typedef struct _UPNP_DEVICE_DESCRIPTION {
   CHAR szUuid[0x80];
   CHAR szType[0x80];
   CHAR szFriendlyName[0x41];
   CHAR szManufacturer[0x41];
   CHAR szManufacturerURL[0x100];
   CHAR szModelName[0x21];
   CHAR szModelDescription[0x81];
   CHAR szModelNumber[0x21];
   CHAR szModelURL[0x100];
   CHAR szSerialNumber[0x41];
   CHAR szUPC[0xD];
   struct _UPNP_DEVICE_DESCRIPTION * pParent;
   struct _UPNP_DEVICE_DESCRIPTION * pNextSibling;
   struct _UPNP_DEVICE_DESCRIPTION * pFirstChild;
   struct _UPNP_SERVICE_DESCRIPTION * pFirstService;
} UPNP_DEVICE_DESCRIPTION, *PUPNP_DEVICE_DESCRIPTION;

typedef int (*LP_INTERCEPT_XMIT_FUNC)(PVOID pvCallbackUserData, const BYTE * pbXmitData, DWORD dwCbData); // return 0 on success
typedef int (*LP_INTERCEPT_RECV_FUNC)(PVOID pvCallbackUserData, const BYTE * pbRecvData, DWORD dwCbData); // return 0 on success

#ifdef __cplusplus
extern "C" {
#endif
	// **WARNING** this is never called directly UNLESS dash version is < 2610
	// NTSYSAPI
	// EXPORTNUM(1)
	// int
	// NTAPI
	// NetDll_WSAStartup(
	// 	IN		XNCALLER_TYPE xnc
    // 	IN		WORD wVersionRequested,
    // 	OUT		LPWSADATA lpWSAData
	// );

	NTSYSAPI
	EXPORTNUM(2)
	int
	NTAPI
	NetDll_WSACleanup(
		IN		XNCALLER_TYPE xnc
	);

	NTSYSAPI
	EXPORTNUM(3)
	SOCKET
	NTAPI
	NetDll_socket(
		IN		XNCALLER_TYPE xnc,
		IN		int af,
		IN		int type,
		IN		int protocol
	);

	NTSYSAPI
	EXPORTNUM(4)
	int
	NTAPI
	NetDll_closesocket(
		IN		XNCALLER_TYPE xnc,
		IN		SOCKET s
	);
	
	NTSYSAPI
	EXPORTNUM(5)
	int
	NTAPI
	NetDll_shutdown(
		IN		XNCALLER_TYPE xnc,
		IN		SOCKET s,
		IN		int how
	);
	
	NTSYSAPI
	EXPORTNUM(6)
	int
	NTAPI
	NetDll_ioctlsocket(
		IN		XNCALLER_TYPE xnc,
		IN		SOCKET s,
		IN		long cmd,
		IN		u_long FAR * argp
	);

	NTSYSAPI
	EXPORTNUM(7)
	int
	NTAPI
	NetDll_setsockopt(
		IN		XNCALLER_TYPE xnc,
		IN		SOCKET s,
		IN		int level,
		IN		int optname,
		IN		const char FAR * optval,
		IN		int optlen
	);

	NTSYSAPI
	EXPORTNUM(8)
	int
	NTAPI
	NetDll_getsockopt(
		IN		XNCALLER_TYPE xnc,
		IN		SOCKET s,
		IN		int level,
		IN		int optname,
		IN		const char FAR * optval,
		IN		int* optlen
	);

	NTSYSAPI
	EXPORTNUM(9)
	int
	NTAPI
	NetDll_getsockname(
		IN		XNCALLER_TYPE xnc,
		IN		SOCKET s,
		IN OUT	struct sockaddr FAR * name,
		IN		int FAR * namelen
	);

	NTSYSAPI
	EXPORTNUM(10)
	int
	NTAPI
	NetDll_getpeername(
		IN		XNCALLER_TYPE xnc,
		IN		SOCKET s,
		IN OUT	struct sockaddr FAR * name,
		IN		int FAR * namelen
	);

	NTSYSAPI
	EXPORTNUM(11)
	int
	NTAPI
	NetDll_bind(
		IN		XNCALLER_TYPE xnc,
		IN		SOCKET s,
		IN		const struct sockaddr FAR * name,
		IN		int namelen
	);
	
	NTSYSAPI
	EXPORTNUM(12)
	int
	NTAPI
	NetDll_connect(
		IN		XNCALLER_TYPE xnc,
		IN		SOCKET s,
		IN		const struct sockaddr FAR * name,
		IN		int namelen
	);

	NTSYSAPI
	EXPORTNUM(13)
	int
	NTAPI
	NetDll_listen(
		IN		XNCALLER_TYPE xnc,
		IN		SOCKET s,
		IN		int backlog
	);

	NTSYSAPI
	EXPORTNUM(14)
	SOCKET
	NTAPI
	NetDll_accept(
		IN		XNCALLER_TYPE xnc,
		IN		SOCKET s,
		IN		struct sockaddr FAR * addr,
		IN		int FAR * addrlen
	);

	NTSYSAPI
	EXPORTNUM(15)
	int
	NTAPI
	NetDll_select(
		IN		XNCALLER_TYPE xnc,
		IN		int nfds,
		IN		fd_set FAR * readfds,
		IN		fd_set FAR * writefds,
		IN		fd_set FAR *exceptfds,
		IN		const struct timeval FAR * timeout
	);

	NTSYSAPI
	EXPORTNUM(16)
	BOOL
	NTAPI
	NetDll_WSAGetOverlappedResult(
		IN		XNCALLER_TYPE xnc,
    	IN		SOCKET s,
    	IN		LPOVERLAPPED lpOverlapped,
    	OUT		LPDWORD lpcbTransfer,
		IN		BOOL fWait,
    	OUT		LPDWORD lpdwFlags
	);

	NTSYSAPI
	EXPORTNUM(17)
	int
	NTAPI
	NetDll_WSACancelOverlappedIO(
		IN		XNCALLER_TYPE xnc,
    	IN		SOCKET s
	);

	NTSYSAPI
	EXPORTNUM(18)
	int
	NTAPI
	NetDll_recv(
		IN		XNCALLER_TYPE xnc,
		IN		SOCKET s,
		IN OUT	const char FAR * buf,
		IN		int len,
		IN		int flags
	);

	NTSYSAPI
	EXPORTNUM(19)
	int
	NTAPI
	NetDll_WSARecv(
		IN		XNCALLER_TYPE xnc,
		IN		SOCKET s,
		IN		LPWSABUF lpBuffers,
		IN		DWORD dwBufferCount,
		OUT 	LPDWORD lpNumberOfBytesRecvd OPTIONAL,
		IN OUT 	LPDWORD lpFlags,
		IN		LPOVERLAPPED lpOverlapped OPTIONAL,
		IN		LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine OPTIONAL
	);

	NTSYSAPI
	EXPORTNUM(20)
	int
	NTAPI
	NetDll_recvfrom(
		IN		XNCALLER_TYPE xnc,
		IN		SOCKET s,
		IN		const char* buf,
		IN		int len,
		IN		int flags,
		IN OUT	void* from,
		IN		int fromlen
	);

	NTSYSAPI
	EXPORTNUM(21)
	int
	NTAPI
	NetDll_WSARecvFrom(
		IN		XNCALLER_TYPE xnc,
		IN		SOCKET s,
		IN		LPWSABUF lpBuffers,
		IN		DWORD dwBufferCount,
		OUT 	LPDWORD lpNumberOfBytesRecvd OPTIONAL,
		IN OUT 	LPDWORD lpFlags,
		OUT		struct sockaddr FAR * lpFrom  OPTIONAL,
		IN OUT	LPINT lpFromlen  OPTIONAL,
		IN		LPOVERLAPPED lpOverlapped OPTIONAL,
		IN		LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine OPTIONAL
	);

	NTSYSAPI
	EXPORTNUM(22)
	int
	NTAPI
	NetDll_send(
		IN		XNCALLER_TYPE xnc,
		IN		SOCKET s,
		IN		const char FAR * buf,
		IN		int len,
		IN		int flags
	);
	
	NTSYSAPI
	EXPORTNUM(23)
	int
	NTAPI
	NetDll_WSASend(
		IN		XNCALLER_TYPE xnc,
		IN		SOCKET s,
		IN		LPWSABUF lpBuffers,
		IN		DWORD dwBufferCount,
		OUT 	LPDWORD lpNumberOfBytesSent OPTIONAL,
		IN		DWORD dwFlags,
		IN		LPOVERLAPPED lpOverlapped OPTIONAL,
		IN		LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine OPTIONAL
	);

	NTSYSAPI
	EXPORTNUM(24)
	int
	NTAPI
	NetDll_sendto(
		IN		XNCALLER_TYPE xnc,
		IN		SOCKET s,
		IN		const VOID* buf,
		IN		int len,
		IN		int flags,
		IN		VOID* to,
		IN		int tolen
	);

	NTSYSAPI
	EXPORTNUM(25)
	int
	NTAPI
	NetDll_WSASendTo(
		IN		XNCALLER_TYPE xnc,
		IN		SOCKET s,
		IN		LPWSABUF lpBuffers,
		IN		DWORD dwBufferCount,
		OUT 	LPDWORD lpNumberOfBytesSent OPTIONAL,
		IN		DWORD dwFlags,
		IN		const struct sockaddr FAR * lpTo,
		IN		int iTolen,
		IN		LPOVERLAPPED lpOverlapped OPTIONAL,
		IN		LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine OPTIONAL
	);

	NTSYSAPI
	EXPORTNUM(26)
	DWORD
	NTAPI
	NetDll_inet_addr(
		IN		const char *cp
	);
	
	NTSYSAPI
	EXPORTNUM(27)
	int
	NTAPI
	NetDll_WSAGetLastError(
		VOID
	);

	NTSYSAPI
	EXPORTNUM(28)
	void
	NTAPI
	NetDll_WSASetLastError(
		IN		int iError
	);

	NTSYSAPI
	EXPORTNUM(29)
	WSAEVENT
	NTAPI
	NetDll_WSACreateEvent(
		void
	);

	NTSYSAPI
	EXPORTNUM(30)
	BOOL
	NTAPI
	NetDll_WSACloseEvent(
		IN		WSAEVENT hEvent
	);

	NTSYSAPI
	EXPORTNUM(31)
	BOOL
	NTAPI
	NetDll_WSASetEvent(
		IN		WSAEVENT hEvent
	);
	
	NTSYSAPI
	EXPORTNUM(32)
	BOOL
	NTAPI
	NetDll_WSAResetEvent(
		IN		WSAEVENT hEvent
	);

	NTSYSAPI
	EXPORTNUM(33)
	DWORD
	NTAPI
	NetDll_WSAWaitForMultipleEvents(
    	IN		DWORD cEvents,
    	IN		const WSAEVENT FAR * lphEvents,
    	IN		BOOL fWaitAll,
    	IN		DWORD dwTimeout,
    	IN		BOOL fAlertable
	);

	NTSYSAPI
	EXPORTNUM(34)
	int
	NTAPI
	NetDll___WSAFDIsSet(
    	IN		SOCKET s,
    	OUT		fd_set* fds
	);

	NTSYSAPI
	EXPORTNUM(35)
	int
	NTAPI
	NetDll_WSAEventSelect(
		IN		XNCALLER_TYPE xnc,
    	IN		SOCKET s,
    	IN		WSAEVENT hEventObject,
    	IN		long lNetworkEvents
	);

	NTSYSAPI
	EXPORTNUM(36)
	int
	NTAPI
	NetDll_WSAStartupEx(
		IN		XNCALLER_TYPE xnc,
		IN		WORD wVersionRequested,
		IN		LPWSADATA wsad,
		IN		DWORD versionReq
	);

	NTSYSAPI
	EXPORTNUM(51)
	int
	NTAPI
	NetDll_XNetStartup(
		IN		XNCALLER_TYPE xnc,
		IN		XNetStartupParams* xnsp
	);

	NTSYSAPI
	EXPORTNUM(52)
	int
	NTAPI
	NetDll_XNetCleanup(
		IN		XNCALLER_TYPE xnc
	);

	NTSYSAPI
	EXPORTNUM(53)
	int
	NTAPI
	NetDll_XNetRandom(
		IN		XNCALLER_TYPE xnc,
		OUT		BYTE * pb,
		IN		DWORD cb
	);


	NTSYSAPI
	EXPORTNUM(54)
	int
	NTAPI
	NetDll_XNetCreateKey(
		IN		XNCALLER_TYPE xnc,
        OUT		XNKID *pxnkid,
        OUT		XNKEY *pxnkey
	);

	NTSYSAPI
	EXPORTNUM(55)
	int
	NTAPI
	NetDll_XNetRegisterKey(
		IN		XNCALLER_TYPE xnc,
		IN		const XNKID * pxnkid,
		IN		const XNKEY * pxnkey
	);

	NTSYSAPI
	EXPORTNUM(56)
	int
	NTAPI
	NetDll_XNetUnregisterKey(
		IN		XNCALLER_TYPE xnc,
		IN		const XNKID * pxnkid
	);

	NTSYSAPI
	EXPORTNUM(57)
	int
	NTAPI
	NetDll_XNetXnAddrToInAddr(
		IN		XNCALLER_TYPE xnc,
		IN		XNADDR * pxna,
        IN		XNKID * pxnkid,
        OUT		PIN_ADDR pina
	);

	NTSYSAPI
	EXPORTNUM(58)
	int
	NTAPI
	NetDll_XNetServerToInAddr(
		IN		XNCALLER_TYPE xnc,
		IN		const IN_ADDR ina,
		IN		DWORD dwServiceId,
		OUT		PIN_ADDR pina
	);

	NTSYSAPI
	EXPORTNUM(59)
	int
	NTAPI
	NetDll_XNetTsAddrToInAddr(
		IN		XNCALLER_TYPE xnc,
		IN		const TSADDR * ptsa,
		IN		DWORD dwServiceId,
		IN		const XNKID * pxnkid,
		OUT		PIN_ADDR pina
	);

	NTSYSAPI
	EXPORTNUM(60)
	int
	NTAPI
	NetDll_XNetInAddrToXnAddr(
		IN		XNCALLER_TYPE xnc,
		IN		IN_ADDR ina,
        OUT		XNADDR * pxna,
        IN		XNKID * pxnkid
	);

	NTSYSAPI
	EXPORTNUM(61)
	int
	NTAPI
	NetDll_XNetInAddrToServer(
		IN		XNCALLER_TYPE xnc,
		IN		const IN_ADDR ina,
		OUT		PIN_ADDR pina
	);

	NTSYSAPI
	EXPORTNUM(62)
	int
	NTAPI
	NetDll_XNetInAddrToString(
		IN		XNCALLER_TYPE xnc,
		IN		const IN_ADDR ina,
		OUT		char * pchBuf,
		IN		int cchBuf
	);

	NTSYSAPI
	EXPORTNUM(63)
	int
	NTAPI
	NetDll_XNetUnregisterInAddr(
		IN		XNCALLER_TYPE xnc,
		IN		const IN_ADDR ina
	);

	NTSYSAPI
	EXPORTNUM(64)
	int
	NTAPI
	NetDll_XNetXnAddrToMachineId(
		IN		XNCALLER_TYPE xnc,
		IN		const XNADDR * pxnaddr,
		OUT		ULONGLONG * pqwMachineId
	);

	NTSYSAPI
	EXPORTNUM(65)
	int
	NTAPI
	NetDll_XNetConnect(
		IN		XNCALLER_TYPE xnc,
        IN		const IN_ADDR ina
	);

	NTSYSAPI
	EXPORTNUM(66)
	DWORD
	NTAPI
	NetDll_XNetGetConnectStatus(
		IN		XNCALLER_TYPE xnc,
		IN		const IN_ADDR ina
	);

	NTSYSAPI
	EXPORTNUM(67)
	int
	NTAPI
	NetDll_XNetDnsLookup(
		IN		XNCALLER_TYPE xnc,
		IN		LPCSTR pszHost,
		IN		WSAEVENT hEvent,
		IN		XNDNS ** ppXNDns
	);

	NTSYSAPI
	EXPORTNUM(68)
	int
	NTAPI
	NetDll_XNetDnsRelease(
		IN		XNCALLER_TYPE xnc,
		IN		XNDNS * pxndns
	);

	NTSYSAPI
	EXPORTNUM(69)
	int
	NTAPI
	NetDll_XNetQosListen(
		IN		XNCALLER_TYPE xnc,
		IN		const XNKID * pxnkid,
		IN		const BYTE * pb,
		IN		DWORD cb,
		IN		DWORD dwBitsPerSec,
		IN		DWORD dwFlags	
	);

	NTSYSAPI
	EXPORTNUM(70)
	int
	NTAPI
	NetDll_XNetQosLookup(
		IN		XNCALLER_TYPE xnc,
		IN		DWORD cxna,
		IN		const XNADDR * apxna[] OPTIONAL,
		IN		const XNKID * apxnkid[] OPTIONAL,
		IN		const XNKEY * apxnkey[] OPTIONAL,
		IN		UINT cina,
		IN		const IN_ADDR aina[] OPTIONAL,
		IN		const DWORD adwServiceId[] OPTIONAL,
		IN		DWORD dwBitsPerSec,
		IN		DWORD dwFlags,
		IN		WSAEVENT hEvent OPTIONAL,
		OUT		XNQOS ** ppxnqos
	);

	NTSYSAPI
	EXPORTNUM(71)
	int
	NTAPI
	NetDll_XNetQosServiceLookup(
		IN		XNCALLER_TYPE xnc,
		IN		DWORD dwFlags,
		IN		WSAEVENT hEvent OPTIONAL,
		OUT		XNQOS ** ppxnqos
	);

	NTSYSAPI
	EXPORTNUM(72)
	int
	NTAPI
	NetDll_XNetQosRelease(
		IN		XNCALLER_TYPE xnc,
		IN		XNQOS * pxnqos
	);

	NTSYSAPI
	EXPORTNUM(73)
	DWORD
	NTAPI
	NetDll_XNetGetTitleXnAddr(
		IN		XNCALLER_TYPE xnc,
		OUT		XNADDR * pxna
	);

	NTSYSAPI
	EXPORTNUM(74)
	DWORD
	NTAPI
	NetDll_XNetGetDebugXnAddr(
		IN		XNCALLER_TYPE xnc,
		OUT		XNADDR * pxna
	);

	NTSYSAPI
	EXPORTNUM(75)
	DWORD
	NTAPI
	NetDll_XNetGetEthernetLinkStatus(
		IN		XNCALLER_TYPE xnc
	);

	NTSYSAPI
	EXPORTNUM(76)
	DWORD
	NTAPI
	NetDll_XNetGetBroadcastVersionStatus(
		IN		XNCALLER_TYPE xnc,
		IN		BOOL fReset
	);

	NTSYSAPI
	EXPORTNUM(77)
	int
	NTAPI
	NetDll_XNetQosGetListenStats(
		IN		XNCALLER_TYPE xnc,
		IN		const XNKID * pxnkid,
		IN OUT	XNQOSLISTENSTATS * pQosListenStats
	);

	NTSYSAPI
	EXPORTNUM(78)
	int
	NTAPI
	NetDll_XNetGetOpt(
		IN		XNCALLER_TYPE xnc,
		IN		DWORD dwOptId,
		OUT		BYTE * pbValue,
		IN OUT	DWORD * pdwValueSize
	);

	NTSYSAPI
	EXPORTNUM(79)
	int
	NTAPI
	NetDll_XNetSetOpt(
		IN		XNCALLER_TYPE xnc,
		IN		DWORD dwOptId,
		IN		BYTE * pbValue,
		IN		DWORD pdwValueSize
	);

	NTSYSAPI
	EXPORTNUM(80)
	int
	NTAPI
	NetDll_XNetStartupEx(
		IN		XNCALLER_TYPE xnc,
		IN		XNetStartupParams * xnsp,
		IN		DWORD versionReq
	);

	NTSYSAPI
	EXPORTNUM(83)
	int
	NTAPI
	NetDll_XNetGetSystemLinkPort(
		IN		XNCALLER_TYPE xnc,
		OUT		WORD * pwSystemLinkPort
	);

	NTSYSAPI
	EXPORTNUM(84)
	int
	NTAPI
	NetDll_XNetSetSystemLinkPort(
		IN		XNCALLER_TYPE xnc,
		IN		WORD wSystemLinkPort
	);

	NTSYSAPI
	EXPORTNUM(104)
	int
	NTAPI
	NetDll_XnpConfig(
		IN		XNCALLER_TYPE xnc,
		IN		PXNetConfigParams pnetconfigparm,
		IN		DWORD ovl
	);

	NTSYSAPI
	EXPORTNUM(108)
	int
	NTAPI
	NetDll_XnpCapture(
		IN		XNCALLER_TYPE xnc,
		IN		PXNetCapBuffer pXncb
	);

	NTSYSAPI
	EXPORTNUM(110)
	int
	NTAPI
	NetDll_XnpEthernetInterceptXmit(
		IN		XNCALLER_TYPE xnc,
		IN		PBYTE pb,
		IN		DWORD cb
	);

	NTSYSAPI
	EXPORTNUM(109)
	int
	NTAPI
	NetDll_XnpEthernetInterceptSetCallbacks(
		IN		XNCALLER_TYPE xnc,
		IN		LP_INTERCEPT_XMIT_FUNC pfnEnetInterceptXmitCallback,
		IN		LP_INTERCEPT_RECV_FUNC pfnEnetInterceptRecvCallback,
		IN		PVOID pvCallbackUserData,
		IN		DWORD dwFlags
	);

	NTSYSAPI
	EXPORTNUM(111)
	VOID
	NTAPI
	NetDll_XnpEthernetInterceptRecv(
		IN		XNCALLER_TYPE xnc,
		IN		PBYTE pb,
		IN		DWORD cb
	);

	NTSYSAPI
	EXPORTNUM(129)
	DWORD
	NTAPI
	NetDll_XnpGetSecAssocList(
		IN		XNCALLER_TYPE xnc,
		IN		PXnSecAssocInfo pSecAssocList,
		IN		PDWORD dwLength
	);

	NTSYSAPI
	EXPORTNUM(134)
	int
	NTAPI
	NetDll_XnpEthernetInterceptSetExtendedReceiveCallback(
		IN		XNCALLER_TYPE xnc,
		IN		LP_INTERCEPT_RECV_FUNC pfnEnetInterceptExtRecvCallback,
		IN		PVOID pvCallbackUserData,
		IN		DWORD dwFlags
	);
	
	NTSYSAPI
	EXPORTNUM(142)
	int
	NTAPI
	NetDll_XnpEthernetInterceptXmitAsIp(
		IN		XNCALLER_TYPE xnc,
		IN		const BYTE * pbData,
		IN		DWORD dwCb,
		IN		DWORD dwFlags
	);
	// *** THESE NEED TO BE CHECKED!!! ***
	NTSYSAPI
	EXPORTNUM(106)
	VOID
	NTAPI
	NetDll_XnpLoadMachineAccount(
		IN		DWORD dwUnk,
		IN		char* abUnk
	);

	NTSYSAPI
	EXPORTNUM(251)
	DWORD 
	NTAPI
	NetDll_UpnpStartup(
		IN	XNCALLER_TYPE CallerType
	);

	
	NTSYSAPI
	EXPORTNUM(252)
	DWORD 
	NTAPI
	NetDll_UpnpCleanup(
		IN	XNCALLER_TYPE CallerType
	);

	NTSYSAPI
	EXPORTNUM(253)
	DWORD 
	NTAPI
	NetDll_UpnpSearchCreate(
		IN		XNCALLER_TYPE CallerType, 
		IN		CHAR* szSearchTarget, 
		IN		DWORD dwMaxCachedDevices, 
		IN		DWORD dwSendTryCount, 
		IN		DWORD dwSendInterval, 
		IN		HANDLE hWorkEvent, 
		IN		DWORD dwFlags, 
		OUT		HANDLE* phUpnpSearch
	);

	NTSYSAPI
	EXPORTNUM(254)
	DWORD 
	NTAPI
	NetDll_UpnpSearchGetDevices(
		IN		XNCALLER_TYPE CallerType, 
		IN		HANDLE hUpnpSearch, 
		OUT		struct _UPNP_DEVICE* paUpnpDevices, 
		OUT		DWORD* pdwNumUpnpDevices
	);

	NTSYSAPI
	EXPORTNUM(255)
	DWORD 
	NTAPI
	NetDll_UpnpDescribeCreate(
		IN		XNCALLER_TYPE CallerType, 
		IN OUT  struct _UPNP_DEVICE* pUpnpDevice, 
		IN		DWORD dwMaxNUmDevices, 
		IN		DWORD dwMaxNumServices, 
		IN		HANDLE hWorkEvent, 
		OUT		HANDLE* phUpnpDescribe
	);

	NTSYSAPI
	EXPORTNUM(256)
	DWORD 
	NTAPI
	NetDll_UpnpDescribeGetResults(
		IN		XNCALLER_TYPE CallerType, 
		IN		HANDLE hUpnpDescibe, 
		IN OUT	struct _UPNP_DEVICE_DESCRIPTION** ppaUpnpDeviceDescriptions, 
		IN OUT	DWORD* pdwNumUpnpDeviceDescriptions,
		IN OUT	struct _UPNP_SERVICE_DESCRIPTION** ppaUpnpServiceDescriptions, 
		IN OUT	DWORD* pdwNumUpnpServicesDescriptions
	);

	NTSYSAPI
	EXPORTNUM(258)
	DWORD 
	NTAPI
	NetDll_UpnpActionCreate(
		IN		XNCALLER_TYPE CallerType, 
		IN		struct sockaddr_in* psaddrinHost, 
		IN		CHAR* szControlURL, 
		IN		CHAR* szServiceType, 
		IN		CHAR* szAction, 
		IN		CHAR* pcInputParameters, 
		IN		DWORD dwInputParametersLength,
		IN OUT	struct _UPNP_ACTION_OUTPUT_PARAMETER* aOutputParameters, 
		IN OUT	DWORD dwNumOutputParameters, 
		IN OUT	BYTE* pbyWorkBuffer, 
		IN OUT	DWORD dwWOrkBufferSize, 
		IN		HANDLE hWorkEvent, 
		IN OUT	HANDLE* phUpnpAction
	);

	NTSYSAPI
	EXPORTNUM(259)
	DWORD 
	NTAPI
	NetDll_UpnpActionGetResults(
		IN		XNCALLER_TYPE CallerType, 
		IN		HANDLE hUpnpAction, 
		OUT		struct _UPNP_ACTION_OUTPUT_PARAMETER** ppaOutputParameters, 
		OUT		DWORD* pdwNumOutputParameters
	);

	NTSYSAPI
	EXPORTNUM(263)
	DWORD 
	NTAPI
	NetDll_UpnpDoWork(
		IN	XNCALLER_TYPE CallerType, 
		IN	HANDLE hHandle, 
		IN	DWORD dwNumMs
	);

	NTSYSAPI
	EXPORTNUM(264)
	DWORD 
	NTAPI
	NetDll_UpnpCloseHandle(
		IN	XNCALLER_TYPE CallerType, 
		IN	HANDLE hHandle
	);

	NTSYSAPI
	EXPORTNUM(308)
	VOID
	NTAPI
	XNetLogonSetConsoleCertificate(
		IN		const char* szCertificate,
		IN		DWORD dwcchCertificate
	);

	NTSYSAPI
	EXPORTNUM(314)
	VOID
	NTAPI
	XNetLogonSetTitleID(
		IN		DWORD dwTitleId
	);

#ifdef __cplusplus
}
#endif

#endif // __XAMNET_DEFINES_H
