//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "sqliteviewer.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "VirtualTrees"
#pragma resource "*.dfm"

using namespace std;

TmainForm *mainForm;
wstring filePath;
int resultOpen;
sqlite3* Database;
char *errmsg;
sqlite3_stmt *pStatement;
wstring sqlSelect = L"SELECT id, url, title, last_visit_time FROM urls;";
typedef struct
{
	int colId;
	UnicodeString colURL;
	UnicodeString colName;
	__int64 colTime;
} VSTData;

VSTData *tData;

//---------------------------------------------------------------------------
__fastcall TmainForm::TmainForm(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------

void __fastcall TmainForm::OpenButtonClick(TObject *Sender)
{
	if (OpenDialog1->Execute())
	{
		if (FileExists(OpenDialog1->FileName))
		{
			pathLabel->Caption = OpenDialog1->FileName;
			filePath = OpenDialog1->FileName;
		}
		else
		{
			throw(Exception("File does not exist."));
		}
	}

	if (sqlite3_open16(filePath.c_str(), &Database) != SQLITE_OK)
	{
		throw(Exception("Could not open the database."));
	}

	int result = sqlite3_prepare16_v2(Database, sqlSelect.c_str(),-1,&pStatement,NULL);

	if(result != SQLITE_OK)
	{
		throw(Exception(sqlite3_errmsg(Database)));
	}


	VirtualStringTree1->BeginUpdate();

	while((result = sqlite3_step(pStatement)) == SQLITE_ROW)
	{
		if(result != SQLITE_ROW)
		{
			throw(Exception(sqlite3_errmsg(Database)));
			break;
		}

		PVirtualNode Node = VirtualStringTree1->AddChild(NULL);
		VSTData *dbData = (VSTData*)VirtualStringTree1->GetNodeData(Node);

		dbData->colId = sqlite3_column_int(pStatement,0);
		dbData->colURL = (wchar_t*)sqlite3_column_text16(pStatement, 1);
		dbData->colName = (wchar_t*)sqlite3_column_text16(pStatement, 2);
		dbData->colTime = sqlite3_column_int64(pStatement, 3);
	}

	VirtualStringTree1->EndUpdate();

	sqlite3_finalize(pStatement);

	OpenButton->Enabled = False;
}
//---------------------------------------------------------------------------
void __fastcall TmainForm::FormClose(TObject *Sender, TCloseAction &Action)
{
	sqlite3_close(Database);
}
//---------------------------------------------------------------------------

void __fastcall TmainForm::FormCreate(TObject *Sender)
{
	VirtualStringTree1->NodeDataSize = sizeof(VSTData);	
}
//---------------------------------------------------------------------------

void __fastcall TmainForm::VirtualStringTree1GetText(TBaseVirtualTree *Sender, PVirtualNode Node,
		  TColumnIndex Column, TVSTTextType TextType, UnicodeString &CellText)
{
	VSTData *dbData = (VSTData*)Sender->GetNodeData(Node);
		switch (Column)
		{
			case 0:
				CellText = dbData->colURL;
				break;
			case 1:
				CellText = dbData->colName;
				break;
		}	
}
//---------------------------------------------------------------------------

void __fastcall TmainForm::VirtualStringTree1AddToSelection(TBaseVirtualTree *Sender,
          PVirtualNode Node)
{
	tData = (VSTData*)Sender->GetNodeData(Node);
	Edit1->Text = tData->colTime;
}
//---------------------------------------------------------------------------

void __fastcall TmainForm::DeleteButtonClick(TObject *Sender)
{
	VirtualStringTree1->BeginUpdate();

	PVirtualNode Node = VirtualStringTree1->GetFirstSelected();
	tData = (VSTData*)VirtualStringTree1->GetNodeData(Node);
	string sqlDelete = "DELETE FROM urls WHERE id = " + to_string(tData->colId) + ";";
	sqlite3_exec(Database,sqlDelete.c_str(),NULL,NULL,&errmsg);

	Node = VirtualStringTree1->GetNextSelected(Node);

	VirtualStringTree1->DeleteSelectedNodes();

	VirtualStringTree1->EndUpdate();

	Edit1->Text = "";
}
//---------------------------------------------------------------------------

void __fastcall TmainForm::ClearButtonClick(TObject *Sender)
{
	sqlite3_exec(Database,"DELETE FROM urls;",NULL,NULL,&errmsg);
	VirtualStringTree1->Clear();
	Edit1->Text = "";
}
//---------------------------------------------------------------------------

