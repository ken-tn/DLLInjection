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

inline bool InjectDLL(DWORD pID)
{
	// Get the process ID of the target process by name
	// DWORD pID = GetTargetThreadIDFromProcName(ProcessName);

	// Get the full path of the DLL (wide-character version)
	// wchar_t buf[MAX_PATH] = {0};
	char buf2[MAX_PATH] = {};
	// GetFullPathNameW(L"Firm.dll", MAX_PATH, buf, NULL);
#ifdef _DEBUG
	GetFullPathNameA("../x64/Debug/Firm.dll", MAX_PATH, buf2, NULL);
	printf("%s\n", buf2);
#endif

	// Wait for the process ID to become available
	/*do
	{
		pID = GetTargetThreadIDFromProcName(ProcessName);
		Sleep(10);
	} while (pID == 0);*/

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

string GetInstallLocation(const string& programName) {
	HKEY hUninstallKey = nullptr;
	HKEY hAppKey = nullptr;
	const char* uninstallPath = "SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall";
	char subKeyName[256];
	char displayName[256];
	char installLocation[1024];
	DWORD subKeyNameSize, displayNameSize, installLocationSize;
	LONG result;

	// Open the Uninstall registry key
	result = RegOpenKeyExA(HKEY_LOCAL_MACHINE, uninstallPath, 0, KEY_READ, &hUninstallKey);
	if (result != ERROR_SUCCESS) {
		printf("Failed to open Uninstall key : %l\n", result);
		return "";
	}

	// Iterate through the subkeys of the Uninstall key
	for (DWORD i = 0;; ++i) {
		subKeyNameSize = sizeof(subKeyName);
		result = RegEnumKeyExA(hUninstallKey, i, subKeyName, &subKeyNameSize, nullptr, nullptr, nullptr, nullptr);
		if (result == ERROR_NO_MORE_ITEMS) {
			break;  // No more subkeys
		}
		if (result != ERROR_SUCCESS) {
			printf("Failed to enumerate subkey: %l\n", result);
			continue;
		}

		// Open the subkey for each application
		result = RegOpenKeyExA(hUninstallKey, subKeyName, 0, KEY_READ, &hAppKey);
		if (result != ERROR_SUCCESS) {
			continue;  // Could not open subkey
		}

		// Query the DisplayName value to see if it matches the program name
		displayNameSize = sizeof(displayName);
		result = RegQueryValueExA(hAppKey, "DisplayName", nullptr, nullptr, (LPBYTE)displayName, &displayNameSize);
		if (result == ERROR_SUCCESS && programName == displayName) {
			// Query the InstallLocation value
			installLocationSize = sizeof(installLocation);
			result = RegQueryValueExA(hAppKey, "InstallPath", nullptr, nullptr, (LPBYTE)installLocation, &installLocationSize);
			if (result == ERROR_SUCCESS) {
				RegCloseKey(hAppKey);
				RegCloseKey(hUninstallKey);
				return std::string(installLocation);  // Return install location
			}
			else
			{
				//fallback
				result = RegQueryValueExA(hAppKey, "InstallLocation", nullptr, nullptr, (LPBYTE)installLocation, &installLocationSize);
				if (result == ERROR_SUCCESS) {
					RegCloseKey(hAppKey);
					RegCloseKey(hUninstallKey);
					return std::string(installLocation);  // Return install location
				}
			}
		}

		RegCloseKey(hAppKey);  // Close the current app key
	}

	RegCloseKey(hUninstallKey);  // Close the uninstall key
	return "";  // Program not found
}

BOOL StartProcess(const char* ExecutablePath)
{
	// Initialize the STARTUPINFO and PROCESS_INFORMATION structures
	STARTUPINFOA si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	// Start the process
	if (CreateProcessA(
		ExecutablePath,          // Application name
		NULL,             // Command line arguments (NULL if no arguments)
		NULL,             // Process handle not inheritable
		NULL,             // Thread handle not inheritable
		FALSE,            // Set handle inheritance to FALSE
		0,                // No creation flags
		NULL,             // Use parent's environment block
		NULL,             // Use parent's starting directory
		&si,              // Pointer to STARTUPINFO structure
		&pi)              // Pointer to PROCESS_INFORMATION structure
		)
	{
		printf("Process started successfully!\n");
		InjectDLL(pi.dwProcessId);

		// Wait until the process exits
		WaitForSingleObject(pi.hProcess, INFINITE);

		//// Close process and thread handles
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);

		return 1;
	}
	else
	{
		printf("Failed to start process.\n");
		return 0;
	}
}

int main(int argc, char* argv[])
{
	std::string gameExecutable;
#ifdef _DEBUG
	gameExecutable = "G:\\WuwaBeta\\wuwa-beta-downloader\\Wuthering Waves Game\\Client\\Binaries\\Win64\\Client-Win64-Shipping.exe";
#endif

	// Iterate through the command-line arguments
	//for (int i = 1; i < argc; ++i) {  // Start at 1 to skip the program name (argv[0])
	//	// Check if the argument is "GameFolder"
	//	if (std::string(argv[i]) == "gameexe" && i + 1 < argc) {
	//		// Get the value of the argument (next element)
	//		gameExecutable = argv[i + 1];
	//		break;
	//	}
	//}
	if (argv[1])
	{
		gameExecutable = argv[1];
	}

	// Check game exe was specified
	if (!gameExecutable.empty())
	{
		printf("Game executable specified: %s\n", gameExecutable.c_str());
	}
	else
	{
		printf("Game executable not specified, auto detecting....\n");
		string InstallPath = GetInstallLocation("Wuthering Waves");
		if (InstallPath.empty())
		{
			printf("Failed to find gameexe.\nLaunch with command (Launcher.exe \"exepath\").\n\nPress enter to close.");
			int x = getchar();
			return 0;
		}
		gameExecutable = InstallPath + "\\Client\\Binaries\\Win64\\Client-Win64-Shipping.exe";
		printf(gameExecutable.c_str());
	}

#ifdef _RELEASE
	#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif
	SetConsoleTitle("Launcher");
	StartProcess(gameExecutable.c_str());
	
	return 1;
}
