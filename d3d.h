#pragma once
#include <Windows.h>
#include <stdint.h>
#include <string>
#include <d3d9.h>

typedef HRESULT(APIENTRY* endSceneFunc)(LPDIRECT3DDEVICE9 pDevice);

void APIENTRY endSceneHook(LPDIRECT3DDEVICE9 p_pDevice);
char** initD3D9Table(HWND window);
void* WINAPI hookFn(char* hookedFn, char* hookFn, int copyBytesSize, unsigned char* backupBytes, std::wstring descr);