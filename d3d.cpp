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

char* d3d9DeviceTable[119];
extern endSceneFunc trampEndScene;

#define FRAME_BUFFER_LEN 30
int frame_idx = 0;
double frame_sum = 0;
int frame_buffer[FRAME_BUFFER_LEN] = { 0 };

LPDIRECT3DDEVICE9 d3dDevice = NULL;
LPDIRECT3D9 d3dOverlay = NULL;
LPDIRECT3DDEVICE9 d3dOverlayDevice = NULL;
LPD3DXFONT font;
extern HWND hwnd;
HWND overlayHandle;
const MARGINS  margin = { 0,0,800,600 };

double calcFps(long long frame_time) {
    frame_sum -= frame_buffer[frame_idx];
    frame_sum += frame_time;
    frame_buffer[frame_idx++] = frame_time;
    frame_idx =  frame_idx % FRAME_BUFFER_LEN;
    return 1000/(frame_sum / FRAME_BUFFER_LEN);

}



void drawRectangle(int x, int y, int h, int w, D3DCOLOR color) {
    D3DRECT r = { x, y, x + w, y + h };
    d3dDevice->Clear(1, &r, D3DCLEAR_TARGET, color, 0, 0);
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_PAINT:
    {
        DwmExtendFrameIntoClientArea(hWnd, &margin);
    }break;

    case WM_DESTROY:
    {
        PostQuitMessage(0);
        return 0;
    } break;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

void createWindow() {
    WNDCLASSEX wc;

    ZeroMemory(&wc, sizeof(WNDCLASSEX));

    HINSTANCE hInstance = GetModuleHandle(0);
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)RGB(0, 0, 0);
    wc.lpszClassName = L"OverlayClass";

    RegisterClassEx(&wc);
    RECT rect = {};
    GetWindowRect(hwnd, &rect);

    overlayHandle = CreateWindowEx(0,
        L"OverlayClass",
        L"",
        WS_EX_TOPMOST | WS_POPUP,
        rect.left, rect.top,
        rect.right - rect.left, rect.top-rect.bottom,
        NULL,
        NULL,
        hInstance,
        NULL);

    SetWindowLong(overlayHandle, GWL_EXSTYLE, (int)GetWindowLong(overlayHandle, GWL_EXSTYLE) | WS_EX_LAYERED | WS_EX_TRANSPARENT);
    SetLayeredWindowAttributes(overlayHandle, RGB(0, 0, 0), 0, ULW_COLORKEY);
    SetLayeredWindowAttributes(overlayHandle, 0, 255, LWA_ALPHA);

}
void initD3D() {
    createWindow();
    d3dOverlay = Direct3DCreate9(D3D_SDK_VERSION);    // create the Direct3D interface
    
    D3DPRESENT_PARAMETERS d3dpp = {};    // create a struct to hold various device information
    
    d3dpp.Windowed = TRUE;    // program windowed, not fullscreen
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;    // discard old frames
    d3dpp.hDeviceWindow = overlayHandle;    // set the window to be used by Direct3D
    d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;     // set the back buffer format to 32-bit
    d3dpp.BackBufferWidth = 800;    // set the width of the buffer
    d3dpp.BackBufferHeight = 600;    // set the height of the buffer

    d3dpp.EnableAutoDepthStencil = TRUE;
    d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

    // create a device class using this information and the info from the d3dpp stuct
    d3dOverlay->CreateDevice(D3DADAPTER_DEFAULT,
        D3DDEVTYPE_HAL,
        hwnd,
        D3DCREATE_SOFTWARE_VERTEXPROCESSING,
        &d3dpp,
        &d3dOverlayDevice);

    D3DXCreateFont(
        d3dOverlayDevice,
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
    
    if (font) {
        font->DrawTextA(
            NULL,
            std::to_string(frame_time).c_str(),
            -1,
            &rect3,
            0,
            fontColor3
        );

    }
    /*DrawTextW(
        hdc,
        std::to_wstring(frame_time).c_str(),
        -1,
        &rect3,
        0
    );*/
}

void renderOverlay(double frame_time) {
    d3dOverlayDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

    //d3dOverlayDevice->BeginScene();

    drawText(frame_time);

   //d3dOverlayDevice->EndScene();

    d3dOverlayDevice->Present(NULL, NULL, NULL, NULL);
}

std::chrono::steady_clock::time_point begin;
std::chrono::steady_clock::time_point end;
void APIENTRY endSceneHook(LPDIRECT3DDEVICE9 p_pDevice) {
    end = std::chrono::steady_clock::now();

    if (!d3dDevice) {
        d3dDevice = p_pDevice;
        initD3D();
    }

    long long duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
    /*int h = 16;
    int w = 16;
    drawRectangle(1920 / 2 - (h / 2), 1080 / 2 - (w / 2), h, w, D3DCOLOR_ARGB(100, 245, 125, 215));*/

    double frame_time = calcFps(duration);

    renderOverlay(frame_time);

    begin = std::chrono::steady_clock::now();
    trampEndScene(d3dDevice);

}

char** initD3D9Table(HWND window) {

    
	IDirect3D9* d3dSys = Direct3DCreate9(D3D_SDK_VERSION);
	if (!d3dSys || !window) {
		return NULL;
	}
	IDirect3DDevice9* dummyDev = NULL;

	// options to create dummy device
	D3DPRESENT_PARAMETERS d3dpp = {};
	d3dpp.Windowed = false;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.hDeviceWindow = window;

    HRESULT dummyDeviceCreated = d3dSys->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3dpp.hDeviceWindow, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &dummyDev);

    if (dummyDeviceCreated != S_OK)
    {
        d3dpp.Windowed = !d3dpp.Windowed;

        dummyDeviceCreated = d3dSys->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3dpp.hDeviceWindow, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &dummyDev);

        if (dummyDeviceCreated != S_OK)
        {
            d3dSys->Release();
            return NULL;
        }
    }

    memcpy(d3d9DeviceTable, *reinterpret_cast<void***>(dummyDev), sizeof(d3d9DeviceTable));

    dummyDev->Release();
    d3dSys->Release();

    return d3d9DeviceTable;
}


