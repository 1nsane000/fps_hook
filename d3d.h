#pragma once
#include <Windows.h>
#include <stdint.h>
#include <string>
#include <d3d9.h>

typedef HRESULT(APIENTRY* endSceneFunc)(LPDIRECT3DDEVICE9 pDevice);
typedef HRESULT(APIENTRY* presentFunc)(LPDIRECT3DDEVICE9 p_pDevice, CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion);

void APIENTRY endSceneHook(LPDIRECT3DDEVICE9 p_pDevice);
void APIENTRY presentHook(LPDIRECT3DDEVICE9 p_pDevice, CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion);
char** initD3D9Table(HWND window);
void* WINAPI hookFnPresent(char* hookedFn, char* hookFn, int copyBytesSize, unsigned char* backupBytes, std::wstring descr);
void* WINAPI hookFnEndscene(char* hookedFn, char* hookFn, int copyBytesSize, unsigned char* backupBytes, std::wstring descr);