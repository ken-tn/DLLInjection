#include "main.h"

using namespace std;
namespace fs = std::filesystem;

inline BOOL Inject(DWORD pID, const char * DLL_NAME) {
	if (!pID)
	{
		debug_print("Failed at pID.\n");
		return false;
	}

	HANDLE ProcessHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pID);
	if (!ProcessHandle)
	{
		debug_print("Failed at ProcessHandle.\n");
		return false;
	}

	HMODULE Kernel32 = GetModuleHandleA("kernel32.dll");
	if (!Kernel32)
	{
		debug_print("Failed at Kernel32.\n");
		return false;
	}
	LPVOID LoadLibAddy = GetProcAddress(Kernel32, "LoadLibraryA");
	LPVOID RemoteString = VirtualAllocEx(ProcessHandle, 0, strlen(DLL_NAME),  MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (!RemoteString)
	{
		DWORD error = GetLastError();
		debug_print("VirtualAllocEx failed with error code: %d\n", error);
		return false;
	}
	WriteProcessMemory(ProcessHandle, RemoteString, DLL_NAME, strlen(DLL_NAME), NULL);
	HANDLE RemoteThread = CreateRemoteThread(ProcessHandle, 0, 0, (LPTHREAD_START_ROUTINE)LoadLibAddy, RemoteString, 0, 0);
	if (!RemoteThread)
	{
		debug_print("Failed at RemoteThread.\n");
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
	debug_print("Executed DLLMain.\n");
	CloseHandle(ProcessHandle);
	return true;
}

inline bool InjectDLL(DWORD pID)
{
	// Open the target process with required access
	HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pID);
	if (!processHandle)
	{
		debug_print("Failed to open process.\n");
		return false;
	}

	// Get a handle to ntdll.dll and retrieve NtSuspendProcess and NtResumeProcess
	HMODULE ntdll = GetModuleHandleA("ntdll");
	if (!ntdll)
	{
		debug_print("Failed at ntdll\n");
		return false;
	}

	debug_print("Process found! Waiting for window to inject.\n");

	// Inject the DLL using the modified Inject function
	if (!Inject(pID, dllPath.c_str())) {
		DeleteFile(dllPath.c_str());
		debug_print("DLL has not injected. Please try again!\n");
		CloseHandle(processHandle);
		return false;
	}

	debug_print("DLL injected successfully.\n");

	// Clean up
	CloseHandle(processHandle);
	return true;
}

static string GetInstallLocation(const string& programName) {
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
		debug_print("Failed to open Uninstall key : %lu \n", result);
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
			debug_print("Failed to enumerate subkey: %lu \n", result);
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
#ifdef NDEBUG
		HideConsole();
#endif
		debug_print("Process started successfully!\n");
		InjectDLL(pi.dwProcessId);

		// Wait until the process exits
		WaitForSingleObject(pi.hProcess, INFINITE);

		// Close process and thread handles
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);

		// Delete DLL file
		DeleteFile(dllPath.c_str());

		return 1;
	}
	else
	{
		printf("Failed to start process.\n");
		Pause();

		return 0;
	}
}

// Function to extract from resources
bool ExtractFromResource(const std::string& outputPath, int resourceId) {
	HRSRC hResource = FindResourceA(NULL, MAKEINTRESOURCE(resourceId), RT_RCDATA);
	if (!hResource) {
		debug_print("Failed to find resource.\n");
		return false;
	}

	HGLOBAL hResourceData = LoadResource(NULL, hResource);
	if (!hResourceData) {
		debug_print("Failed to load resource.\n");
		return false;
	}

	DWORD resourceSize = SizeofResource(NULL, hResource);
	void* pResourceData = LockResource(hResourceData);

	// Write the DLL to disk
	std::ofstream outputFile(outputPath, std::ios::binary);
	if (!outputFile) {
		debug_print("Failed to create file: %s\n", outputPath.c_str());
		return false;
	}

	outputFile.write(static_cast<const char*>(pResourceData), resourceSize);
	outputFile.close();

	return true;
}

void HideConsole()
{
	::ShowWindow(::GetConsoleWindow(), SW_HIDE);
}

void ShowConsole()
{
	::ShowWindow(::GetConsoleWindow(), SW_SHOW);
}

void Pause()
{
	printf("Press enter to close.\n");
	int x = getchar();
}

int main(int argc, char* argv[])
{
	std::string gameExecutable;
	// gameExecutable = "G:\\WuwaBeta\\wuwa-beta-downloader\\Wuthering Waves Game\\Client\\Binaries\\Win64\\Client-Win64-Shipping.exe";

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
		debug_print("Game executable not specified, auto detecting....\n");
		string InstallPath = GetInstallLocation("Wuthering Waves");
		string ModPath = InstallPath + "\\Wuthering Waves Game\\Client\\Content\\Paks\\~mod";
		fs::create_directories(ModPath);
		if (fs::exists(ModPath + "\\kmnew.pak"))
		{
			printf("Mod already installed.");
		}
		else
		{
			deleteFlag = 1;
			debug_print("Installing mod.");
		}
		if (InstallPath.empty())
		{
			printf("Failed to find game executable.\nLaunch with command (Launcher.exe \"exepath\").\n");
			Pause();
			
			return 0;
		}
		gameExecutable = InstallPath + "\\Wuthering Waves Game\\Client\\Binaries\\Win64\\Client-Win64-Shipping.exe";
	}

	if (!ExtractFromResource(dllPath, DLL_RCDATA_ID))
	{
		debug_print("Failed to extract.\n");
		Pause();

		return 0;
	}

	debug_print("%s\n", dllPath.c_str());

	SetConsoleTitle("Launcher");
	StartProcess(gameExecutable.c_str());
	
	return 1;
}
