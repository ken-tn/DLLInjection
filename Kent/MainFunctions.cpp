#include "stdafx.h"
#include "detours.h"
#include "MainFunctions.h"

using namespace std;
void init();
void ShowWindowForm();
bool Startt(std::string Cmd);

char* mfm;

void Print(HWND hWndEdit, std::string pszText, int debug = 1)
{
#ifdef NDEBUG
    if (debug == 1)
    {
        return;
    }
#endif
    int nLength = GetWindowTextLength(hWndEdit);
    SendMessage(hWndEdit, EM_SETSEL, (WPARAM)nLength, (LPARAM)nLength);
    SendMessage(hWndEdit, EM_REPLACESEL, (WPARAM)FALSE, (LPARAM)pszText.c_str());
}

#define INPUT_CHAR_LIMIT 500

#define ALX_CONSOLE_WINDOW (WM_APP + 500)
#define ALX_INPUT_FIELD (WM_APP + 501)
#define ALX_WAYPOINT_BOX (WM_APP + 502)

#define MALX_EXIT (WM_APP + 600)
#define MALX_RESTART (WM_APP + 601)
#define MALX_ABOUT (WM_APP + 602)
#define MALX_CREDITS (WM_APP + 603)
#define MALX_COMMANDS (WM_APP + 604)

HWND ParentWindow = NULL;
HWND MainWindow = NULL;
HMENU WindowMenu = NULL;
HINSTANCE HInstance = NULL;

HWND InputField = NULL;
HWND txtbox = NULL;
HWND WaypointBox = NULL;

LRESULT CALLBACK DLLWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CTLCOLORSTATIC:
    {
        HDC hEdit = (HDC)wParam;
        SetTextColor(hEdit, RGB(6, 121, 158));
        SetBkColor(hEdit, RGB(255, 255, 255));
        return (LRESULT)GetStockObject(WHITE_BRUSH);
    }
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case MALX_EXIT:
            if (MessageBox(hwnd, "Are you sure you want to exit?", "Exit", MB_YESNO) == IDYES)
                terminate();
                // ExitThread(0);
            break;
        case MALX_RESTART:
            if (MessageBox(hwnd, "Are you sure you want to restart?", "Restart", MB_YESNO) == IDYES)
                // ExitThread(1);
            break;
        case MALX_CREDITS:
            MessageBox(hwnd, "- kent911t", "Credits", MB_OK);
            break;
        case MALX_COMMANDS:
            Startt("cmds");
            break;
        case ALX_INPUT_FIELD:
            if (HIWORD(wParam) == EN_MAXTEXT)
            {
                char cText[INPUT_CHAR_LIMIT];
                SendMessage((HWND)lParam, WM_GETTEXT, INPUT_CHAR_LIMIT, (LPARAM)cText);

                if (strcmp(cText, "") == 0)
                    break;

                SendMessage((HWND)lParam, WM_SETTEXT, NULL, (LPARAM)"");

                // std::string command = cText;
                Startt(cText);
            }

            break;
        }
        break;
    case WM_DESTROY:
        ExitThread(0);
        break;

    case WM_QUIT:
        ExitThread(0);
        break;
    default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }
    return 0;
}

BOOL RegisterWindowClass(const char* wClassName)
{
    WNDCLASSEX nClass;

    nClass.cbSize = sizeof(WNDCLASSEX);
    nClass.style = CS_DBLCLKS;
    nClass.lpfnWndProc = DLLWindowProc;
    nClass.cbClsExtra = 0;
    nClass.cbWndExtra = 0;
    nClass.hInstance = GetModuleHandleA(NULL);
    nClass.hIcon = LoadIcon(DllInstance, MAKEINTRESOURCE(IDI_ICON1)); // TODO: make an icon for alx?
    nClass.hIconSm = LoadIcon(DllInstance, MAKEINTRESOURCE(IDI_ICON1));
    nClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    nClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    nClass.lpszMenuName = "what";
    nClass.lpszClassName = wClassName;

    if (!RegisterClassEx(&nClass))
        return 0;

    return 1;
}

