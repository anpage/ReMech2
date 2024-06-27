// Replacement functions for the game's shell (main menu and such), contained in MW2SHELL.DLL

#pragma once

#include "framework.h"

// Globals
extern void* PrjObject;
extern int(__thiscall* LoadFileFromPrj)(void* _this, char* fileName, int something);

extern volatile BOOL* pBitBltResult;
extern volatile HDC* pHdc;
extern volatile unsigned int* pBitBlitWidth;
extern volatile unsigned int* pBitBltHeight;
extern volatile HDC* pHdcSrc;

extern volatile char* MechVariantFilename;
extern volatile char(*pMechVariantFilenames)[200][13];

// Original Functions
extern void(__cdecl* TrueLoadMechVariantList)(char* mechType);
extern int(__stdcall* TrueCallsBitBlit)(void);

// Replacement functions
void __cdecl FakeLoadMechVariantList(char* mechType);
int __stdcall FakeCallsBitBlit();
