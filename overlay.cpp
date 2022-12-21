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
#include <atomic>
#include "overlay.h"

LPD3DXFONT font;
HWND overlayHandle;

//extern HWND hwnd;
extern endSceneFunc trampEndScene;
extern volatile long long frame_time;
int test = 0;
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

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message)
    {
    case WM_PAINT:
    {
        break;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    default:
        break;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}


DWORD WINAPI WndThread(LPVOID lpParam) {
    HWND mainHWND = (HWND)lpParam;
    RECT windowRect;
    GetWindowRect(mainHWND, &windowRect);
    
    const wchar_t k_WndClassName[] = L"OverlayWindowClass";
    HINSTANCE hInstance = GetModuleHandle(0);
    // Register window class
    WNDCLASSEXW wcex = { 0 };
    wcex.cbSize = sizeof(wcex);
    //wcex.style = CS_HREDRAW | CS_VREDRAW ;
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hInstance;
    //wcex.hCursor = LoadCursorW(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wcex.lpszClassName = k_WndClassName;
    RegisterClassExW(&wcex);
    HWND hWnd = CreateWindowExW(WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE,
        k_WndClassName,
        L"Overlay Window",
        WS_POPUP | WS_VISIBLE | WS_OVERLAPPED,
        CW_USEDEFAULT, CW_USEDEFAULT,
        windowRect.right, windowRect.bottom,
        NULL, NULL,
        hInstance,
        NULL);
    SetLayeredWindowAttributes(hWnd, RGB(0, 0, 0), 128, LWA_ALPHA | LWA_COLORKEY);

    ShowWindow(hWnd, SW_SHOW);
    UpdateWindow(hWnd);

    MSG msg = { 0 };
    while (true) {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        if (msg.message == WM_QUIT) {
            break;
        }
        HDC hDC = GetDC(hWnd);
		RECT rc = { 0 };
		GetClientRect(hWnd, &rc);
        InvalidateRect(hWnd, 0, 1);
		SetTextColor(hDC, RGB(255, 255, 255));
		SetBkMode(hDC, TRANSPARENT);
		std::wstring fps(std::to_wstring(frame_time));
		DrawTextExW(hDC, (LPWSTR)fps.c_str(), -1, &rc,
			DT_SINGLELINE | DT_CENTER | DT_VCENTER, NULL);
        UpdateWindow(hWnd);
        ReleaseDC(hWnd, hDC);

    }

    return (int)msg.wParam;
}

void initOverlay(HWND hwnd) {
    //Rect r = { width,height };

    HANDLE hThread = CreateThread(NULL, 0, WndThread, hwnd, 0, NULL);
    if (hThread) CloseHandle(hThread);

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

void drawGdi(int x, int y);

void renderOverlay(double frame_time, LPDIRECT3DDEVICE9 d3dDevice) {
    //drawing with dxfont
    drawText(frame_time);
    //drawGdi(200, 200);
    // 
    // drawTextid3d(frame_time, d3dDevice);
    //
    //drawDirectWrite();
    //drawDirectWrite2(d3dDevice);
}