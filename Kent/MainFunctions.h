#pragma once
#include <Windows.h>
#include <stdlib.h>
#include <filesystem>
#include "resource.h"

namespace fs = std::filesystem;
using std::string;
using std::thread;

// Define a structure that resembles the Rust `Registers` structure.
// You may need to adjust this based on the actual structure in use.
struct Registers {
    uintptr_t rcx;  // Equivalent to the RCX register in x64
    // other registers...
};

HICON hIcon;
HINSTANCE DllInstance;
HANDLE mthread = nullptr;
void Print(HWND hWndEdit, std::string pszText, int debug);
