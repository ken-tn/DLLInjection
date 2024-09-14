#include "resource.h"
#include "stdafx.h"

std::string dllPath = "C:\\Temp\\Firm.dll";  // Temporary path to extract the DLL
int deleteFlag = 0;

void HideConsole();
void ShowConsole();
void Pause();

bool ExtractFromResource(const std::string& outputPath, int resourceId);

#ifdef _DEBUG
#define DLL_RCDATA_ID IDR_RCDATA1
#else
#define DLL_RCDATA_ID IDR_RCDATA2
#endif

#ifdef _DEBUG
#define debug_print(fmt, ...) \
        do { fprintf(stderr, fmt, __VA_ARGS__); } while (0)
#else
#define debug_print(fmt, ...) do {} while (0)
#endif

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
typedef LONG(NTAPI *NtSuspendProcess)(IN HANDLE ProcessHandle);
typedef LONG(NTAPI *NtResumeProcess)(IN HANDLE ProcessHandle);
bool InjectDLL(DWORD pID);
static string GetInstallLocation(const string& programName);
