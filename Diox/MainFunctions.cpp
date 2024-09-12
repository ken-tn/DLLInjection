#include <Windows.h>
#include "stdafx.h"
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include "detours.h"

using namespace std;
void init();
void ShowWindowForm();
bool Startt(std::string Cmd);


bool Changing;
int State;
// Trying something else in some minutes
void doState()
{
    if (Changing == true)
    {
    }
}

char* mfm;

void Loop()
{
    do
    {
        __try
        {
            doState();
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            Sleep(1); // do absolutely fucking nothing
        }
    } while (true);
}

const char* Commands =
// \r\n
"kill [p] - Kill's the [p], set's [p]'s health to 0.\r\n"
"god [p] - Set's [p]'s maxhealth to infinite.\r\n"
"ungod [p] - Set's [p]'s maxhealth back too 100.\r\n"
"noclip - Toggles noclip.\r\n"
"airwalk - Toggles airwalk.\r\n"
"swim - Toggles swim.\r\n"
"superjump - Toggles jump.\r\n"
"jesusfly - Toggles fly.\r\n"
"ragdoll - Toggles ragdoll.\r\n"
"disable - Disables all localplayer cmds.\r\n"
"ws [p] [n] - Set's [p]'s walkspeed to [n].\r\n"
"sethealth [p] [n] - Set's [p]'s health to [n].\r\n"
"jumppower [p] [n] - Set's [p]'s jump to [n].\r\n"
"shutdown - Shutsdown the game & server.\r\n"
"btools [p] - Give's [p] btools.\r\n"
"kick [p] - Kick's the [p] from game.\r\n"
"freeze [p] - Freezes player. \r\n"
"fog [n] - Sets Fog to [n} (set fog to 10000 to turn fog off). \r\n"
"time [n] - Sets time to [n]. \r\n"
"night - Sets time to night. \r\n"
"day - Sets time to day. \r\n"
"freeze [p] - Freezes player. \r\n"
"thaw [p] - Unfreezes player. \r\n"
"heal [p] - heals damaged player. \r\n"
"dusk - sets time to dusk. \r\n"
"stools [p] - Steals [p]'s tools. \r\n"
"purge on/off - changes lighting and fog. \r\n"
"explore - Explores the games Workspace. \r\n";

void Print(HWND hWndEdit, std::string pszText)
{
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
HMODULE HInstance = NULL;

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
            if (MessageBox(hwnd, "Are you sure you want to exit?", "Exit", MB_YESNOCANCEL) == IDYES)
                ExitThread(0);
            break;
        case MALX_RESTART:
            if (MessageBox(hwnd, "Are you sure you want to restart?", "Restart", MB_YESNOCANCEL) == IDYES)
                ExitThread(1);
            break;

        case MALX_CREDITS:
            MessageBox(hwnd, "Created by DionRBLX ", "Credits", MB_OKCANCEL);
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
    nClass.hInstance = GetModuleHandle(NULL);
    nClass.hIcon = LoadIcon(NULL, IDI_APPLICATION); // TODO: make an icon for alx?
    nClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
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
    InputField = CreateWindowEx(NULL, "EDIT", "", WS_CHILD | WS_BORDER | ES_MULTILINE | WS_VISIBLE, 10, 250, 500, 20, MainWindow, (HMENU)ALX_INPUT_FIELD, HInstance, 0);
    HWND inputFieldLabel = CreateWindowEx(NULL, "STATIC", "", WS_CHILD | WS_VISIBLE, 10, 230, 500, 20, MainWindow, NULL, HInstance, NULL);
    SendMessage(inputFieldLabel, WM_SETTEXT, NULL, (LPARAM)"Scriptexe and commandexe");
    SendMessage(consoleFieldLabel, WM_SETTEXT, NULL, (LPARAM)"Checks and console");
    SendMessage(InputField, EM_SETLIMITTEXT, INPUT_CHAR_LIMIT, NULL);

    // WaypointBox = CreateWindowEx(NULL, "LISTBOX", "", WS_CHILD | WS_BORDER | WS_VSCROLL | WS_VISIBLE, 10, 280, 520, 100, MainWindow, (HMENU)ALX_WAYPOINT_BOX, HInstance, 0);
    // SendMessage(WaypointBox, LVM_SETITEMTEXT, )

    HFONT textFont = CreateFont(18, 0, 0, 0, FW_LIGHT, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("Segoe UI"));

    SendMessage(inputFieldLabel, WM_SETFONT, (WPARAM)textFont, MAKELPARAM(TRUE, 0));
    SendMessage(consoleFieldLabel, WM_SETFONT, (WPARAM)textFont, MAKELPARAM(TRUE, 0));
    SendMessage(txtbox, WM_SETFONT, (WPARAM)textFont, MAKELPARAM(TRUE, 0));
    SendMessage(InputField, WM_SETFONT, (WPARAM)textFont, MAKELPARAM(TRUE, 0));

    UpdateWindow(MainWindow);

    return 1;
}

