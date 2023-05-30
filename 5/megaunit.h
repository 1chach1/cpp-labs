//---------------------------------------------------------------------------

#ifndef megaunitH
#define megaunitH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include "VirtualTrees.hpp"
#include <windows.h>
#include "SafeQueue.hpp"
#include <wchar.h>
#include <string>
#include "ReadThread.h"
#include "WriteThread.h"
using namespace std;
//---------------------------------------------------------------------------
typedef struct
{
	int colId;
	ULONGLONG colCluster;
} VSTData;
//---------------------------------------------------------------------------
 class FileSystem
 {
 public:
	DWORD bytesPerCluster, clustersTotal;
	LARGE_INTEGER fileOffset;
	BYTE bootSector[512];
	FileSystem(BYTE bootSectorBuffer[512])
	{
		copy(bootSectorBuffer, bootSectorBuffer + 512, bootSector);
	};
	virtual void getFS() = 0;
	virtual ~FileSystem() { };
 };

 class ExFAT: public FileSystem
 {
 public:
	ExFAT(BYTE bootSectorBuffer[512]) : FileSystem(bootSectorBuffer)
	{
		getFS();
	};
	void getFS()
	{
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

		ExFAT_BootRecord *bRecord;
		bRecord = (ExFAT_BootRecord*)(bootSector);
		DWORD bytesPerSector = ((DWORD)0x01 << *bRecord->BytesPerSectorShift);
		int sectorsPerCluster = (0x01 << *bRecord->SectorsPerClusterShift);
		bytesPerCluster = bytesPerSector * sectorsPerCluster;
		clustersTotal = *bRecord->ClusterCount;
		ULONGLONG clusterOffset = bytesPerSector * *bRecord->ClusterHeapOffset - (DWORD)2 * bytesPerCluster;
		fileOffset.QuadPart = clusterOffset;
	};
 };

 class NTFS: public FileSystem
 {
 public:
	NTFS(BYTE bootSectorBuffer[512]) : FileSystem(bootSectorBuffer)
	{
		getFS();
	};
	void getFS()
	{
		#pragma pack(push, 1)
		typedef struct
		{
			BYTE JumpBoot[3];
			ULONGLONG FileSystemName[1];
			WORD BytesPerSector;
			BYTE SectorsPerCluster;
			WORD NotUsed[13];
			ULONGLONG TotalSectors[1];
		} NTFS_BootRecord;
		#pragma pack(pop)

		NTFS_BootRecord *bRecord;
		bRecord = (NTFS_BootRecord*)(bootSector);
		bytesPerCluster = bRecord->BytesPerSector * bRecord->SectorsPerCluster;
		clustersTotal = *bRecord->TotalSectors / bRecord->SectorsPerCluster;

		fileOffset.QuadPart = 0;
	};
	~NTFS() { };
 };

class ClientFS
{
private:
	FileSystem *FS;
public:
	DWORD bytesPerCluster, clustersTotal;
	LARGE_INTEGER fileOffset;

	ClientFS(BYTE bootSectorBuffer[512])
	{
        BYTE nameExFAT[8] = {0x45, 0x58, 0x46, 0x41, 0x54, 0x20, 0x20, 0x20};
		BYTE nameNTFS[8] = {0x4E, 0x54, 0x46, 0x53, 0x20, 0x20, 0x20, 0x20};

        if(memcmp(nameExFAT, bootSectorBuffer + 3, sizeof(nameExFAT)) == 0)
		{
			FS = new ExFAT(bootSectorBuffer);
			bytesPerCluster = FS->bytesPerCluster;
			clustersTotal = FS->clustersTotal;
			fileOffset = FS->fileOffset;
		}
		else if(memcmp(nameNTFS, bootSectorBuffer + 3, sizeof(nameNTFS)) == 0)
		{
			FS = new NTFS(bootSectorBuffer);
			bytesPerCluster = FS->bytesPerCluster;
			clustersTotal = FS->clustersTotal;
			fileOffset = FS->fileOffset;
		}
		else
		{
			FS = NULL;
			throw("Unsupported file system! NTFS and exFAT only.");
		}

	};

