//---------------------------------------------------------------------------

#ifndef ReadThreadH
#define ReadThreadH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <string>
//---------------------------------------------------------------------------
using namespace std;

class TReadThread : public TThread
{
protected:
	void __fastcall Execute();
public:
	__fastcall TReadThread(bool CreateSuspended, wstring volPath);

};
//---------------------------------------------------------------------------
#endif
