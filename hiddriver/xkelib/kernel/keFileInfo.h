#ifndef __KEFILEINFO_H
#define __KEFILEINFO_H

#define INVALID_FILE_ATTRIBUTES -1

typedef enum _FILE_INFORMATION_CLASS
{
	FileDirectoryInformation = 0x1,
	FileFullDirectoryInformation = 0x2,
	FileBothDirectoryInformation = 0x3,
	FileBasicInformation = 0x4, // FILE_BASIC_INFORMATION below
	FileStandardInformation = 0x5,
	FileInternalInformation = 0x6,
	FileEaInformation = 0x7,
	FileAccessInformation = 0x8,
	FileNameInformation = 0x9,
	FileRenameInformation = 0xa,
	FileLinkInformation = 0xb,
	FileNamesInformation = 0xc,
	FileDispositionInformation = 0xd, // use sdk FILE_DISPOSITION_INFO
	FilePositionInformation = 0xe, // FILE_POSITION_INFORMATION below
	FileFullEaInformation = 0xf,
	FileModeInformation = 0x10,
	FileAlignmentInformation = 0x11,
	FileAllInformation = 0x12,
	FileAllocationInformation = 0x13, // use sdk FILE_ALLOCATION_INFO
	FileEndOfFileInformation = 0x14, // use sdk FILE_END_OF_FILE_INFO
	FileAlternateNameInformation = 0x15,
	FileStreamInformation = 0x16,
	FileMountPartitionInformation = 0x17,
	FileMountPartitionsInformation = 0x18,
	FilePipeRemoteInformation = 0x19,
	FileSectorInformation = 0x1a,
	FileXctdCompressionInformation = 0x1b,
	FileCompressionInformation = 0x1c,
	FileObjectIdInformation = 0x1d,
	FileCompletionInformation = 0x1e,
	FileMoveClusterInformation = 0x1f,
	FileIoPriorityInformation = 0x20,
	FileReparsePointInformation = 0x21,
	FileNetworkOpenInformation = 0x22,
	FileAttributeTagInformation = 0x23,
	FileTrackingInformation = 0x24,
	FileMaximumInformation = 0x25
} FILE_INFORMATION_CLASS;

typedef enum _FSINFOCLASS {
	FileFsVolumeInformation = 0x1,
	FileFsLabelInformation = 0x2,
	FileFsSizeInformation = 0x3,
	FileFsDeviceInformation = 0x4,
	FileFsAttributeInformation = 0x5,
	FileFsControlInformation = 0x6,
	FileFsFullSizeInformation = 0x7,
	FileFsObjectIdInformation = 0x8,
	FileFsMaximumInformation = 0x9,
} FSINFOCLASS;

//  *******   structs for the data passed with the above classes when using NtSetInformationFile/NtQueryInformationFile

// class FilePositionInformation = 0xd
typedef struct FILE_POSITION_INFORMATION { // query/set
	LARGE_INTEGER  CurrentByteOffset;
} FILE_POSITION_INFORMATION, *PFILE_POSITION_INFORMATION;

// class FileBasicInformation = 0x4
typedef struct _FILE_BASIC_INFORMATION {
	LARGE_INTEGER CreationTime;
	LARGE_INTEGER LastAccessTime;
	LARGE_INTEGER LastWriteTime;
	LARGE_INTEGER ChangeTime;
	DWORD FileAttributes;
} FILE_BASIC_INFORMATION, *PFILE_BASIC_INFORMATION;

// returned by a call to 'NtQueryInformationFile' with 0x22 = FileNetworkOpenInformation
typedef struct _FILE_NETWORK_OPEN_INFORMATION {
	LARGE_INTEGER  CreationTime;
	LARGE_INTEGER  LastAccessTime;
	LARGE_INTEGER  LastWriteTime;
	LARGE_INTEGER  ChangeTime;
	LARGE_INTEGER  AllocationSize;
	LARGE_INTEGER  EndOfFile;
	DWORD  FileAttributes;
} FILE_NETWORK_OPEN_INFORMATION, *PFILE_NETWORK_OPEN_INFORMATION;

//  *******   structs related to FSINFOCLASS for use with NtQueryVolumeInformationFile

// class FileFsVolumeInformation = 0x1
typedef struct _FILE_FS_VOLUME_INFORMATION { 
	LARGE_INTEGER VolumeCreationTime; // 0x0 sz:0x8
	DWORD VolumeSerialNumber; // 0x8 sz:0x4
	DWORD VolumeLabelLength; // 0xC sz:0x4
	BYTE SupportsObjects; // 0x10 sz:0x1
	BYTE VolumeLabel[1]; // 0x11 sz:0x1
} FILE_FS_VOLUME_INFORMATION, *PFILE_FS_VOLUME_INFORMATION; // size 24
C_ASSERT(sizeof(FILE_FS_VOLUME_INFORMATION) == 0x18);

