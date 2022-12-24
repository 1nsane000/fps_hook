// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <windows.h>
#include <stdio.h>
#include <psapi.h>
#include <string>
#include <stdio.h>
#include <iostream>
#include "d3d.h"
#include <stdexcept>

//number of bytes that are being overwritten for each hook
constexpr int numBytesPresent = 7;
constexpr int numBytesEndscene = 6;

//vtable offset of each function
#define PRESENT_OFF 17
#define ENDSCENE_OFF 42

//trampoline address
endSceneFunc trampEndScene;
presentFunc trampPresent;

unsigned char presentBytes[numBytesPresent];
unsigned char endSceneBytes[numBytesEndscene];

HWND hwnd = NULL;
HANDLE threadStopEvent;

BOOL CALLBACK enumGetProcessWindow(HWND _hwnd, LPARAM lParam)
{
	DWORD lpdwProcessId;
	GetWindowThreadProcessId(_hwnd, &lpdwProcessId);

	if (lpdwProcessId == lParam)
	{
		hwnd = _hwnd;
		return FALSE;
	}

	return TRUE;
}

HANDLE hThread;

HMODULE hdllModule;
char** vtable = 0;
DWORD WINAPI RunThread(LPVOID lpParam) {
	EnumWindows(enumGetProcessWindow, GetCurrentProcessId());

	if (!hwnd) {
		MessageBox(0, L"[D3D] HWND Not Found", L":(", 0);
		return 1;
	}

	while (!vtable) {
		vtable = initD3D9Table(hwnd);
	}
	if (vtable) {
		char* endscene_adr = vtable[ENDSCENE_OFF];
		trampEndScene = (endSceneFunc)hookFnEndscene((char*)endscene_adr, (char*)endSceneHook, numBytesEndscene, endSceneBytes, L"EndScene");
		char* present_adr = vtable[PRESENT_OFF];
		trampPresent = (presentFunc)hookFnPresent((char*)present_adr, (char*)presentHook, numBytesPresent, presentBytes, L"Present");

	}

	WaitForSingleObject(
		threadStopEvent,sd	
		INFINITE
	);a
	return 1;
}




BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		{
				threadStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
			hdllModule = hModule;
			hThread = CreateThread(0, 0, RunThread, hModule, 0, 0);
			if (hThread) CloseHandle(hThread);
		
		}
		break;
		case DLL_THREAD_ATTACH:
		{
		}
		break;
		case DLL_THREAD_DETACH:
		{
		}
		break;
		case DLL_PROCESS_DETACH:
		{
			SetEvent(threadStopEvent);

			//WaitForSingleObject(hThread, INFINITE);
			CloseHandle(hThread);
			CloseHandle(threadStopEvent);

			char* endscene_adr = vtable[ENDSCENE_OFF];
			restoreFunc(endscene_adr, numBytesEndscene, endSceneBytes, L"Restore Endscene");
			char* present_adr = vtable[PRESENT_OFF];
			restoreFunc(present_adr, numBytesPresent, presentBytes, L"Restore Present");
			break;
		}
	
			
	}
    return TRUE;
}

extern "C" __declspec(dllexport) int getFPS(int code, WPARAM wParam, LPARAM lParam) {

    return(CallNextHookEx(NULL, code, wParam, lParam));
}
