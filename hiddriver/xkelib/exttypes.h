#ifndef __EXTTYPES_H
#define __EXTTYPES_H

typedef unsigned char		u8;
typedef unsigned short		u16;
typedef unsigned int		u32;
typedef unsigned __int64	u64;
typedef unsigned __int64	QWORD;

typedef signed char			s8;
typedef signed short		s16;
typedef signed int			s32;
typedef signed __int64		s64;

typedef QWORD near           *PQWORD;
typedef QWORD far            *LPQWORD;

#define CONSTANT_OBJECT_STRING(s)   { strlen( s ) / sizeof( OCHAR ), (strlen( s ) / sizeof( OCHAR ))+1, s }
#define MAKE_STRING(s)   {(USHORT)(strlen(s)), (USHORT)((strlen(s))+1), (PCHAR)s}
#define EXPORTNUM(x) // Just for documentation, thx XBMC!

#define STATUS_SUCCESS	0
#define NT_EXTRACT_ST(Status)			((((ULONG)(Status)) >> 30)& 0x3)
#define NT_SUCCESS(Status)              (((NTSTATUS)(Status)) >= 0)
#define NT_INFORMATION(Status)          (NT_EXTRACT_ST(Status) == 1)
#define NT_WARNING(Status)              (NT_EXTRACT_ST(Status) == 2)
#define NT_ERROR(Status)                (NT_EXTRACT_ST(Status) == 3)

#define NEG_ONE_AS_DWORD				((DWORD)-1)

typedef long			NTSTATUS;
typedef ULONG			ACCESS_MASK;

typedef struct _STRING {
    USHORT Length;
    USHORT MaximumLength;
    PCHAR Buffer;
} STRING, *PSTRING;

typedef struct _CSTRING {
	USHORT Length;
	USHORT MaximumLength;
	CONST char *Buffer;
} CSTRING, *PCSTRING;

typedef struct _UNICODE_STRING {
	USHORT Length;
	USHORT MaximumLength;
	PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

typedef STRING			OBJECT_STRING;
typedef CSTRING			COBJECT_STRING;
typedef PSTRING			POBJECT_STRING;
typedef PCSTRING		PCOBJECT_STRING;
typedef STRING			OEM_STRING;
typedef PSTRING			POEM_STRING;
typedef CHAR			OCHAR;
typedef CHAR*			POCHAR;
typedef PSTR			POSTR;
typedef PCSTR			PCOSTR;
typedef CHAR*			PSZ;
typedef CONST CHAR*		PCSZ;
typedef STRING			ANSI_STRING;
typedef PSTRING			PANSI_STRING;
typedef CSTRING			CANSI_STRING;
typedef PCSTRING		PCANSI_STRING;
#define ANSI_NULL		((CHAR)0)     // winnt
typedef CONST UNICODE_STRING*	PCUNICODE_STRING;
#define UNICODE_NULL	((WCHAR)0) // winnt

#define OTEXT(quote) __OTEXT(quote)

typedef struct _IO_STATUS_BLOCK {
	union {
		NTSTATUS Status;
		PVOID Pointer;
	} st;
	ULONG_PTR Information;
} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;

typedef VOID (NTAPI *PIO_APC_ROUTINE) (
	IN PVOID ApcContext,
	IN PIO_STATUS_BLOCK IoStatusBlock,
	IN ULONG Reserved
	);

typedef struct _OBJECT_ATTRIBUTES {
	HANDLE RootDirectory;
	POBJECT_STRING ObjectName;
	ULONG Attributes;
} OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES;

// VOID InitializeObjectAttributes(
//     OUT POBJECT_ATTRIBUTES p,
//     IN STRING n,
//     IN ULONG a,
//     IN HANDLE r)
#define InitializeObjectAttributes( p, name, attrib, root){		\
	(p)->RootDirectory = root;                            \
	(p)->Attributes = attrib;                             \
	(p)->ObjectName = name;                               \
}

#ifndef NULL
#define NULL	0
#endif // NULL


#endif // __EXTTYPES_H