	FileSystem *returnFS() { return FS; };

	~ClientFS()
	{
		delete FS;
	};
};

class ClusterIterator
{
public:
	int index;
	DWORD bpc;
	BYTE *cluster, *buff;
	ULONGLONG buffS;
	ClusterIterator(BYTE &buffer, ULONGLONG buffSize, DWORD bytesPerCluster)
	{
		cluster = new BYTE[bytesPerCluster];
		index = 0;
		bpc = bytesPerCluster;
		buffS = buffSize;
		//buff = (BYTE*)(buffer);
		//buff = new BYTE[buffS];
		buff = &buffer;
		memcpy(buff, &buffer, buffS);
	};
	virtual void First()
	{
		index = bpc;
		memcpy(cluster, buff, bpc);

	};
	virtual void Next()
	{
		index += bpc;
		if ((buff[0] + index) <= buff[buffS])
		{
			memcpy(cluster,buff + index, bpc);
		}

	};
	virtual BYTE *Current()
	{
		return cluster;
	};
	virtual bool IsDone()
	{
		if (index > buffS)
		{
			return true;
		}
		else
		{
			return false;
		}
	};
	virtual ~ClusterIterator()
	{
		delete[] cluster;
		//delete[] buff;
	};

};

class ClusterIteratorDecorator : public ClusterIterator
{
public:

		BYTE pngMagicNumber[8] = {0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a};
		ClusterIteratorDecorator(BYTE buffer, ULONGLONG buffSize, DWORD bytesPerCluster)
		: ClusterIterator(buffer, buffSize, bytesPerCluster) { };

	virtual void First()
	{
		for (ClusterIterator::First(); !ClusterIterator::IsDone(); ClusterIterator::Next())
		{
			cluster = ClusterIterator::Current();
			if(memcmp(cluster, pngMagicNumber, 8) == 0)
			{
				break;
			}
		}
	};
	virtual void Next()
	{
		for (; !ClusterIterator::IsDone(); ClusterIterator::Next())
		{
			cluster = ClusterIterator::Current();
			if(memcmp(cluster, pngMagicNumber, 8) == 0)
			{
				break;
			}
		}
	};
	virtual BYTE *Current()
	{
		return cluster;
	};
	virtual bool IsDone()
	{
		ClusterIterator::IsDone();
	};
	virtual ~ClusterIteratorDecorator()
	{
		delete[] cluster;
	};

};

//---------------------------------------------------------------------------
class TForm1 : public TForm
{
__published:	// IDE-managed Components
	TVirtualStringTree *VirtualStringTree1;
	TButton *ButtonStart;
	TButton *ButtonStop;
	TComboBox *ComboBoxVolume;
	void __fastcall ComboBoxVolumeSelect(TObject *Sender);
	void __fastcall ButtonStartClick(TObject *Sender);
	void __fastcall VirtualStringTree1GetText(TBaseVirtualTree *Sender, PVirtualNode Node,
          TColumnIndex Column, TVSTTextType TextType, UnicodeString &CellText);
	void __fastcall ButtonStopClick(TObject *Sender);
	void __fastcall FormClick(TObject *Sender);

private:	// User declarations
public:		// User declarations
	__fastcall TForm1(TComponent* Owner);
	DWORD bytesPerCluster, clustersTotal;
	LARGE_INTEGER fileOffset;
	HANDLE fileHandle;
	bool flagStop;
	SafeQueue<ULONGLONG> queue;
	TReadThread *readThread;
	TWriteThread *writeThread;


};
//---------------------------------------------------------------------------
extern PACKAGE TForm1 *Form1;
//---------------------------------------------------------------------------
#endif
