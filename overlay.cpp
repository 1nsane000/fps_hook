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
#include "direct_write.h"

LPD3DXFONT font;
HWND overlayHandle;

extern HWND hwnd;
extern endSceneFunc trampEndScene;
extern double frame_time;

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

    HRESULT hr = d3dDevice->CreateTexture(
        200,
        200,
        0,
        [in]          DWORD             Usage,
        [in]          D3DFORMAT         Format,
        [in]          D3DPOOL           Pool,
        [out, retval] IDirect3DTexture9 * *ppTexture,
        [in]          HANDLE * pSharedHandle
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
    font->DrawText(
        NULL,
        std::to_wstring(frame_time).c_str(),
        -1,
        &rect3,
        0,
        fontColor3
    );
}

IDirect3D9* d3d;
void drawTextid3d(double frame_time, LPDIRECT3DDEVICE9 d3dDevice) {
    if (!d3d) {
        d3d = Direct3DCreate9(D3D_SDK_VERSION);
    }
    RECT rect = { 10, 10, 200, 200 };
}


void renderOverlay(double frame_time, LPDIRECT3DDEVICE9 d3dDevice) {
    //drawing with dxfont
    drawText(frame_time);
    drawTextid3d(frame_time, d3dDevice);
    //
    //drawDirectWrite();
    //drawDirectWrite2(d3dDevice);
}