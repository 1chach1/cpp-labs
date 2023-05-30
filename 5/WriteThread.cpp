//---------------------------------------------------------------------------

#include <System.hpp>
#pragma hdrstop

#include "WriteThread.h"
#include "megaunit.h"
#include "ReadThread.h"
#pragma package(smart_init)
//---------------------------------------------------------------------------

//   Important: Methods and properties of objects in VCL can only be
//   used in a method called using Synchronize, for example:
//
//      Synchronize(&UpdateCaption);
//
//   where UpdateCaption could look like:
//
//      void __fastcall TWriteThread::UpdateCaption()
//      {
//        Form1->Caption = "Updated in a thread";
//      }
//---------------------------------------------------------------------------

__fastcall TWriteThread::TWriteThread(bool CreateSuspended)
	: TThread(CreateSuspended)
{
	if (sqlite3_open16(L"clusters.db", &Database) != SQLITE_OK)
	{
		throw(Exception(sqlite3_errmsg(Database)));
		Terminate();
	}

	wstring sqlSelect = L"CREATE TABLE IF NOT EXISTS clusters\
						(id integer primary key autoincrement,\
						cluster integer);";

	int result = sqlite3_prepare16_v2(Database, sqlSelect.c_str(),-1,&pStatement,NULL);
	if(result != SQLITE_OK)
	{
		throw(Exception(sqlite3_errmsg(Database)));
		Terminate();
	}

	result = sqlite3_step(pStatement);
    if (result != SQLITE_DONE)
	{
		throw(Exception(sqlite3_errmsg(Database)));
		Terminate();
	}

	sqlite3_exec(Database,"DELETE FROM clusters", NULL, NULL, &errmsg);
	sqlite3_exec(Database,"DELETE FROM sqlite_sequence where name='clusters'", NULL, NULL, &errmsg);
}
//---------------------------------------------------------------------------
void __fastcall TWriteThread::Execute()
{
	NameThreadForDebugging(System::String(L"WriteThread"));
	//---- Place thread code here ----
	FreeOnTerminate = true;
	wstring sqlInsert = L"INSERT INTO clusters(cluster) values(?);";
	ULONGLONG clusterIndex = NULL;
	int id = 0;

	int result = sqlite3_prepare16_v2(Database, sqlInsert.c_str(),-1,&pStatement,NULL);

	if(result != SQLITE_OK)
	{
		throw(Exception(sqlite3_errmsg(Database)));
		Terminate();
	}

	while (Form1->queue.Consume(clusterIndex) && !Form1->readThread->Finished)
	{
        if (Form1->flagStop)
		{
			Free();
		}

		if (clusterIndex != NULL)
		{
			sqlite3_bind_int(pStatement, 1, clusterIndex);
			sqlite3_step(pStatement);
			sqlite3_reset(pStatement);
			Synchronize(&Update);
		}
	}

}
//---------------------------------------------------------------------------
void __fastcall TWriteThread::Update()
{
	Form1->VirtualStringTree1->BeginUpdate();
	PVirtualNode Node = Form1->VirtualStringTree1->AddChild(NULL);
	VSTData* data = (VSTData*)Form1->VirtualStringTree1->GetNodeData(Node);
	//data->colId = ID;
	//data->colCluster = NCluster;
	Form1->VirtualStringTree1->EndUpdate();
}
//---------------------------------------------------------------------------
__fastcall TWriteThread::~TWriteThread()
{
	sqlite3_finalize(pStatement);
	sqlite3_close(Database);
}
