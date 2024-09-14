#include "main.h"

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

void CleanUp()
{
	// Wait until it's definitely terminated.
	Sleep(2000);

	// Delete DLL file
	fs::remove(dllPath.c_str());

	// Delete
	if (deleteModFlag)
	{
		fs::remove(kunModPath.c_str());
		// Try to delete the ~mod folder, won't delete if files inside
		fs::remove(modPath);
	}
}

BOOL StartProcess(const char* ExecutablePath, const char* cmdArgs)
{
	// Prepare the command line string
	string commandLine = string(ExecutablePath) + " " + cmdArgs;
	// Initialize the STARTUPINFO and PROCESS_INFORMATION structures
	STARTUPINFOA si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	// Start the process
	if (CreateProcessA(
		ExecutablePath,          // Application name
		&commandLine[0],             // Command line arguments (NULL if no arguments)
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
		if (doInjectionFlag)
		{
			debug_print("Injecting...\n");
			InjectDLL(pi.dwProcessId);
		}

		// Wait until the process exits
		WaitForSingleObject(pi.hProcess, INFINITE);

		// Close process and thread handles
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);

		CleanUp();

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
static bool ExtractFromResource(const std::string& outputPath, DWORD resourceId, LPCSTR lpType) {
	HRSRC hResource = FindResourceA(NULL, MAKEINTRESOURCE(resourceId), lpType);
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

BOOL ExtractMod(string InstallPath)
{
	if (!fs::exists(InstallPath + "\\Wuthering Waves Game"))
	{
		printf("Failed to find Wuthering Waves Game at %s.\n", InstallPath.c_str());
		return 0;
	}
	modPath = InstallPath + "\\Wuthering Waves Game\\Client\\Content\\Paks\\~mod";
	kunModPath = modPath + "\\kmnew.pak";
	if (!fs::exists(modPath))
	{
		fs::create_directories(modPath);
	}
	if (fs::exists(kunModPath))
	{
		printf("Mod already installed.\n");
	}
	else
	{
		deleteModFlag = 1;
		if (!ExtractFromResource(kunModPath, KUNMOD, RT_RCDATA))
		{
			debug_print("Failed to extract mod.\n");

			return 0;
		}
		debug_print("Mod installed at %s.\n", kunModPath.c_str());
	}
	

	return 1;
}

std::string GetParentDirectory(const std::string& path) {
	// Ensure the path is not empty and has a valid length
	if (path.empty()) {
		return "";
	}

	// Find the position of the last directory separator ('\\' or '/')
	size_t pos = path.find_last_of("\\/");

	// If a separator is found and it's not the root directory (like "C:\")
	if (pos != std::string::npos) {
		// If the last character is a separator, remove it first
		if (pos == path.length() - 1) {
			pos = path.find_last_of("\\/", pos - 1);
		}
		// Return the substring up to the parent directory
		if (pos != std::string::npos) {
			return path.substr(0, pos);
		}
	}

	// If no parent directory is found or invalid path, return empty string
	return "";
}

int main(int argc, char* argv[])
{
	string gameExecutable;
#ifdef _DEBUG
	gameExecutable = "G:\\WuwaBeta\\wuwa-beta-downloader\\Wuthering Waves Game\\Client\\Binaries\\Win64\\Client-Win64-Shipping.exe";
#endif

	debug_print("argsc: %lu\n", argc);
	if (argv[1])
	{
		gameExecutable = argv[1];
	}
	string cmdArgs = "";
	if (argc > 2)
	{
		debug_print("Debug injection flagged.\n");
		doInjectionFlag = 0;

		// Concatenate all arguments from argv[2] onward into a single string
		std::ostringstream cmdArgsStream;
		for (int i = 2; i < argc; ++i) {
			cmdArgsStream << argv[i];
			if (i < argc - 1) {
				cmdArgsStream << " ";  // Add a space between arguments
			}
		}

		// Get the final command line arguments
		cmdArgs = cmdArgsStream.str();
	}

	// Check game exe was specified
	string InstallPath;
	if (!gameExecutable.empty())
	{
		printf("Game executable specified: %s\n", gameExecutable.c_str());
		const std::filesystem::path path = gameExecutable;
		InstallPath = path.parent_path().parent_path().parent_path().parent_path().parent_path().generic_string();
	}
	else
	{
		debug_print("Game executable not specified, auto detecting....\n");
		InstallPath = GetInstallLocation("Wuthering Waves");
		if (InstallPath.empty())
		{
			printf("Failed to find game executable.\nLaunch with command (Launcher.exe \"exepath\").\n");

			return 0;
		}
		gameExecutable = InstallPath + "\\Wuthering Waves Game\\Client\\Binaries\\Win64\\Client-Win64-Shipping.exe";
	}

	debug_print("Expected game path: %s\n", InstallPath.c_str());
	ExtractMod(InstallPath);

	if (!ExtractFromResource(dllPath, DLL_RCDATA_ID, RT_RCDATA))
	{
		debug_print("Failed to extract.\n");
		Pause();

		return 0;
	}

	debug_print("%s\n", dllPath.c_str());

	SetConsoleTitle("Launcher");
	debug_print("Cmdargs: %s\n", cmdArgs.c_str());
	StartProcess(gameExecutable.c_str(), cmdArgs.c_str());
	
	return 1;
}
