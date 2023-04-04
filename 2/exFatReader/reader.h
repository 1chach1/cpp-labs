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
	BYTE FileSystemName[8];
	BYTE MustBeZero[53];
	BYTE PartitionOffset[8];
	BYTE VolumeLength[8];
	BYTE FatOffset[4];
	BYTE FatLength[4];
	BYTE ClusterHeapOffset[4];
	BYTE ClusterCount[4];
	BYTE FirstClusterOfRootDirectory[4];
	BYTE VolumeSerialNumber[4];
	BYTE FileSystemRevision[2];
	BYTE VolumeFlags[2];
	BYTE BytesPerSectorShift[1];
	BYTE SectorsPerClusterShift[1];
	BYTE NumberOfFats[1];
	BYTE DriveSelect[1];
	BYTE PercentInUse[1];
	BYTE Reserved[7];
	BYTE BootCode[390];
	BYTE BootSignature[2];

} ExFAT_BootRecord;

#pragma pack(pop)