BOOL CreateSubwindows()
{
    // HINSTANCE hInstance = GetModuleHandle(NULL);
    //  execute = CreateWindowEx(NULL, "button", "Execute Command", WS_CHILD | WS_VISIBLE | WS_BORDER, 1, 400, 100, 36, MainWindow, NULL, NULL, NULL);//Top prority makeit work
    txtbox = CreateWindowEx(NULL, "EDIT", "", WS_CHILD | WS_BORDER | WS_VSCROLL | ES_MULTILINE | WS_VISIBLE | ES_READONLY | ES_AUTOVSCROLL, 10, 20, 520, 200, MainWindow, (HMENU)ALX_CONSOLE_WINDOW, HInstance, 0);
    HWND consoleFieldLabel = CreateWindowEx(NULL, "STATIC", "", WS_CHILD | WS_VISIBLE, 10, 0, 500, 20, MainWindow, NULL, HInstance, NULL);
    /*InputField = CreateWindowEx(NULL, "EDIT", "", WS_CHILD | WS_BORDER | ES_MULTILINE | WS_VISIBLE, 10, 250, 500, 20, MainWindow, (HMENU)ALX_INPUT_FIELD, HInstance, 0);
    HWND inputFieldLabel = CreateWindowEx(NULL, "STATIC", "", WS_CHILD | WS_VISIBLE, 10, 230, 500, 20, MainWindow, NULL, HInstance, NULL);
    SendMessage(inputFieldLabel, WM_SETTEXT, NULL, (LPARAM)"Scriptexe and commandexe");
    SendMessage(consoleFieldLabel, WM_SETTEXT, NULL, (LPARAM)"Console");
    SendMessage(InputField, EM_SETLIMITTEXT, INPUT_CHAR_LIMIT, NULL);*/

    // WaypointBox = CreateWindowEx(NULL, "LISTBOX", "", WS_CHILD | WS_BORDER | WS_VSCROLL | WS_VISIBLE, 10, 280, 520, 100, MainWindow, (HMENU)ALX_WAYPOINT_BOX, HInstance, 0);
    // SendMessage(WaypointBox, LVM_SETITEMTEXT, )

    HFONT textFont = CreateFont(18, 0, 0, 0, FW_LIGHT, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("Segoe UI"));

    // SendMessage(inputFieldLabel, WM_SETFONT, (WPARAM)textFont, MAKELPARAM(TRUE, 0));
    SendMessage(consoleFieldLabel, WM_SETFONT, (WPARAM)textFont, MAKELPARAM(TRUE, 0));
    SendMessage(txtbox, WM_SETFONT, (WPARAM)textFont, MAKELPARAM(TRUE, 0));
    // SendMessage(InputField, WM_SETFONT, (WPARAM)textFont, MAKELPARAM(TRUE, 0));

    return 1;
}

BOOL CreateWindowMenu()
{
    WindowMenu = CreateMenu();
    if (!WindowMenu)
        return 0;

    HMENU mainDropdown = CreatePopupMenu();
    AppendMenu(mainDropdown, MF_STRING, MALX_EXIT, "Exit");

    AppendMenu(WindowMenu, MF_POPUP, (UINT_PTR)mainDropdown, "File");

    HMENU aboutDropdown = CreatePopupMenu();
    AppendMenu(aboutDropdown, MF_STRING, MALX_CREDITS, "Credits");

    AppendMenu(WindowMenu, MF_POPUP, (UINT_PTR)aboutDropdown, "Options");

    return 1;
}

// Structure to hold the target substring and the result HWND
struct FindWindowData {
    std::string partTitle;
    HWND hWnd;
};

// Callback function for EnumWindows
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    // Buffer to hold the window title
    char windowTitle[256];

    // Get the window title
    if (GetWindowTextA(hwnd, windowTitle, sizeof(windowTitle)) > 0) {
        // Convert the window title to std::string for easier manipulation
        std::string title(windowTitle);

        // Get the FindWindowData structure
        FindWindowData* data = reinterpret_cast<FindWindowData*>(lParam);

        // Check if the window title contains the target substring
        if (title.find(data->partTitle) != std::string::npos) {
            std::cout << "Window found: " << title << std::endl;

            // Store the found window handle in the structure
            data->hWnd = hwnd;

            // Stop enumeration by returning FALSE
            return FALSE;
        }
    }

    // Continue enumerating windows
    return TRUE;
}

HWND FindWindowByPartialTitle(const std::string& partTitle) {
    FindWindowData data = { partTitle, NULL };

    // Enumerate all top-level windows and pass the FindWindowData structure
    EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&data));

    // Return the found HWND (or NULL if not found)
    return data.hWnd;
}

void CloseDelayed()
{
    /*HWND hWnd = FindWindowByPartialTitle("Wave");
    while (!hWnd)
    {
        hWnd = FindWindowByPartialTitle("Wave");
    }
    SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
    SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
    UpdateWindow(hWnd);*/
    Print(txtbox, "Closing window in 10 seconds...\n");
    Sleep(10000);
    PostMessage(MainWindow, WM_CLOSE, 0, 0);
}

