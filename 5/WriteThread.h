//---------------------------------------------------------------------------

#ifndef WriteThreadH
#define WriteThreadH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include "sqlite3.h"
//---------------------------------------------------------------------------
using namespace std;

class TWriteThread : public TThread
{
protected:
	void __fastcall Execute();

public:
	__fastcall TWriteThread(bool CreateSuspended);
	__fastcall ~TWriteThread();
	void __fastcall Update();
	sqlite3* Database;
	char *errmsg;
	sqlite3_stmt *pStatement;
};
//---------------------------------------------------------------------------
#endif
