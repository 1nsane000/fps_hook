#include "pch.h"
#include <Windows.h>
#include <d3d9.h>
#include <d3dx9core.h>
#include <stdint.h>
#include <string>
#include "d3d.h"
#include <chrono>
#include <Dwmapi.h> 
#include <TlHelp32.h>

LPDIRECT3D9 d3dOverlay = NULL;
LPDIRECT3DDEVICE9 d3dOverlayDevice = NULL;
LPD3DXFONT font;
HWND overlayHandle;
const MARGINS  margin = { 0,0,800,600 };

extern HWND hwnd;
extern endSceneFunc trampEndScene;
extern double frame_time;

void initD3D();
void renderOverlay(double frame_time);

void initD3D(LPDIRECT3DDEVICE9 d3dDevice) {

    D3DXCreateFont(
        d3dDevice,
        10,
        10,
        FW_BOLD,
        0,
        0,
        DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,
        ANTIALIASED_QUALITY,
        FF_DONTCARE | DEFAULT_PITCH,
        TEXT("Arial"),
        &font
    );
}

void drawText(double frame_time) {
    RECT rect3;
    rect3.left = 20;
    rect3.top = 20;
    rect3.right = 330;
    rect3.bottom = 330;
    D3DCOLOR fontColor3 = D3DCOLOR_ARGB(255, 255, 1, 1);
    if (!font) {
        font->OnLostDevice();
        font->OnResetDevice();
    }
    font->DrawTextW(
        NULL,
        std::to_wstring(frame_time).c_str(),
        -1,
        &rect3,
        0,
        fontColor3
    );
}


void renderOverlay(double frame_time) {
    drawText(frame_time);
}