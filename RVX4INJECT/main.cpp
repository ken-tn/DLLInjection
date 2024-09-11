//#include "Injection.h"
#include "main.h"

using namespace std;

DWORD GetTargetThreadIDFromProcName(const char* ProcName) {
	PROCESSENTRY32 pe;
	HANDLE thSnapShot;

	thSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (thSnapShot == INVALID_HANDLE_VALUE)
		return false;

	pe.dwSize = sizeof(PROCESSENTRY32);
	Process32First(thSnapShot, &pe);
	while (Process32Next(thSnapShot, &pe)) {
		WCHAR* temp = (WCHAR*)ProcName;
		char fileLoc[260];
		for (int i = 0; i < 260; i++) {
			fileLoc[i] = pe.szExeFile[i];
		}
		if (_strcmpi(fileLoc, ProcName) == 0)
			return pe.th32ProcessID;
	}
	return 0; // if nothing is found I guess? :3
}

inline BOOL Inject(DWORD pID, const char * DLL_NAME) {
	if (!pID)
		return false;

	HANDLE ProcessHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pID);
	if (!ProcessHandle)
		return false;

	HMODULE Kernel32 = GetModuleHandleA("kernel32.dll");
	if (!Kernel32)
	{
		return false;
	}
	auto LoadLibAddy = GetProcAddress(Kernel32, "LoadLibraryA");
	auto RemoteString = VirtualAllocEx(ProcessHandle, 0, strlen(DLL_NAME) + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (!RemoteString)
	{
		return false;
	}
	WriteProcessMemory(ProcessHandle, RemoteString, DLL_NAME, strlen(DLL_NAME), NULL);
	HANDLE RemoteThread = CreateRemoteThread(ProcessHandle, 0, 0, (LPTHREAD_START_ROUTINE)LoadLibAddy, RemoteString, 0, 0);
	if (!RemoteThread)
	{
		printf("Failed at RemoteThread.");
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
	printf("Executed DLLMain");
	CloseHandle(ProcessHandle);
	return true;
}

inline bool InjectDLL(char* ProcessName)
{
	DWORD pID = GetTargetThreadIDFromProcName(ProcessName);
	char buf[MAX_PATH] = {};
	GetFullPathNameA("rvx4.dll", MAX_PATH, buf, NULL);
	do
	{
		pID = GetTargetThreadIDFromProcName(ProcessName);
		Sleep(250);
	} while (pID == 0);
	HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pID);
	NtSuspendProcess pfnNtSuspendProcess = (NtSuspendProcess)GetProcAddress(
		GetModuleHandle("ntdll"), "NtSuspendProcess");
	NtResumeProcess pfnNtResumeProcess = (NtSuspendProcess)GetProcAddress(
		GetModuleHandle("ntdll"), "NtResumeProcess");
	printf("WW PROCESS HAS BEEN FOUND! WAITING FOR WINDOW TO INJECT. ");
	pfnNtSuspendProcess(processHandle);
	if (!Inject(pID, buf)) {
		printf("DLL HAS NOT INJECTED. PLEASE TRY AGAIN!");
		pfnNtResumeProcess(processHandle);
		return false;
	}

	printf("DLL INJECTED.");
	pfnNtResumeProcess(processHandle);
	return true;
}

int main() {
	SetConsoleTitle("Injector");
	printf("Looking for target...\n");
	char* ProcessName = "cheatengine-x86_64-SSE4-AVX2.exe";
	DWORD pID = GetTargetThreadIDFromProcName(ProcessName);
	if (!pID) {
		printf("ERROR: Failed to find WW.\n");
		system("PAUSE");
	}
	else {
		printf("Found WW!\n");
		InjectDLL(ProcessName);
	}
}
