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

constexpr int numBytes = 6;
endSceneFunc trampEndScene;

unsigned char endSceneBytes[numBytes];

HWND hwnd = NULL;

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

DWORD WINAPI RunThread(LPVOID lpParam) {
	EnumWindows(enumGetProcessWindow, GetCurrentProcessId());

	if (!hwnd) {
		MessageBox(0, L"[D3D] HWND Not Found", L":(", 0);
		return 1;
	}

	char** vtable = initD3D9Table(hwnd);
	if (vtable) {
		char* endscene_adr = vtable[42];
		trampEndScene = (endSceneFunc)hookFn((char*)endscene_adr, (char*)endSceneHook, numBytes, endSceneBytes, L"EndScene");
	}

	while (!GetAsyncKeyState(VK_DELETE)) {
		Sleep(1000);
	}

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
		HANDLE hThread = CreateThread(NULL, 0, RunThread, hModule, 0, NULL);
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
		if (GetConsoleWindow()) {
			FreeConsole();
		}
	}
		
		break;
	}
    return TRUE;
}

extern "C" __declspec(dllexport) int getFPS(int code, WPARAM wParam, LPARAM lParam) {

    return(CallNextHookEx(NULL, code, wParam, lParam));
}