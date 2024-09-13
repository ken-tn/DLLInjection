#pragma once
#include <Windows.h>
#include <stdlib.h>

// Define a structure that resembles the Rust `Registers` structure.
// You may need to adjust this based on the actual structure in use.
struct Registers {
    uintptr_t rcx;  // Equivalent to the RCX register in x64
    // other registers...
};

HANDLE mthread = nullptr;
void Print(HWND hWndEdit, std::string pszText, int debug);