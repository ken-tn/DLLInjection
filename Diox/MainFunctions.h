#pragma once
#include <Windows.h>

// Define a structure that resembles the Rust `Registers` structure.
// You may need to adjust this based on the actual structure in use.
struct Registers {
    uintptr_t rcx;  // Equivalent to the RCX register in x64
    // other registers...
};

HANDLE thread = nullptr;