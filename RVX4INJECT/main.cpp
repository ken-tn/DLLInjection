//#include "Injection.h"
#include "main.h"
#include "Injection.h"

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

//inline BOOL Inject(DWORD pID, const wchar_t* DLL_NAME) {
//	if (!pID)
//	{
//		printf("Failed at pID.\n");
//		return false;
//	}
//
//	HANDLE ProcessHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pID);
//	if (!ProcessHandle)
//	{
//		printf("Failed at ProcessHandle.\n");
//		return false;
//	}
//
//	HMODULE Kernel32 = GetModuleHandleW(L"kernel32.dll");
//	if (!Kernel32)
//	{
//		printf("Failed at Kernel32.\n");
//		return false;
//	}
//
//	// Get the address of LoadLibraryExW
//	LPVOID LoadLibExAddy = GetProcAddress(Kernel32, "LoadLibraryExW");
//	if (!LoadLibExAddy)
//	{
//		printf("Failed to get address of LoadLibraryExW.\n");
//		return false;
//	}
//
//	// Allocate memory in the target process for the DLL path (wide character)
//	LPVOID RemoteString = VirtualAllocEx(ProcessHandle, 0, (wcslen(DLL_NAME) + 1) * sizeof(wchar_t), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
//	if (!RemoteString)
//	{
//		DWORD error = GetLastError();
//		printf("VirtualAllocEx failed with error code: %d\n", error);
//		return false;
//	}
//
//	// Write the DLL path to the allocated memory in the target process
//	WriteProcessMemory(ProcessHandle, RemoteString, DLL_NAME, (wcslen(DLL_NAME) + 1) * sizeof(wchar_t), NULL);
//
//	// Create the remote thread in the target process, calling LoadLibraryExW
//	HANDLE RemoteThread = CreateRemoteThread(ProcessHandle, 0, 0, (LPTHREAD_START_ROUTINE)LoadLibExAddy, RemoteString, 0, 0);
//	if (!RemoteThread)
//	{
//		printf("Failed at RemoteThread.\n");
//		return false;
//	}
//
//	// Wait for the remote thread to complete
//	printf("Waiting.\n");
//	while (true)
//	{
//		DWORD status = WaitForSingleObject(RemoteThread, INFINITE);
//		if (status == WAIT_OBJECT_0)
//		{
//			break;
//		}
//	}
//
//	printf("Executed DLLMain.\n");
//	CloseHandle(ProcessHandle);
//	return true;
//}

//inline BOOL Inject(DWORD pID, const wchar_t* DLL_NAME) {
//    if (!pID)
//    {
//        printf("Failed at pID.\n");
//        return false;
//    }
//
//    // Open the target process
//    HANDLE ProcessHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pID);
//    if (!ProcessHandle)
//    {
//        printf("Failed at ProcessHandle.\n");
//        return false;
//    }
//
//    // Get the address of LoadLibraryExW in kernel32.dll
//    HMODULE Kernel32 = GetModuleHandleW(L"kernel32.dll");
//    if (!Kernel32)
//    {
//        printf("Failed at Kernel32.\n");
//        return false;
//    }
//    LPVOID LoadLibExAddy = GetProcAddress(Kernel32, "LoadLibraryExW");
//    if (!LoadLibExAddy)
//    {
//        printf("Failed to get address of LoadLibraryExW.\n");
//        return false;
//    }
//
//    // Allocate memory in the target process for the DLL path (wide character)
//    LPVOID RemoteString = VirtualAllocEx(ProcessHandle, 0, (wcslen(DLL_NAME) + 1) * sizeof(wchar_t), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
//    if (!RemoteString)
//    {
//        DWORD error = GetLastError();
//        printf("VirtualAllocEx failed with error code: %d\n", error);
//        return false;
//    }
//
//    // Write the DLL path to the allocated memory in the target process
//    WriteProcessMemory(ProcessHandle, RemoteString, DLL_NAME, (wcslen(DLL_NAME) + 1) * sizeof(wchar_t), NULL);
//
//    // Find a thread in the target process
//    HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
//    if (hThreadSnap == INVALID_HANDLE_VALUE) {
//        printf("Failed to create thread snapshot.\n");
//        return false;
//    }
//
//    THREADENTRY32 te32;
//    te32.dwSize = sizeof(THREADENTRY32);
//
//    DWORD targetThreadID = 0;
//
//    // Iterate through the threads in the snapshot
//    if (Thread32First(hThreadSnap, &te32)) {
//        do {
//            if (te32.th32OwnerProcessID == pID) {
//                targetThreadID = te32.th32ThreadID;
//                break;  // We found a thread to hijack
//            }
//        } while (Thread32Next(hThreadSnap, &te32));
//    }
//
//    CloseHandle(hThreadSnap);
//
//    if (!targetThreadID) {
//        printf("Failed to find thread in target process.\n");
//        return false;
//    }
//
//    // Open the thread
//    HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, targetThreadID);
//    if (!hThread) {
//        printf("Failed to open target thread.\n");
//        return false;
//    }
//
//    // Suspend the target thread
//    SuspendThread(hThread);
//
//    // Get the context of the target thread
//    CONTEXT ctx;
//    ctx.ContextFlags = CONTEXT_FULL;
//    GetThreadContext(hThread, &ctx);
//
//#ifdef _WIN64
//    // For x64 systems, set the RIP register to the address of LoadLibraryExW
//    ctx.Rip = (DWORD64)LoadLibExAddy;
//    ctx.Rcx = (DWORD64)RemoteString;  // Argument for LoadLibraryExW
//#else
//    // For x86 systems, set the EIP register to the address of LoadLibraryExW
//    ctx.Eip = (DWORD)LoadLibExAddy;
//    ctx.Eax = (DWORD)RemoteString;  // Argument for LoadLibraryExW
//#endif
//
//    // Set the modified context
//    SetThreadContext(hThread, &ctx);
//
//    // Resume the thread so it will execute LoadLibraryExW
//    ResumeThread(hThread);
//
//    // Clean up
//    CloseHandle(hThread);
//    CloseHandle(ProcessHandle);
//
//    printf("Injected using thread hijacking.\n");
//
//    return true;
//}

