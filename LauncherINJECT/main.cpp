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
				return string(installLocation);  // Return install location
			}
			else
			{
				//fallback
				result = RegQueryValueExA(hAppKey, "InstallLocation", nullptr, nullptr, (LPBYTE)installLocation, &installLocationSize);
				if (result == ERROR_SUCCESS) {
					RegCloseKey(hAppKey);
					RegCloseKey(hUninstallKey);
					return string(installLocation);  // Return install location
				}
			}
		}

		RegCloseKey(hAppKey);  // Close the current app key
	}

	RegCloseKey(hUninstallKey);  // Close the uninstall key
	return "";  // Program not found
}

bool removeFile(const string& filePath) {
	while (fs::exists(filePath)) {
		// Try to remove the file
		std::error_code ec;  // To avoid throwing exceptions
		fs::remove(filePath, ec);
		if (!ec) {
			// File removed successfully
			return true;
		}
		// If removal failed, wait for a short duration before retrying
		Sleep(100);
	}
	// If the file doesn't exist, return true since it is already "removed"
	return true;
}

void CleanUp()
{
	// Delete DLL file
	removeFile(dllPath);
	// Delete loader
	removeFile(loaderPath);
	// Try to delete the ~mod folder, won't delete if files inside
	removeFile(modPath);
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

BOOL ExtractMod(const string ClientPath)
{
	if (!fs::exists(ClientPath + "\\Content"))
	{
		printf("Failed to find directory \\Content at %s.\n", ClientPath.c_str());
		Pause();

		return 0;
	}
	modPath = ClientPath + "\\Content\\Paks\\~mod";
	loaderPath = modPath + loadFile;

	string modFolderPath = ClientPath + "\\..\\Mod\\KunMod";
	kunModPath = modFolderPath + kunModFile;
	tpFilePath = modFolderPath + tpFile;
	// Create ~mod folder
	if (!fs::exists(modPath))
	{
		fs::create_directories(modPath);
	}
	// Create Game/Mod folder
	if (!fs::exists(modFolderPath))
	{
		fs::create_directories(modFolderPath);
	}

	// Remove mod if already installed
	removeFile(loaderPath);
	// Extract mods
	if (!ExtractFromResource(loaderPath, LOADERMOD, RT_RCDATA))
	{
		printf("Failed to extract loader.\n");
		Pause();

		return 0;
	}
	printf("Loader extracted to %s.\n", loaderPath.c_str());

	if (!fs::exists(kunModPath))
	{
		if (!ExtractFromResource(kunModPath, KUNMOD, RT_RCDATA))
		{
			printf("Failed to extract mod.\n");
			Pause();

			return 0;
		}
		printf("Mod extracted to %s.\n", kunModPath.c_str());
	}
	else
	{
		printf("Mod already installed, skipping...\n");
	}
	
	if (!fs::exists(tpFilePath))
	{
		if (!ExtractFromResource(tpFilePath, TPFILE, RT_RCDATA))
		{
			printf("Failed to extract TP.\n");
			Pause();

			return 0;
		}
		printf("TP file extracted to %s.\n", tpFilePath.c_str());
	}
	else
	{
		printf("TP file already installed, skipping...\n");
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
	map<string, string> args;
	std::string positionalArgument; // For storing standalone values (like "PathHere")

	// Parse command line arguments
	for (int i = 1; i < argc; ++i) {
		std::string arg = argv[i];
		if (arg.find('=') != std::string::npos) {
			// It's a key-value pair (e.g., -launchoption=fullscreen)
			std::string key = arg.substr(0, arg.find('='));
			std::string value = arg.substr(arg.find('=') + 1);
			args[key] = value;
		}
		else if (arg[0] == '-') {
			// It's a flag without a value (e.g., -skiplogs)
			args[arg] = "";
		}
		else {
			// It's a positional argument
			positionalArgument = arg;
		}
	}

	// Get the ExecutablePath from the positional argument (or handle differently if needed)
	string ExecutablePath;
	if (!positionalArgument.empty()) {
		ExecutablePath = positionalArgument.c_str();
	}
	else if (args.count("-path")) {
		ExecutablePath = args["-path"].c_str();
	}

	// Build the cmdArgs string with the remaining parameters
	std::ostringstream cmdArgsStream;
	for (const auto& [key, value] : args) {
		if (key != "-path") { // Exclude the path from cmdArgs
			cmdArgsStream << key;
			if (!value.empty()) {
				cmdArgsStream << "=" << value;
			}
			cmdArgsStream << " "; // Add space between arguments
		}
	}

	// Trim any trailing spaces from the cmdArgs string
	std::string cmdArgsStr = cmdArgsStream.str();
	if (!cmdArgsStr.empty() && cmdArgsStr.back() == ' ') {
		cmdArgsStr.pop_back();
	}

	if (args.count("-noinject"))
	{
		doInjectionFlag = true;
	}

	// Check game exe was specified
	string ClientPath;
	// Get the ExecutablePath (from the `-path` argument)
	
	if (!ExecutablePath.empty())
	{
		printf("Game executable specified: %s\n", ExecutablePath.c_str());
		const std::filesystem::path path = ExecutablePath;
		ClientPath = path.parent_path().parent_path().parent_path().generic_string();
	}
	else
	{
		printf("Game executable not specified, auto detecting....\n");
		string InstallPath = GetInstallLocation("Wuthering Waves");
		if (InstallPath.empty())
		{
			printf("Failed to find game executable.\nLaunch with command (Launcher.exe \"exepath\").\n");
			Pause();

			return 0;
		}
		ClientPath = InstallPath + "\\Wuthering Waves Game\\Client";
		ExecutablePath = ClientPath + "\\Binaries\\Win64\\Client-Win64-Shipping.exe";
	}

	debug_print("Client path: %s\n", ClientPath.c_str());
	debug_print("Game path: %s\n", ExecutablePath.c_str());
	ExtractMod(ClientPath);

	if (!ExtractFromResource(dllPath, DLL_RCDATA_ID, RT_RCDATA))
	{
		debug_print("Failed to extract.\n");
		Pause();

		return 0;
	}

	debug_print("%s\n", dllPath.c_str());

	SetConsoleTitle("Launcher");
	debug_print("Cmdargs: %s\n", cmdArgsStr.c_str());
	StartProcess(ExecutablePath.c_str(), cmdArgsStr.c_str());
	
	return 1;
}
