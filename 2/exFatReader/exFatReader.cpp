#include <iostream>
#include <windows.h>
#include <wchar.h>
#include "reader.h"

using namespace std;

int main()
{
    wstring volumeName;
    
    while (1)
    {
        cout << "Enter volume letter: ";
        wcin >> volumeName;
        if (volumeName.length() != 1)
        {
            cout << "Please enter volume letter only! (ex. \"C\")" << endl;
        }
        else
        {
            break;
        }
    }
    
    wstring volumeNameFomated = L"\\\\.\\" + volumeName + L":";
    
    const wchar_t* volume = volumeNameFomated.c_str();

    HANDLE fileHandle = CreateFileW(
        volume, 
        GENERIC_READ,	  
        FILE_SHARE_READ | FILE_SHARE_WRITE, 
        NULL, 
        OPEN_EXISTING, 
        FILE_ATTRIBUTE_NORMAL, 
        NULL 
    );

    if (fileHandle == INVALID_HANDLE_VALUE)
    {
        if (GetLastError() == 5)
        {
            cout << "Error! Run this program as administrator!";
        }
        else if (GetLastError() == 2)
        {
            cout << "Error! The system cannot find the file specified.";
        }
        else
        {
            cout << "Error! Win32 error code: " << GetLastError();
        }
        
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
        cout << "Error! Win32 error code: " << GetLastError();
    }

    ExFAT_BootRecord *bRecord;
    bRecord = (ExFAT_BootRecord*)(bootSectorBuffer);
    
    BYTE exFatName[8] = {69, 88, 70, 65, 84, 32, 32, 32};

    for (int i = 0; i < 8; i++)
    {
        if (*(bRecord->FileSystemName + i) != exFatName[i])
        {
            cout << "Error! No exFat volume found!";
            return 0;
        }
    } 
        
    int bytesPerSector = (0x01 << *bRecord->BytesPerSectorShift);
    int sectorsPerCluster = (0x01 << *bRecord->SectorsPerClusterShift);
    int bytesPerCluster = bytesPerSector * sectorsPerCluster;
    
















    CloseHandle(fileHandle);
}
