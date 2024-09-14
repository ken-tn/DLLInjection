#include "stdafx.h"
#include "resource.h"

using std::string;
namespace fs = std::filesystem;

string dllPath = fs::temp_directory_path().generic_string() + "\\Firm.dll";  // Temporary path to extract the DLL
string modPath = "";
string kunModPath = "";  // Temporary path to extract the DLL
BOOL deleteModFlag = 0;
BOOL doInjectionFlag = 1;

void CleanUp();
void HideConsole();
void ShowConsole();
void Pause();

static bool ExtractFromResource(const string& outputPath, DWORD resourceId);

#ifdef _DEBUG
#define DLL_RCDATA_ID DEBUGDLL
#else
#define DLL_RCDATA_ID RELEASEDLL
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