// class FileFsLabelInformation = 0x2
typedef struct _FILE_FS_LABEL_INFORMATION { 
	DWORD VolumeLabelLength; // 0x0 sz:0x4
	BYTE VolumeLabel[1]; // 0x4 sz:0x1
} FILE_FS_LABEL_INFORMATION, *PFILE_FS_LABEL_INFORMATION; // size 8
C_ASSERT(sizeof(FILE_FS_LABEL_INFORMATION) == 0x8);

// class FileFsSizeInformation = 0x3
typedef struct _FILE_FS_SIZE_INFORMATION { 
	LARGE_INTEGER TotalAllocationUnits; // 0x0 sz:0x8
	LARGE_INTEGER AvailableAllocationUnits; // 0x8 sz:0x8
	DWORD SectorsPerAllocationUnit; // 0x10 sz:0x4
	DWORD BytesPerSector; // 0x14 sz:0x4
} FILE_FS_SIZE_INFORMATION, *PFILE_FS_SIZE_INFORMATION; // size 24
C_ASSERT(sizeof(FILE_FS_SIZE_INFORMATION) == 0x18);

// class FileFsDeviceInformation = 0x4
typedef struct _FILE_FS_DEVICE_INFORMATION { 
	DWORD DeviceType; // 0x0 sz:0x4
	DWORD Characteristics; // 0x4 sz:0x4
} FILE_FS_DEVICE_INFORMATION, *PFILE_FS_DEVICE_INFORMATION; // size 8
C_ASSERT(sizeof(FILE_FS_DEVICE_INFORMATION) == 0x8);

// class FileFsAttributeInformation = 0x5
typedef struct _FILE_FS_ATTRIBUTE_INFORMATION { 
	DWORD FileSystemAttributes; // 0x0 sz:0x4
	DWORD MaximumComponentNameLength; // 0x4 sz:0x4
	DWORD FileSystemNameLength; // 0x8 sz:0x4
	BYTE FileSystemName[1]; // 0xC sz:0x1
} FILE_FS_ATTRIBUTE_INFORMATION, *PFILE_FS_ATTRIBUTE_INFORMATION; // size 16
C_ASSERT(sizeof(FILE_FS_ATTRIBUTE_INFORMATION) == 0x10);

// class FileFsControlInformation = 0x6
typedef struct _FILE_FS_CONTROL_INFORMATION { 
	LARGE_INTEGER FreeSpaceStartFiltering; // 0x0 sz:0x8
	LARGE_INTEGER FreeSpaceThreshold; // 0x8 sz:0x8
	LARGE_INTEGER FreeSpaceStopFiltering; // 0x10 sz:0x8
	LARGE_INTEGER DefaultQuotaThreshold; // 0x18 sz:0x8
	LARGE_INTEGER DefaultQuotaLimit; // 0x20 sz:0x8
	DWORD FileSystemControlFlags; // 0x28 sz:0x4
} FILE_FS_CONTROL_INFORMATION, *PFILE_FS_CONTROL_INFORMATION; // size 48
C_ASSERT(sizeof(FILE_FS_CONTROL_INFORMATION) == 0x30);

// class FileFsFullSizeInformation = 0x7
typedef struct _FILE_FS_FULL_SIZE_INFORMATION { 
	LARGE_INTEGER TotalAllocationUnits; // 0x0 sz:0x8
	LARGE_INTEGER CallerAvailableAllocationUnits; // 0x8 sz:0x8
	LARGE_INTEGER ActualAvailableAllocationUnits; // 0x10 sz:0x8
	DWORD SectorsPerAllocationUnit; // 0x18 sz:0x4
	DWORD BytesPerSector; // 0x1C sz:0x4
} FILE_FS_FULL_SIZE_INFORMATION, *PFILE_FS_FULL_SIZE_INFORMATION; // size 32
C_ASSERT(sizeof(FILE_FS_FULL_SIZE_INFORMATION) == 0x20);

// class FileFsObjectIdInformation = 0x8
typedef struct _FILE_FS_OBJECTID_INFORMATION { 
	BYTE ObjectId[0x10]; // 0x0 sz:0x10
	BYTE ExtendedInfo[0x30]; // 0x10 sz:0x30
} FILE_FS_OBJECTID_INFORMATION, *PFILE_FS_OBJECTID_INFORMATION; // size 64
C_ASSERT(sizeof(FILE_FS_OBJECTID_INFORMATION) == 0x40);


#endif // __KEFILEINFO_H

