//---------------------------------------------------------------------------

#include <System.hpp>
#pragma hdrstop

#include "ReadThread.h"
#include "megaunit.h"
#include "WriteThread.h"
#pragma package(smart_init)
//---------------------------------------------------------------------------

//   Important: Methods and properties of objects in VCL can only be
//   used in a method called using Synchronize, for example:
//
//      Synchronize(&UpdateCaption);
//
//   where UpdateCaption could look like:
//
//      void __fastcall TReadThread::UpdateCaption()
//      {
//        Form1->Caption = "Updated in a thread";
//      }
//---------------------------------------------------------------------------


__fastcall TReadThread::TReadThread(bool CreateSuspended, wstring volPath)
	: TThread(CreateSuspended)
{

}
//---------------------------------------------------------------------------
void __fastcall TReadThread::Execute()
{
	NameThreadForDebugging(System::String(L"ReadThread"));
	//---- Place thread code here ----
	FreeOnTerminate = true;

	SetFilePointer(
		Form1->fileHandle,
		Form1->fileOffset.LowPart,
		&Form1->fileOffset.HighPart,
        FILE_BEGIN
	);


	ULONGLONG bufferSize = Form1->bytesPerCluster * 512;
	BYTE *buffer = new BYTE[bufferSize];
	DWORD bytesRead;
	ULONGLONG clusterNumber;

	if (Form1->fileOffset.QuadPart == 0)
	{
		clusterNumber = 0;
	}
	else
	{
		clusterNumber = 2;
    }

	for (; clusterNumber < Form1->clustersTotal; clusterNumber += 512)
	{
		if (Form1->flagStop)
		{
			Free();
		}

		bool readResult = ReadFile(
			Form1->fileHandle,
			buffer,
			bufferSize,
			&bytesRead,
			NULL
			);
		if (!readResult || bytesRead != bufferSize)
		{
			throw(Exception(GetLastError()));
			Terminate();
		}

		ClusterIteratorDecorator *iter = new ClusterIteratorDecorator(*buffer, bufferSize, Form1->bytesPerCluster);

		for (iter->First(); iter->IsDone(); iter->Next())
		{
		  Form1->queue.Produce(move(clusterNumber));
		}
		delete iter;

	}
	delete[] buffer;
}
//---------------------------------------------------------------------------