//inline BOOL Inject(DWORD pID, const char* DLL_NAME) {
//	HINSTANCE hInjectionMod = LoadLibrary(GH_INJ_MOD_NAME);
//
//	auto InjectA = (f_InjectA)GetProcAddress(hInjectionMod, "InjectA");
//	auto GetSymbolState = (f_GetSymbolState)GetProcAddress(hInjectionMod, "GetSymbolState");
//	auto GetImportState = (f_GetSymbolState)GetProcAddress(hInjectionMod, "GetImportState");
//	auto StartDownload = (f_StartDownload)GetProcAddress(hInjectionMod, "StartDownload");
//	auto GetDownloadProgressEx = (f_GetDownloadProgressEx)GetProcAddress(hInjectionMod, "GetDownloadProgressEx");
//
//	//due to a minor bug in the current version you have to wait a bit before starting the download
//		//will be fixed in version 4.7
//	Sleep(10);
//
//	StartDownload();
//
//	//since GetSymbolState and GetImportState only return after the downloads are finished 
//		//checking the download progress is not necessary
//	while (GetDownloadProgressEx(PDB_DOWNLOAD_INDEX_NTDLL, false) != 1.0f)
//	{
//		Sleep(10);
//	}
//
//#ifdef _WIN64
//	while (GetDownloadProgressEx(PDB_DOWNLOAD_INDEX_NTDLL, true) != 1.0f)
//	{
//		Sleep(10);
//	}
//#endif
//
//	while (GetSymbolState() != 0)
//	{
//		Sleep(10);
//	}
//
//	while (GetImportState() != 0)
//	{
//		Sleep(10);
//	}
//
//	INJECTIONDATAA data =
//	{
//		"",
//		pID,
//		INJECTION_MODE::IM_LdrLoadDll,
//		LAUNCH_METHOD::LM_NtCreateThreadEx,
//		NULL, //INJ_ERASE_HEADER
//		(DWORD)0,
//		NULL,
//		NULL,
//		true
//	};
//
//	strcpy(data.szDllPath, DLL_NAME);
//
//	InjectA(&data);
//
//	return true;
//}

