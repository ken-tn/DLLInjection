#include "main.h"

using namespace std;

DWORD GetTargetThreadIDFromProcName(const wchar_t* ProcName) {
	PROCESSENTRY32W pe;
	HANDLE thSnapShot;

	// Take a snapshot of all processes in the system
	thSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (thSnapShot == INVALID_HANDLE_VALUE)
		return 0;

	pe.dwSize = sizeof(PROCESSENTRY32W);

	// Get the first process from the snapshot
	if (Process32FirstW(thSnapShot, &pe)) {
		do {
			// Compare the process name (case-insensitive)
			if (_wcsicmp(pe.szExeFile, ProcName) == 0) {
				CloseHandle(thSnapShot);
				return pe.th32ProcessID;  // Return process ID if match is found
			}
		} while (Process32NextW(thSnapShot, &pe)); // Continue looping through the processes
	}

	CloseHandle(thSnapShot);
	return 0;  // Return 0 if no matching process is found
}

inline BOOL Inject(DWORD pID, const char * DLL_NAME) {
	if (!pID)
	{
		printf("Failed at pID.\n");
		return false;
	}

	HANDLE ProcessHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pID);
	if (!ProcessHandle)
	{
		printf("Failed at ProcessHandle.\n");
		return false;
	}

	HMODULE Kernel32 = GetModuleHandleA("kernel32.dll");
	if (!Kernel32)
	{
		printf("Failed at Kernel32.\n");
		return false;
	}
	LPVOID LoadLibAddy = GetProcAddress(Kernel32, "LoadLibraryA");
	LPVOID RemoteString = VirtualAllocEx(ProcessHandle, 0, strlen(DLL_NAME),  MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (!RemoteString)
	{
		DWORD error = GetLastError();
		printf("VirtualAllocEx failed with error code: %d\n", error);
		return false;
	}
	WriteProcessMemory(ProcessHandle, RemoteString, DLL_NAME, strlen(DLL_NAME), NULL);
	HANDLE RemoteThread = CreateRemoteThread(ProcessHandle, 0, 0, (LPTHREAD_START_ROUTINE)LoadLibAddy, RemoteString, 0, 0);
	if (!RemoteThread)
	{
		printf("Failed at RemoteThread.\n");
		return false;
	}
	while (true)
	{
		DWORD status = WaitForSingleObject(RemoteThread, INFINITE);
		if (status == WAIT_OBJECT_0)
		{
			break;
		}
	}
	printf("Executed DLLMain.\n");
	CloseHandle(ProcessHandle);
	return true;
}

inline bool InjectDLL(const wchar_t* ProcessName)
{
	// Get the process ID of the target process by name
	DWORD pID = GetTargetThreadIDFromProcName(ProcessName);

	// Get the full path of the DLL (wide-character version)
	// wchar_t buf[MAX_PATH] = {0};
	char buf2[MAX_PATH] = {};
	// GetFullPathNameW(L"RVX4.dll", MAX_PATH, buf, NULL);
	GetFullPathNameA("../x64/Debug/RVX4.dll", MAX_PATH, buf2, NULL);
	printf(buf2);

	// Wait for the process ID to become available
	do
	{
		pID = GetTargetThreadIDFromProcName(ProcessName);
		Sleep(10);
	} while (pID == 0);

	// Open the target process with required access
	HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pID);
	if (!processHandle)
	{
		printf("Failed to open process.\n");
		return false;
	}

	// Get a handle to ntdll.dll and retrieve NtSuspendProcess and NtResumeProcess
	HMODULE ntdll = GetModuleHandleA("ntdll");
	if (!ntdll)
	{
		printf("Failed at ntdll\n");
		return false;
	}

	NtSuspendProcess pfnNtSuspendProcess = (NtSuspendProcess)GetProcAddress(ntdll, "NtSuspendProcess");
	NtResumeProcess pfnNtResumeProcess = (NtSuspendProcess)GetProcAddress(ntdll, "NtResumeProcess");

	if (!pfnNtSuspendProcess || !pfnNtResumeProcess)
	{
		printf("Failed to get NtSuspendProcess or NtResumeProcess.\n");
		return false;
	}

	printf("Process found! Waiting for window to inject.\n");

	// Suspend the target process
	// pfnNtSuspendProcess(processHandle);

	// Inject the DLL using the modified Inject function
	if (!Inject(pID, buf2)) {
		// printf("DLL has not injected. Please try again!\n");
		// Resume the process if injection fails
		// pfnNtResumeProcess(processHandle);
		CloseHandle(processHandle);
		return false;
	}

	printf("DLL injected successfully.\n");

	// Resume the target process after successful injection
	// pfnNtResumeProcess(processHandle);

	// Clean up
	CloseHandle(processHandle);
	return true;
}

int main() {
	SetConsoleTitle("Injector");
	// Path to the executable you want to run
	const char* exePath = "G:\\WuwaBeta\\wuwa-beta-downloader\\Wuthering Waves Game\\Client\\Binaries\\Win64\\Client-Win64-Shipping.exe";
	// Initialize the STARTUPINFO and PROCESS_INFORMATION structures
	STARTUPINFOA si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	// Start the process
	if (CreateProcessA(
		exePath,          // Application name
		NULL,             // Command line arguments (NULL if no arguments)
		NULL,             // Process handle not inheritable
		NULL,             // Thread handle not inheritable
		FALSE,            // Set handle inheritance to FALSE
		0,                // No creation flags
		NULL,             // Use parent's environment block
		NULL,             // Use parent's starting directory
		&si,              // Pointer to STARTUPINFO structure
		&pi)              // Pointer to PROCESS_INFORMATION structure
		) {
		printf("Process started successfully!");

		printf("Waiting for target...\n");
		wchar_t* ProcessName = L"Client-Win64-Shipping.exe";
		DWORD pID = GetTargetThreadIDFromProcName(ProcessName);
		while (!pID) {
			pID = GetTargetThreadIDFromProcName(ProcessName);
			Sleep(50);
		}
		/*if (!pID) {
			printf("ERROR: Failed to find WW.\n");
			system("PAUSE");
		}
		else {
			printf("Found WW!\n");
			InjectDLL(ProcessName);
		}*/
		printf("Found WW!\n");
		InjectDLL(ProcessName);

		// Wait until the process exits
		//WaitForSingleObject(pi.hProcess, INFINITE);

		//// Close process and thread handles
		//CloseHandle(pi.hProcess);
		//CloseHandle(pi.hThread);
	}
	else {
		printf("Failed to start process.");
	}

	return 1;
}
