//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "megaunit.h"
#include "ReadThread.h"
#include "WriteThread.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "VirtualTrees"
#pragma resource "*.dfm"
TForm1 *Form1;

wstring volName, volPath;
//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner)
	: TForm(Owner)
{
	VirtualStringTree1->NodeDataSize = sizeof(VSTData);
	DWORD vols = GetLogicalDrives();
	for (int i = 0; i < 26; i++)
	{
		if ((vols & ( 1 << i)))
		{
			ComboBoxVolume->AddItem((char)(TEXT('A') + i), NULL);
		}
	}
}
//---------------------------------------------------------------------------

void __fastcall TForm1::ComboBoxVolumeSelect(TObject *Sender)
{
	volName = ComboBoxVolume->Items->Strings[ComboBoxVolume->ItemIndex];
	volPath = L"\\\\.\\" + volName + L":";
}
//---------------------------------------------------------------------------

void __fastcall TForm1::ButtonStartClick(TObject *Sender)
{
	flagStop = false;
	VirtualStringTree1->Clear();

	fileHandle = CreateFileW(
		volPath.c_str(),
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
	);

    if (fileHandle == INVALID_HANDLE_VALUE)
    {
		throw(Exception(GetLastError()));
	}

	BYTE bootSectorBuffer[512];
    DWORD bytesToRead = 512;
	DWORD bytesRead;


	bool readResult = ReadFile(
		fileHandle,
		bootSectorBuffer,
		bytesToRead,
		&bytesRead,
		NULL
	);
	if (!readResult || bytesRead != bytesToRead)
	{
		throw(Exception(GetLastError()));
	}

	ClientFS *pClientFS = new ClientFS(bootSectorBuffer);

	bytesPerCluster = pClientFS->bytesPerCluster;
	clustersTotal = pClientFS->clustersTotal;
	fileOffset = pClientFS->fileOffset;

	readThread = new TReadThread(false, volPath);
	writeThread = new TWriteThread(false);

	delete pClientFS;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::VirtualStringTree1GetText(TBaseVirtualTree *Sender, PVirtualNode Node,
          TColumnIndex Column, TVSTTextType TextType, UnicodeString &CellText)

{
	VSTData *data = (VSTData*)Sender->GetNodeData(Node);
		switch (Column)
		{
			case 0:
				CellText = data->colId;
				break;
			case 1:
				CellText = data->colCluster;
				break;
		}
}
//---------------------------------------------------------------------------


void __fastcall TForm1::ButtonStopClick(TObject *Sender)
{
	flagStop = true;
	ShowMessage("Process stoped.");
}
//---------------------------------------------------------------------------

void __fastcall TForm1::FormClick(TObject *Sender)
{
	CloseHandle(fileHandle);
}
//---------------------------------------------------------------------------

