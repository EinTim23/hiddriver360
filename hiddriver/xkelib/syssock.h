#ifndef _SYSTEM_SOCKET_H
#define _SYSTEM_SOCKET_H

#define _USE_SYS_SOCKET_REPLACEMENT

#define CUR_VER 0x20352400   //  (((XboxKrnlVersion->Major&0xF)<<28) | ((XboxKrnlVersion->Minor)<<24) | ((XboxKrnlVersion->Build &0xFFFF)<<8) | ((XboxKrnlVersion->Qfe&0xFF)))

#ifdef _USE_SYS_SOCKET_REPLACEMENT
	#define WSAStartup(x,a)						NetDll_WSAStartupEx(XNCALLER_SYSAPP, 2, a, CUR_VER) // x may always need to be 2... just sayin
	#define WSACleanup()						NetDll_WSACleanup(XNCALLER_SYSAPP)
	#define socket(a,t,p)						NetDll_socket(XNCALLER_SYSAPP, a, t ,p)
	#define closesocket(s)						NetDll_closesocket(XNCALLER_SYSAPP, s)
	#define shutdown(s,h)						NetDll_shutdown(XNCALLER_SYSAPP, s, h)
	#define ioctlsocket(s,c,a)					NetDll_ioctlsocket(XNCALLER_SYSAPP, s, c, a)
	#define setsockopt(s,lv,o,v,l)				NetDll_setsockopt(XNCALLER_SYSAPP, s, lv, o, v, l)
	#define getsockopt(s,lv,o,v,l)				NetDll_getsockopt(XNCALLER_SYSAPP, s, lv, o, v, l)
	#define getsockname(s,n,l)					NetDll_getsockname(XNCALLER_SYSAPP, s, n, l)
	#define getpeername(s,n,l)					NetDll_getpeername(XNCALLER_SYSAPP, s, n, l)
	#define bind(s,n,l)							NetDll_bind(XNCALLER_SYSAPP, s, n, l)
	#define connect(s,n,l)						NetDll_connect(XNCALLER_SYSAPP, s, n, l)
	#define listen(s,b)							NetDll_listen(XNCALLER_SYSAPP, s, b)
	#define accept(s,a,l)						NetDll_accept(XNCALLER_SYSAPP, s, a, l)
	#define select(n,r,w,e,t)					NetDll_select(XNCALLER_SYSAPP, n, r, w, e, t)
	#define WSAGetOverlappedResult(s,o,c,w,f)	NetDll_WSAGetOverlappedResult(XNCALLER_SYSAPP, s, o, c, w, f)
	#define WSACancelOverlappedIO(s)			NetDll_WSACancelOverlappedIO(XNCALLER_SYSAPP, s)	
	#define recv(s,b,l,f)						NetDll_recv(XNCALLER_SYSAPP, s, b, l, f)
	#define WSARecv(s,b,c,r,f,o,d) 				NetDll_WSARecv(XNCALLER_SYSAPP, s, b, c, r, f, o, d)
	#define recvfrom(s,b,l,f,a,n)				NetDll_recvfrom(XNCALLER_SYSAPP, s, b, l, f, a, n)
	#define WSARecvFrom(s,b,c,n,f,a,m,o,r)		NetDll_WSARecvFrom(XNCALLER_SYSAPP, s, b, c, n, f, a, m, o, r)
	#define send(s,b,l,f)						NetDll_send(XNCALLER_SYSAPP, s, b, l, f)
	#define WSASend(s,l,b,n,f,p,c)				NetDll_WSASend(XNCALLER_SYSAPP, s, l, b, n, f, p, c)
	#define sendto(s,b,l,f,a,n)					NetDll_sendto(XNCALLER_SYSAPP, s, b, l, f, a, n)
	#define WSASendTo(s,b,c,n,f,a,l,o,r)		NetDll_WSASendTo(XNCALLER_SYSAPP, s, b, c, n, f, a, l, o, r)
	#define WSAEventSelect(s,h,n)				NetDll_WSAEventSelect(XNCALLER_SYSAPP, s, h, n)

	#define inet_addr(c)						NetDll_inet_addr(c)
	#define WSAGetLastError						NetDll_WSAGetLastError
	#define WSASetLastError						NetDll_WSASetLastError
	#define WSACreateEvent						NetDll_WSACreateEvent
	#define WSACloseEvent						NetDll_WSACloseEvent
	#define WSASetEvent							NetDll_WSASetEvent
	#define WSAResetEvent						NetDll_WSAResetEvent
	#define WSAWaitForMultipleEvents			NetDll_WSAWaitForMultipleEvents
	#define __WSAFDIsSet						NetDll___WSAFDIsSet

	#define XNetStartup(p)						NetDll_XNetStartupEx(XNCALLER_SYSAPP, p, CUR_VER) // there is also an actual NetDll_XNetStartup
	#define XNetGetEthernetLinkStatus()			NetDll_XNetGetEthernetLinkStatus(XNCALLER_SYSAPP)
	#define XNetDnsLookup(h,e,x)				NetDll_XNetDnsLookup(XNCALLER_SYSAPP, h, e, x)
	#define XNetDnsRelease(h)					NetDll_XNetDnsRelease(XNCALLER_SYSAPP, h)

	#define XNetCleanup							NetDll_XNetCleanup(XNCALLER_SYSAPP)
	#define XNetRandom(p,c)						NetDll_XNetRandom(XNCALLER_SYSAPP, p, c)
	#define XNetCreateKey(k,p)					NetDll_XNetCreateKey(XNCALLER_SYSAPP, k, p)
	#define XNetRegisterKey(k,p)				NetDll_XNetRegisterKey(XNCALLER_SYSAPP, k, p)
	#define XNetUnregisterKey(k)				NetDll_XNetUnregisterKey(XNCALLER_SYSAPP, k)
	#define XNetServerToInAddr(i,s,p)			NetDll_XNetServerToInAddr(XNCALLER_SYSAPP, i, s, p)
	#define XNetTsAddrToInAddr(t,i,s,p)			NetDll_XNetTsAddrToInAddr(XNCALLER_SYSAPP, t, i, s, p)
	#define XNetInAddrToServer(i,o)				NetDll_XNetInAddrToServer(XNCALLER_SYSAPP, i, o)
	#define XNetInAddrToString(i,p,c)			NetDll_XNetInAddrToString(XNCALLER_SYSAPP, i, p, c)
	#define XNetUnregisterInAddr(i)				NetDll_XNetUnregisterInAddr(XNCALLER_SYSAPP, i)
	#define XNetXnAddrToMachineId(x,i)			NetDll_XNetXnAddrToMachineId(XNCALLER_SYSAPP, x, i)
	#define XNetGetConnectStatus(i)				NetDll_XNetGetConnectStatus(XNCALLER_SYSAPP, i)
	#define XNetQosListen(k,p,c,b,f)			NetDll_XNetQosListen(XNCALLER_SYSAPP, k, p, c, b, f)
	#define XNetQosLookup(c,x,k,y,i,a,s,b,f,h,q)		NetDll_XNetQosLookup(XNCALLER_SYSAPP, c,x,k,y,i,a,s,b,f,h,q)
	#define XNetQosServiceLookup(f,h,q)			NetDll_XNetQosServiceLookup(XNCALLER_SYSAPP,f,h,q)
	#define XNetQosRelease(q)					NetDll_XNetQosRelease(XNCALLER_SYSAPP,q)
	#define XNetGetTitleXnAddr(x)				NetDll_XNetGetTitleXnAddr(XNCALLER_SYSAPP,x)
	#define XNetGetDebugXnAddr(x)				NetDll_XNetGetDebugXnAddr(XNCALLER_SYSAPP,x)
	#define XNetGetBroadcastVersionStatus(r)	NetDll_XNetGetBroadcastVersionStatus(XNCALLER_SYSAPP,r)
	#define XNetQosGetListenStats(k,l)			NetDll_XNetQosGetListenStats(XNCALLER_SYSAPP,k,l)
	#define XNetGetOpt(i,v,c)					NetDll_XNetGetOpt(XNCALLER_SYSAPP,i,v,c)
	#define XNetSetOpt(i,v,c)					NetDll_XNetSetOpt(XNCALLER_SYSAPP,i,v,c)
	

#endif // _USE_SYS_SOCKET_REPLACEMENT


#endif	//_SYSTEM_SOCKET_H

///* new ones */

//NetDll_WSAGetLastError // 0x1B
//NetDll___WSAFDIsSet // 0x22
//NetDll_XNetStartup // 0x33
//NetDll_XNetRandom // 0x35
//NetDll_XNetCreateKey // 0x36
//NetDll_XNetRegisterKey // 0x37
//NetDll_XNetUnregisterKey // 0x38
//NetDll_XNetXnAddrToInAddr // 0x39
//NetDll_XNetServerToInAddr // 0x3A
//NetDll_XNetInAddrToXnAddr // 0x3C
//NetDll_XNetUnregisterInAddr // 0x3F
//NetDll_XNetXnAddrToMachineId // 0x40
//NetDll_XNetConnect // 0x41
//NetDll_XNetGetConnectStatus // 0x42
//NetDll_XNetQosListen // 0x45
//NetDll_XNetQosLookup // 0x46
//NetDll_XNetQosServiceLookup // 0x47
//NetDll_XNetQosRelease // 0x48
//NetDll_XNetGetTitleXnAddr // 0x49
//NetDll_XNetReplaceKey // 0x51

