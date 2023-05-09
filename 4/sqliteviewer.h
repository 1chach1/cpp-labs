//---------------------------------------------------------------------------

#ifndef sqliteviewerH
#define sqliteviewerH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include "VirtualTrees.hpp"
#include <Vcl.Dialogs.hpp>
#include "sqlite3.h"
#include <string>
#include <wchar.h>
//---------------------------------------------------------------------------
class TmainForm : public TForm
{
__published:	// IDE-managed Components
	TVirtualStringTree *VirtualStringTree1;
	TOpenDialog *OpenDialog1;
	TButton *OpenButton;
	TButton *DeleteButton;
	TButton *ClearButton;
	TLabel *pathLabel;
	TEdit *Edit1;
	TLabel *Label1;
	void __fastcall OpenButtonClick(TObject *Sender);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall FormCreate(TObject *Sender);
	void __fastcall VirtualStringTree1GetText(TBaseVirtualTree *Sender, PVirtualNode Node,
          TColumnIndex Column, TVSTTextType TextType, UnicodeString &CellText);
	void __fastcall VirtualStringTree1AddToSelection(TBaseVirtualTree *Sender, PVirtualNode Node);
	void __fastcall DeleteButtonClick(TObject *Sender);
	void __fastcall ClearButtonClick(TObject *Sender);






private:	// User declarations
public:		// User declarations
	__fastcall TmainForm(TComponent* Owner);

};
//---------------------------------------------------------------------------
extern PACKAGE TmainForm *mainForm;
//---------------------------------------------------------------------------
#endif