BOOL CreateWindowMenu()
{
    WindowMenu = CreateMenu();
    if (!WindowMenu)
        return 0;

    HMENU mainDropdown = CreatePopupMenu();
    AppendMenu(mainDropdown, MF_STRING, MALX_EXIT, "Exit");
    AppendMenu(mainDropdown, MF_STRING, MALX_RESTART, "Restart");

    AppendMenu(WindowMenu, MF_POPUP, (UINT_PTR)mainDropdown, "RVX-4");

    HMENU aboutDropdown = CreatePopupMenu();
    AppendMenu(aboutDropdown, MF_STRING, MALX_CREDITS, "Credits");
    AppendMenu(aboutDropdown, MF_STRING, MALX_COMMANDS, "Commands");

    AppendMenu(WindowMenu, MF_POPUP, (UINT_PTR)aboutDropdown, "Options");

    return 1;
}

BOOL StartMessageLoop()
{
    MSG msg;
    BOOL bRet;

    while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0)
    {
        if (bRet == 0)
        {
            return 0;
        }
        else if (bRet == -1)
        {
            // handle the error and possibly exit
            // return msg.wParam;
            return 0;
        }
        else
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return 0;
}

BOOL InitiateWindow()
{
    HInstance = GetModuleHandle(NULL);

    UnregisterClass("ALX_WINDOW", HInstance);
    RegisterWindowClass("ALX_WINDOW");

    char alxName[50];

    _snprintf_s(alxName, 50, "RVX-4");

    // ParentWindow = FindWindow(NULL, "ROBLOX");
    if (!CreateWindowMenu())
        return 0;

    if (!(MainWindow = CreateWindowEx(
        NULL,
        "ALX_WINDOW",
        alxName,
        WS_SYSMENU | WS_MINIMIZEBOX,
        50,
        50,
        600,
        600,
        NULL, // ParentWindow,
        WindowMenu,
        HInstance,
        NULL)))
        return 0;

    // ScrollWindowEx(MainWindow, 0, 560, NULL, NULL, NULL, NULL, SW_SCROLLCHILDREN | SW_SMOOTHSCROLL);
    // EnableScrollBar(MainWindow, SB_VERT, ESB_ENABLE_BOTH);

    CreateSubwindows();
    UpdateWindow(MainWindow);

    ShowWindow(MainWindow, SW_SHOWNORMAL);
    init();

    return StartMessageLoop();
}

void ShowWindowForm() {
    InitiateWindow();
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

int ScriptContextVftable;
int ScriptContext;

DWORD base = (DWORD)GetModuleHandle("Client-Win64-Shipping.exe");

DWORD getaddy(int address)
{
    return (address - 0x400000 + base);
}

// DWORD ScriptContextVftable = getaddy(0x52D130);

void init()
{
    Print(txtbox, "Checking for Filtering Enabled & Disabled Games...\r\n");
    Print(txtbox, "Checking whitelist..\r\n");
    //Sleep(2000);
    //Print(txtbox, "Whitelisted ..\r\n");
    //Sleep(2000);
    //Print(txtbox, "Welcome to RVX4 ..\r\n");
    //Print(txtbox, "Scanning... \r\n");
    // int id = 0x023B1720;
    // std::string status = doauth(*(int*)(id)).c_str();
    // std::size_t found = status.find("Success!");
    // if (found != std::string::npos) {

    // ScriptContextVftable = 0x5257C0;
    // 84c3201
    DWORD Datamodel = getaddy(0x3D2F460);
    // DWORD ScriptContextVftable = getaddy(0x10AB2130);
    // ScriptContext = Memory::Scan(PAGE_READWRITE, (char*)&ScriptContextVftable, "xxxx");
    // Roblox::DataModel = Roblox::GetParent(ScriptContext);

    // Roblox::DataModel = Memory::Scan(PAGE_READWRITE, (char*)&Datamodel, "xxxx");
    // Roblox::DataModel = Memory::Scan(PAGE_READWRITE, "\x0D\x40\xBA\x00", "xxxx");
    /*Roblox::Lighting = Roblox::FindFirstClass(Roblox::DataModel, "Lighting");
    Roblox::Workspace = Roblox::FindFirstClass(Roblox::DataModel, "Workspace");
    Roblox::Players = Roblox::FindFirstClass(Roblox::DataModel, "Players");
    Roblox::PlayerName = Roblox::GetName(Roblox::GetLocalPlayer(Roblox::Players));*/
    /*Print(txtbox, "Done \r\n");

    MessageBox(0, "Working on scriptexe /n updated to gui /n added new commands", "Update logs", MB_OK);*/

    /*Print(txtbox, "No updates found \r\n");
    Print(txtbox, "Welcome to rvx4 \r\n");
    GetLua();
    lua_getglobal(lua_State, "print");
    lua_pushstring(lua_State, "Hello world!");
    lua_pcall(lua_State, 1, 0, 0);*/

    CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)Loop, NULL, NULL, NULL);
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

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(hinstDLL);
        CreateThread(0, 0, (LPTHREAD_START_ROUTINE)ShowWindowForm, 0, 0, 0); //RVX
    }
    return 1;
}
