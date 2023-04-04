#include <iostream>
#include <fstream>
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
    
    if (*(bRecord->FileSystemName) != 2314885754712905797)
    {
        cout << "Error! No exFat volume found!";
        return 0;
    }

            
    DWORD bytesPerSector = ((DWORD)0x01 << *bRecord->BytesPerSectorShift);
    int sectorsPerCluster = (0x01 << *bRecord->SectorsPerClusterShift);
    DWORD bytesPerCluster = bytesPerSector * sectorsPerCluster;
    DWORD clustersTotal = *bRecord->ClusterCount;
    ULONGLONG clusterOffset = bytesPerSector * *bRecord->ClusterHeapOffset - (DWORD)2 * bytesPerCluster;
   
    BYTE *dataBuffer = new BYTE[bytesPerCluster];
    ULONGLONG clusterToRead;
   
    cout << "Enter cluster number you want to write to file (from 0 to " << clustersTotal << "): ";
    cin >> clusterToRead;

    LARGE_INTEGER fileOffset;
    fileOffset.QuadPart = clusterOffset + clusterToRead * bytesPerCluster;

    unsigned long currentPosition = SetFilePointer(
        fileHandle,
        fileOffset.LowPart,
        &fileOffset.HighPart,
        FILE_BEGIN
    );

    if (currentPosition != fileOffset.LowPart)
    {
        cout << "Error! Win32 error code: " << GetLastError();
    }

    readResult = ReadFile(
        fileHandle,
        dataBuffer,
        bytesPerCluster,
        &bytesRead,
        NULL
    );

    if (!readResult || bytesRead != bytesPerCluster)
    {
        cout << "Error! Win32 error code: " << GetLastError();
    }
   
    ofstream outputFile;
    outputFile.open("cluster");
    for (int i = 0; i < bytesPerCluster; i++) {
        outputFile << dataBuffer[i];
    }
        
    cout << "Successfully writen cluster to file \"cluster\"! Praise the Omnissiah!" << endl;

    outputFile.close();
    delete[] dataBuffer;
    CloseHandle(fileHandle);
    system("pause");
}