BOOL InitiateWindow()
{
    HInstance = GetModuleHandleA(NULL);
    UnregisterClass("FR_WINDOW", HInstance);
    RegisterWindowClass("FR_WINDOW");

    char windowName[50];

    _snprintf_s(windowName, 50, "Launcher v1.3");

    // ParentWindow = FindWindow(NULL, "ROBLOX");
    if (!CreateWindowMenu())
        return 0;

    if (!(MainWindow = CreateWindowExA(
        NULL,
        "FR_WINDOW",
        windowName,
        WS_SYSMENU | WS_MINIMIZEBOX,
        50,
        50,
        600,
        350,
        NULL, // ParentWindow,
        WindowMenu,
        HInstance,
        NULL)))
        return 0;

    // ScrollWindowEx(MainWindow, 0, 560, NULL, NULL, NULL, NULL, SW_SCROLLCHILDREN | SW_SMOOTHSCROLL);
    // EnableScrollBar(MainWindow, SB_VERT, ESB_ENABLE_BOTH);

    CreateSubwindows();
    Print(txtbox, "Loading...\r\n");

    ShowWindow(MainWindow, SW_NORMAL);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

namespace Memory
{
    bool Compare(const BYTE* pData, const BYTE* bMask, const char* szMask)
    {
        for (; *szMask; ++szMask, ++pData, ++bMask)
            if (*szMask == 'x' && *pData != *bMask)
                return 0;
        return (*szMask) == NULL;
    }

    DWORD FindPattern(DWORD dwAddress, DWORD dwLen, BYTE* bMask, char* szMask)
    {
        for (int i = 0; i < (int)dwLen; i++)
            if (Compare((BYTE*)(dwAddress + (int)i), bMask, szMask))
                return (int)(dwAddress + i);
        return 0;
    }

    int Scan(DWORD mode, char* content, char* mask)
    {
        DWORD PageSize;
        SYSTEM_INFO si;
        GetSystemInfo(&si);
        PageSize = si.dwPageSize;
        MEMORY_BASIC_INFORMATION mi;
        for (DWORD lpAddr = 0; lpAddr < 0x7FFFFFFF; lpAddr += PageSize)
        {
            DWORD vq = VirtualQuery((void*)lpAddr, &mi, PageSize);
            if (vq == ERROR_INVALID_PARAMETER || vq == 0)
                break;
            if (mi.Type == MEM_MAPPED)
                continue;
            if (mi.Protect == mode)
            {
                int addr = FindPattern(lpAddr, PageSize, (PBYTE)content, mask);
                if (addr != 0)
                {
                    return addr;
                }
            }
        }

        return 0;
    }

}

std::vector<std::string> split(std::string s)
{
    std::vector<std::string> elems;
    std::stringstream ss(s);
    std::istream_iterator<std::string> begin(ss);
    std::istream_iterator<std::string> end;
    std::vector<std::string> vstrings(begin, end);
    return vstrings;
}

std::string IntToHex(int Val)
{
    std::stringstream stream;
    stream << std::hex << Val;
    return stream.str();
}

std::string Input()
{
    std::string wotthefuck;
    getline(std::cin, wotthefuck);
    return wotthefuck;
}

//  Original function pointer (trampoline)
typedef uintptr_t(__fastcall* MyFunctionType)(uintptr_t* rcx, uintptr_t, uintptr_t);
MyFunctionType originalFunction = nullptr;

// Function to convert a wide string (UTF-16) to a narrow string (UTF-8)
std::string ws2s(const std::wstring& wstr)
{
    using convert_typeX = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_typeX, wchar_t> converterX;

    return converterX.to_bytes(wstr);
}

void hexPrint(uintptr_t* val, int additionalInt)
{
    std::stringstream s;
    s << "registers: " << std::hex << std::uppercase << val
        << " | index: " << additionalInt << "\r\n";
    Print(txtbox, s.str());
    const wchar_t* wstr = reinterpret_cast<const wchar_t*>(*(reinterpret_cast<uintptr_t*>(val) + 1));
    try {
        Print(txtbox, ws2s(wstr).c_str());
    }
    catch (...) {
    }
}

// Detour function, ensure this matches the x64 calling convention
extern "C" __declspec(dllexport) uintptr_t __fastcall MyDetourFunction(uintptr_t* rcx, uintptr_t, uintptr_t)
{
    // uintptr_t result = originalFunction(reg, a1, a2);

    // Return the original result or modify as needed
    // Safeguard against null pointer access
    if (!rcx || rcx == 0) {
        Print(txtbox, "Error: Invalid register or RCX is null");
        return 1;  // You can decide on the appropriate return for this case
    }

    const wchar_t* wstr = reinterpret_cast<const wchar_t*>(*(reinterpret_cast<uintptr_t*>(rcx) + 1));

    // Access the wide string (wchar_t*) from memory (based on how Rust handles RCX + 8)
    
    // Convert the wide string (wchar_t*) to a standard string
    /*ws2s(wstr)*/
    std::string pak_name = "Trying to verify pak " + ws2s(wstr) + "\r\n";

    // Print the string (equivalent to the Rust println! macro)
    //std::cout << "Trying to verify pak: " << pak_name << ", returning true" << std::endl;
    Print(txtbox, pak_name.c_str());

    // Return 1 (as the Rust function does)
    return 1;
}

size_t base = reinterpret_cast<size_t>(GetModuleHandle("Client-Win64-Shipping.exe"));
size_t getAddress(int address)
{
    return base + address;
}

void InitHook()
{
    /*std::stringstream ss;
    ss << "Base: " << std::hex << std::uppercase << base << "\r\n";
    Print(txtbox, ss.str());*/

    size_t SigCheck = getAddress(0x3D2F460);

    // Validate the address
    if (SigCheck == 0) {
        Print(txtbox, "Invalid address for SigCheck\r\n");
        return;
    }

    // Wait for anticheat to load
    int value = *reinterpret_cast<int*>(SigCheck);
    while (*reinterpret_cast<int*>(SigCheck) == value) {
        Sleep(10);
    }

    // Use a stringstream to format the address as hex with uppercase letters
    std::stringstream s2;
    s2 << "Sigcheck: " << std::hex << std::uppercase << SigCheck << "\r\n";
    Print(txtbox, s2.str());

    // Disable write protection on the process to insert the detour
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    Print(txtbox, "Setting up detour\r\n");

    originalFunction = reinterpret_cast<MyFunctionType>(SigCheck);
    if (!originalFunction) {
        Print(txtbox, "Failed to cast SigCheck to function pointer\r\n");
        DetourTransactionAbort();
        return;
    }

    Print(txtbox, "Cast success\r\n");

    // Attach the detour
    LONG error = DetourAttach(&(PVOID&)originalFunction, MyDetourFunction);
    if (error != NO_ERROR) {
        Print(txtbox, "Failed to attach detour: Error code " + std::to_string(error) + "\r\n");
        DetourTransactionAbort();
        return;
    }

    // Commit the transaction
    error = DetourTransactionCommit();
    if (error == NO_ERROR) {
        Print(txtbox, "Finished loading. \r\n", 0);
        std::thread t(CloseDelayed);
        t.detach();
    }
    else {
        Print(txtbox, "Detour transaction commit failed: Error code " + std::to_string(error) + "\r\n");
    }
}

void RemoveHook()
{
    // Begin a Detour transaction to remove the detour
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    // Detach the detour
    DetourDetach(&(PVOID&)originalFunction, MyDetourFunction);

    // Commit the removal
    DetourTransactionCommit();
}

void init()
{
    std::thread t(InitiateWindow);
    t.detach();
    InitHook();
}

void Console(char* title)
{
    AllocConsole();
    SetConsoleTitleA(title);
    freopen("CONOUT$", "w", stdout);
    freopen("CONIN$", "r", stdin);
    HWND ConsoleHandle = GetConsoleWindow();
    ::SetWindowPos(ConsoleHandle, HWND_TOPMOST, 0, 0, 0, 0, SWP_DRAWFRAME | SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
    ::ShowWindow(ConsoleHandle, SW_NORMAL);
}

bool Startt(std::string Cmd)
{
    std::vector<std::string> Arguments = split(Cmd);
    // Sleep(2000);
    if (Arguments.size() == 0)
    {
        return false;
    }

    return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD ul_reason_for_call, LPVOID lpvReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hinstDLL);
        DllInstance = hinstDLL;
        // Initialize the hook in a separate thread to avoid freezing
        CreateThread(0, 0, (LPTHREAD_START_ROUTINE)init, 0, 0, 0);
        break;
    case DLL_PROCESS_DETACH:
        // Remove the hook when the DLL is unloaded
        RemoveHook();
        break;
    }
    return 1;
}
