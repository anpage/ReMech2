#include "AIL.h"

// Original function locations
void(CALLBACK* TrueWaveOutProc)(
	HWAVEOUT  hwo,
	UINT      uMsg,
	DWORD_PTR dwInstance,
	DWORD_PTR dwParam1,
	DWORD_PTR dwParam2
	) = (void(CALLBACK*)(
		HWAVEOUT  hwo,
		UINT      uMsg,
		DWORD_PTR dwInstance,
		DWORD_PTR dwParam1,
		DWORD_PTR dwParam2
		))(0x20008e6d);
void* (WINAPI* TrueAilFileRead)(char* filename, void* dest) = (void* (WINAPI*)(char*, void*))0x2000845f;
void (WINAPI* TrueAilMemFreeLock)(void* ptr) = (void(WINAPI*)(void*))0x20001f14;


volatile WAVEHDR** pLastFinishedWaveHdr = (volatile WAVEHDR**)0x2001ba10;
volatile WaveHdrUser** pLastFinishedWaveHdrUser = (volatile WaveHdrUser**)0x2001ba0c;
volatile unsigned int* pWaveOutProcGlobalThing = (unsigned int*)0x2001ba04;

// Replacement for AIL's waveOutOpen callback that doesn't try to suspend the main thread
void CALLBACK FakeWaveOutProc(
	HWAVEOUT  hwo,
	UINT uMsg,
	DWORD_PTR dwInstance,
	DWORD_PTR dwParam1,
	DWORD_PTR dwParam2
)
{
	if (uMsg == WOM_DONE) {
		WAVEHDR* waveHdr = (WAVEHDR*)dwParam1;
		*pLastFinishedWaveHdr = waveHdr;
		if (waveHdr->dwUser != 0) {
			*pLastFinishedWaveHdrUser = *(WaveHdrUser**)waveHdr->dwUser;
			if ((*pLastFinishedWaveHdrUser)->field_4 != 0) {
				*pWaveOutProcGlobalThing = (*pLastFinishedWaveHdrUser)->field_3;
				*(WAVEHDR**)((*pLastFinishedWaveHdrUser)->field_2 + *pWaveOutProcGlobalThing * 4) = waveHdr;
				*pWaveOutProcGlobalThing = (*pWaveOutProcGlobalThing + 1) % (*pLastFinishedWaveHdrUser)->field_1;
				(*pLastFinishedWaveHdrUser)->field_3 = *pWaveOutProcGlobalThing;
			}
		}
	}
}

// Hack to help mitigate the memory leaks while also preventing double frees
static size_t AILAllocatedBlocks[MAX_AIL_ALLOCATED_BLOCKS];

void* WINAPI FakeAilFileRead(char* filename, void* dest)
{
	void* result = TrueAilFileRead(filename, dest);
	if (dest == NULL)
	{
		for (size_t i = 0; i < MAX_AIL_ALLOCATED_BLOCKS; i++)
		{
			if (AILAllocatedBlocks[i] == NULL)
			{
				AILAllocatedBlocks[i] = (size_t)result;
				break;
			}
		}
	}
	return result;
}

// Only try to free blocks that we know haven't been freed yet
void WINAPI FakeAilMemFreeLock(void* ptr)
{
	for (size_t i = 0; i < MAX_AIL_ALLOCATED_BLOCKS; i++)
	{
		if (AILAllocatedBlocks[i] == (size_t)ptr)
		{
			TrueAilMemFreeLock(ptr);
			AILAllocatedBlocks[i] = 0;
			return;
		}
	}
	return;
}