const char* REL_JMP = "\xE9";
const char* ABS_JMP = "\xFF";
const char* MOV_CMS = "\x49\xBB";
const char* JMP_CMS = "\x41\xFF\xE3";

const char* NOP = "\x90";
// 1 byte instruction + 4 bytes address
const uint64_t SIZE_OF_REL_JMP = 5;
const uint64_t CM_SIZE = 13;
// adapted from https://guidedhacking.com/threads/simple-x86-c-trampoline-hook.14188/
// hookedFn: The function that's about to the hooked
// hookFn: The function that will be executed before `hookedFn` by causing `hookFn` to take a detour
void* WINAPI hookFn(char* hookedFn, char* hookFn, int copyBytesSize, unsigned char* backupBytes, std::wstring descr) {

    if (copyBytesSize < 5)
    {
        // the function prologue of the hooked function
        // should be of size 5 (or larger)
        return nullptr;
    }

    //
    // 1. Backup the original function prologue
    //
    SIZE_T bytesRead;
    if (!ReadProcessMemory(GetCurrentProcess(), hookedFn, backupBytes, copyBytesSize, &bytesRead))
    {
        DWORD err = GetLastError();
        MessageBox(0, std::wstring(L"[hookFn] Failed to Backup Original Bytes for " + descr + std::to_wstring(err)).c_str(), L":(", 0);
        return nullptr;
    }

    //
    // 2. Setup the trampoline
    // --> Cause `hookedFn` to return to `hookFn` without causing an infinite loop
    // Otherwise calling `hookedFn` directly again would then call `hookFn` again, and so on :)
    //
    // allocate executable memory for the trampoline
    // the size is (amount of bytes copied from the original function) + (size of a relative jump + address)

    char* trampoline = (char*)VirtualAlloc(0, copyBytesSize + CM_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

    // steal the first `copyBytesSize` bytes from the original function
    // these will be used to make the trampoline work
    // --> jump back to `hookedFn` without executing `hookFn` again
    memcpy(trampoline, hookedFn, copyBytesSize);
    // append the relative JMP instruction after the stolen instructions
    memcpy(trampoline + copyBytesSize, MOV_CMS, sizeof(MOV_CMS));

    // calculate the offset between the hooked function and the trampoline
    // --> distance between the trampoline and the original function `hookedFn`
    // this will land directly *after* the inserted JMP instruction, hence subtracting 5
    uint64_t hookedFnTrampolineOffset = reinterpret_cast<uint64_t>(hookedFn) + 5;// - trampoline - CM_SIZE;
    memcpy(trampoline + copyBytesSize + 2, &hookedFnTrampolineOffset, sizeof(hookedFnTrampolineOffset));
    memcpy(trampoline + copyBytesSize + 10, JMP_CMS, sizeof(JMP_CMS));
    //
    // 3. Detour the original function `hookedFn`
    // --> cause `hookedFn` to execute `hookFn` first
    // remap the first few bytes of the original function as RXW
    DWORD oldProtect;
    if (!VirtualProtect(hookedFn, copyBytesSize, PAGE_EXECUTE_READWRITE, &oldProtect))
    {
        MessageBox(0, std::wstring(L"[hookFn] Failed to set RXW for " + descr).c_str(), L":(", 0);
        return nullptr;
    }

    // best variable name ever
    // calculate the size of the relative jump between the start of `hookedFn` and the start of `hookFn`.
    int hookedFnHookFnOffset = hookFn - hookedFn - SIZE_OF_REL_JMP;

    // Take a relative jump to `hookFn` at the beginning
    // of course, `hookFn` has to expect the same parameter types and values
    memcpy(hookedFn, REL_JMP, 1);
    memcpy(hookedFn + 1, &hookedFnHookFnOffset, sizeof(hookedFnHookFnOffset));

    // restore the previous protection values
    if (!VirtualProtect(hookedFn, copyBytesSize, oldProtect, &oldProtect))
    {
        MessageBox(0, std::wstring(L"[hookFn] Failed to Restore Protection for " + descr).c_str(), L":(", 0);
    }

    return trampoline;
}