//inline BOOL Inject(DWORD pID, const wchar_t* DLL_NAME) {
//	if (!pID)
//	{
//		printf("Failed at pID.\n");
//		return false;
//	}
//
//	// Open the target process
//	HANDLE ProcessHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pID);
//	if (!ProcessHandle)
//	{
//		printf("Failed at ProcessHandle.\n");
//		return false;
//	}
//
//	// Get the address of LdrLoadDll from ntdll.dll
//	HMODULE hNtdll = GetModuleHandleW(L"ntdll.dll");
//	if (!hNtdll) {
//		printf("Failed to load ntdll.dll\n");
//		return false;
//	}
//
//	auto LdrLoadDll = GetProcAddress(hNtdll, "LdrLoadDll");
//	if (!LdrLoadDll) {
//		printf("Failed to get LdrLoadDll address\n");
//		return false;
//	}
//
//	// Allocate memory in the target process for the DLL path (wide character)
//	LPVOID RemoteString = VirtualAllocEx(ProcessHandle, 0, (wcslen(DLL_NAME) + 1) * sizeof(wchar_t), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
//	if (!RemoteString)
//	{
//		DWORD error = GetLastError();
//		printf("VirtualAllocEx failed with error code: %d\n", error);
//		return false;
//	}
//
//	// Write the DLL path to the allocated memory in the target process
//	WriteProcessMemory(ProcessHandle, RemoteString, DLL_NAME, (wcslen(DLL_NAME) + 1) * sizeof(wchar_t), NULL);
//
//	// Allocate memory for the UNICODE_STRING structure in the target process
//	UNICODE_STRING unicodeString;
//	unicodeString.Buffer = (PWSTR)RemoteString;
//	unicodeString.Length = (USHORT)(wcslen(DLL_NAME) * sizeof(wchar_t));
//	unicodeString.MaximumLength = unicodeString.Length + sizeof(wchar_t);
//
//	LPVOID RemoteUnicodeString = VirtualAllocEx(ProcessHandle, NULL, sizeof(UNICODE_STRING), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
//	if (!RemoteUnicodeString) {
//		printf("Failed to allocate memory for UNICODE_STRING.\n");
//		return false;
//	}
//
//	WriteProcessMemory(ProcessHandle, RemoteUnicodeString, &unicodeString, sizeof(UNICODE_STRING), NULL);
//
//	// Find a thread in the target process
//	HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
//	if (hThreadSnap == INVALID_HANDLE_VALUE) {
//		printf("Failed to create thread snapshot.\n");
//		return false;
//	}
//
//	THREADENTRY32 te32;
//	te32.dwSize = sizeof(THREADENTRY32);
//
//	DWORD targetThreadID = 0;
//
//	// Iterate through the threads in the snapshot
//	if (Thread32First(hThreadSnap, &te32)) {
//		do {
//			if (te32.th32OwnerProcessID == pID) {
//				targetThreadID = te32.th32ThreadID;
//				break;  // We found a thread to hijack
//			}
//		} while (Thread32Next(hThreadSnap, &te32));
//	}
//
//	CloseHandle(hThreadSnap);
//
//	if (!targetThreadID) {
//		printf("Failed to find thread in target process.\n");
//		return false;
//	}
//
//	// Open the thread
//	HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, targetThreadID);
//	if (!hThread) {
//		printf("Failed to open target thread.\n");
//		return false;
//	}
//
//	// Suspend the target thread
//	SuspendThread(hThread);
//
//	// Get the context of the target thread
//	CONTEXT ctx;
//	ctx.ContextFlags = CONTEXT_FULL;
//	GetThreadContext(hThread, &ctx);
//
//#ifdef _WIN64
//	// Set RIP and register values for 64-bit systems
//	ctx.Rip = (DWORD64)LdrLoadDll;
//	ctx.Rcx = (DWORD64)NULL;  // PathToFile (NULL)
//	ctx.Rdx = 0;              // Flags (0)
//	ctx.R8 = (DWORD64)RemoteUnicodeString;  // UNICODE_STRING (ModuleFileName)
//	ctx.R9 = (DWORD64)NULL;  // ModuleHandle (NULL)
//#else
//	// For x86 systems, arguments go on the stack (push them manually)
//	ctx.Eip = (DWORD)LdrLoadDll;
//	ctx.Ecx = (DWORD)RemoteUnicodeString;  // UNICODE_STRING (ModuleFileName)
//	ctx.Edx = 0; // Flags (0)
//	ctx.Ebx = 0;  // PathToFile (NULL)
//#endif
//
//	// Set the modified context
//	SetThreadContext(hThread, &ctx);
//
//	// Resume the thread so it will execute LoadLibraryExW
//	ResumeThread(hThread);
//
//	// Clean up
//	CloseHandle(hThread);
//	CloseHandle(ProcessHandle);
//
//	printf("Injected using thread hijacking.\n");
//
//	return true;
//}


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
	system("PAUSE");

	return 1;
}
