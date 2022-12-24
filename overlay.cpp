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
#include <uxtheme.h>
#include <ShellScalingApi.h>
#include <shellapi.h>

LPD3DXFONT font;
HWND overlayHandle;

extern HWND hwnd;
extern endSceneFunc trampEndScene;
extern volatile long long frame_time;
int test = 0;

struct CUSTOMVERTEX
{
    FLOAT x, y, z, rhw;    // from the D3DFVF_XYZRHW flag
    DWORD color;    // from the D3DFVF_DIFFUSE flag
};

LPDIRECT3DVERTEXBUFFER9 v_buffer;

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

RECT rc = { 0 };
HPAINTBUFFER    hBufferedPaint = NULL;
PAINTSTRUCT     ps;
HFONT hFont;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message)
    {
    case WM_PAINT:
    {
        break;
    }
    case WM_ERASEBKGND:
    {
        return 1;
    }
    case WM_TIMER:
    {
        HDC hDC = GetDC(hWnd);

        // Get doublebuffered DC
        HDC hdcMem;/*
        hBufferedPaint = BeginBufferedPaint(winDC, &rc, BPBF_COMPOSITED, NULL, &hdcMem);
        if (hBufferedPaint)
        {
            hDC = hdcMem;
        }*/
        GetWindowRect(hwnd, &rc);
        RECT rc_show = rc;
        //InvalidateRect(hWnd, 0, 1);
        RECT drawRect;
        GetClientRect(hWnd, &drawRect);
        FillRect(hDC, &drawRect, (HBRUSH)GetStockObject(BLACK_BRUSH));
        //FillRect(hDC, &drawRect, (HBRUSH)GetStockObject(GRAY_BRUSH));
        HGDIOBJ hof = SelectObject(hDC, hFont);
        SetTextColor(hDC, RGB(255, 255, 255));
        SetBkMode(hDC, TRANSPARENT);
        std::wstring fps(std::to_wstring(frame_time));
        DrawTextExW(hDC, (LPWSTR)fps.c_str(), -1, &drawRect,
            DT_SINGLELINE | DT_LEFT | DT_TOP, NULL);
        SelectObject(hDC, hof);
        if (hBufferedPaint)
        {
            // end painting
            BufferedPaintMakeOpaque(hBufferedPaint, NULL);
            EndBufferedPaint(hBufferedPaint, TRUE);
        }
        SetWindowPos(hWnd, HWND_TOPMOST, rc_show.left, rc_show.top,  rc_show.right - rc_show.left, rc_show.bottom - rc_show.top, SWP_SHOWWINDOW| SWP_NOSENDCHANGING);
        //SetWindowPos(hWnd, hwnd, 0, 0,  500, 500, SWP_SHOWWINDOW| SWP_NOSENDCHANGING);
        ReleaseDC(hWnd, hDC);
        return 0;
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
    
    SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
    HWND mainHWND = (HWND)lpParam;
    RECT windowRect;

    GetWindowRect(mainHWND, &windowRect);
    HRESULT hr = BufferedPaintInit();
    bool g_bDblBuffered = SUCCEEDED(hr);

    const wchar_t k_WndClassName[] = L"OverlayWindowClass";
    QUERY_USER_NOTIFICATION_STATE pquns;
    SHQueryUserNotificationState(&pquns);
    HINSTANCE hInstance = GetModuleHandle(0);

    // Register window class
    WNDCLASSEXW wcex = { 0 };
    wcex.cbSize = sizeof(wcex);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hInstance;
    wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wcex.lpszClassName = k_WndClassName;
    RegisterClassExW(&wcex);
   

    HWND hWnd = CreateWindowExW(WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE,
        k_WndClassName,
        L"Overlay Window",
        WS_POPUP,
        0, 0,
        windowRect.right, windowRect.bottom,
        //500,500,
        NULL, NULL,
        hInstance,
        NULL);
    SetLayeredWindowAttributes(hWnd, RGB(0, 0, 0), 255, LWA_ALPHA | LWA_COLORKEY);


    hFont = CreateFont(64, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, 0, 0, 0, 0, 0, L"Segoe UI");

    UINT_PTR timer = SetTimer(hWnd, 101, 20, NULL);

    MSG msg = { 0 };
    RECT rc_show;
    while (true) {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        if (msg.message == WM_QUIT) {
            break;
        }
        GetWindowRect(mainHWND, &rc_show);
        SetWindowPos(hWnd, HWND_TOPMOST, rc_show.left, rc_show.top, rc_show.right - rc_show.left, rc_show.bottom - rc_show.top, SWP_SHOWWINDOW | SWP_NOSENDCHANGING);
        //SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 500, 500, SWP_SHOWWINDOW | SWP_NOSENDCHANGING);
    }
    if (g_bDblBuffered)
        BufferedPaintUnInit();
    CloseHandle(mainHWND);
    ExitThread(0);
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

void drawTextd3d9(LPDIRECT3DDEVICE9 d3dDevice, std::wstring str) {
    d3dDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);

    // select the vertex buffer to display
    d3dDevice->SetStreamSource(0, v_buffer, 0, sizeof(CUSTOMVERTEX));

    // copy the vertex buffer to the back buffer
    d3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 1);
}

void renderOverlay(double frame_time, LPDIRECT3DDEVICE9 d3dDevice) {
    //drawing with dxfont
    //drawText(frame_time);
    // select which vertex format we are using
    //drawTextd3d9(d3dDevice, L"test");
    //drawGdi(200, 200);
    // 
    // drawTextid3d(frame_time, d3dDevice);
    //
    //drawDirectWrite();
    //drawDirectWrite2(d3dDevice);
}