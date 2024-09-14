#include <Windows.h>
#include <TlHelp32.h>
#include <Shlwapi.h>
#include <conio.h>
#include <stdio.h>
#include <iostream>
#include <tchar.h>
#include <wchar.h>

void HideConsole();
void ShowConsole();
void Pause();

#define debug_print(fmt, ...) \
            do { if (_DEBUG) fprintf(stderr, fmt, __VA_ARGS__); } while (0)

// Define UNICODE_STRING structure
typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR Buffer;
} UNICODE_STRING, * PUNICODE_STRING;

// Function pointer type for LdrLoadDll
typedef NTSTATUS(NTAPI* _LdrLoadDll)(
    PWSTR PathToFile,
    ULONG Flags,
    PUNICODE_STRING ModuleFileName,
    PHANDLE ModuleHandle
);

BOOL Inject(DWORD pID, const char* DLL_NAME);
DWORD GetTargetThreadIDFromProcName(const wchar_t* ProcName);
typedef LONG(NTAPI *NtSuspendProcess)(IN HANDLE ProcessHandle);
typedef LONG(NTAPI *NtResumeProcess)(IN HANDLE ProcessHandle);

bool InjectDLL(const wchar_t* ProcessName);