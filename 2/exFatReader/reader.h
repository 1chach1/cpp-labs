#pragma once

#ifdef READER_EXPORTS
#define READER_API __declspec(dllexport)
#else
#define READER_API __declspec(dllimport)
#endif

#pragma pack(push, 1)

typedef struct
{
	BYTE JumpBoot[3];
	ULONGLONG FileSystemName[1];
	BYTE MustBeZero[53];
	ULONGLONG PartitionOffset[1];
	ULONGLONG VolumeLength[1];
	DWORD FatOffset[1];
	DWORD FatLength[1];
	DWORD ClusterHeapOffset[1];
	DWORD ClusterCount[1];
	DWORD FirstClusterOfRootDirectory[1];
	DWORD VolumeSerialNumber[1];
	BYTE FileSystemRevision[2];
	WORD VolumeFlags[1];
	BYTE BytesPerSectorShift[1];
	BYTE SectorsPerClusterShift[1];
	BYTE NumberOfFats[1];
	BYTE DriveSelect[1];
	BYTE PercentInUse[1];
	BYTE Reserved[7];
	BYTE BootCode[390];
	WORD BootSignature[1];

} ExFAT_BootRecord;

#pragma pack(pop)



