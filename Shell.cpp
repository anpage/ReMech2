#include "Shell.h"

// Original function locations
void(__cdecl* TrueLoadMechVariantList)(char* mechType) = (void(__cdecl*)(char*))0x1000c8b8;
int(__stdcall* TrueCallsBitBlit)(void) = (int(__stdcall*)(void))(0x10030ef9);

volatile char* MechVariantFilename = (char*)0x1007a800;
volatile char(*pMechVariantFilenames)[200][13] = (char(*)[200][13])0x10079d80;

void* PrjObject = (void*)0x10071230;
int(__thiscall* LoadFileFromPrj)(void* _this, char* fileName, int something) = (int(__thiscall*)(void*, char*, int))0x1002e346;

// Replacement function that uses cFilename instead of cAlternateFileName
void __cdecl FakeLoadMechVariantList(char* mechType)
{
	// Clear the list
	memset((void*)*pMechVariantFilenames, 0, sizeof(*pMechVariantFilenames));

	// Make sure we at least have the default variant
	sprintf((char*)MechVariantFilename, "%s00std", mechType);
	strcpy((char*)(*pMechVariantFilenames)[0], (const char*)MechVariantFilename);

	// Load the built-in mech variants from the MW2.PRJ file into the first 100 indices
	for (int i = 1; i <= 99; i++)
	{
		sprintf((char*)MechVariantFilename, "%s%02dstd", mechType, i);
		int result = LoadFileFromPrj(PrjObject, (char*)MechVariantFilename, 6);

		if (result > -1)
		{
			strcpy((char*)(*pMechVariantFilenames)[i], (const char*)MechVariantFilename);
		}
	};

	// Find all user-defined mech variants from the filesystem and load their names into index 100 and higher
	sprintf((char*)MechVariantFilename, "mek\\%s??usr.mek", mechType);
	_WIN32_FIND_DATAA findData;
	HANDLE hFindFile = FindFirstFileA((LPCSTR)MechVariantFilename, &findData);
	if (hFindFile != (HANDLE)-1)
	{
		BOOL foundNextFile;
		do
		{
			int i = (int)findData.cFileName[4] + (findData.cFileName[3] * 5 + -240) * 2 + 52;
			strncpy((char*)(*pMechVariantFilenames)[i], findData.cFileName, 8);
			(*pMechVariantFilenames)[i][8] = '\0';
			foundNextFile = FindNextFileA(hFindFile, &findData);
		} while (foundNextFile);
		FindClose(hFindFile);
	}
}

volatile BOOL* pBitBltResult = (BOOL*)0x100965f4;
volatile HDC* pHdc = (HDC*)0x10066df8;
volatile unsigned int* pBitBlitWidth = (unsigned int*)0x10096e8cl;
volatile unsigned int* pBitBltHeight = (unsigned int*)0x10096e90;
volatile HDC* pHdcSrc = (HDC*)0x10067204;

// Replacement function that doesn't assume BitBlt will assume the number of lines blitted
int __stdcall FakeCallsBitBlit()
{
	*pBitBltResult = BitBlt(*pHdc, 0, 0, *pBitBlitWidth, *pBitBltHeight, *pHdcSrc, 0, 0, SRCCOPY);

	if (*pBitBltResult == FALSE)
	{
		DWORD lastError = GetLastError();
		// debug_print("GDI StretchBlt err: %d\n", lastError);
		return -1;
	}

	return 0;
}