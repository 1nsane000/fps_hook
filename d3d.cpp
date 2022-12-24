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
#include "overlay.h"
#include "direct_write.h"
#include <atomic>

char* d3d9DeviceTable[119];

extern endSceneFunc trampEndScene;
extern presentFunc trampPresent;
extern HANDLE threadStopEvent;

constexpr int FRAME_BUFFER_LEN = 30;

int frame_idx = 0;
long long frame_sum = 0;
long long frame_buffer[FRAME_BUFFER_LEN] = { 0 };

LPDIRECT3DDEVICE9 d3dDevice = NULL;

extern HWND hwnd;

volatile long long frame_time;




long long calcFps(long long frame_time) {
    frame_sum -= frame_buffer[frame_idx];
    frame_sum += frame_time;
    frame_buffer[frame_idx++] = frame_time;
    frame_idx =  frame_idx % FRAME_BUFFER_LEN;
    return 1000000000/(frame_sum / FRAME_BUFFER_LEN);

}


IDirect3DVertexBuffer9* createVertexBuffer(LPDIRECT3DDEVICE9 d3dDevice, const D3DCOLOR& vertexColour, const RECT& rDest) {
    IDirect3DVertexBuffer9* vertexBuffer = 0;


    DWORD fvf;
    IDirect3DVertexShader9* vertex_shader = 0;
    // Set vertex shader.
    d3dDevice->GetVertexShader(&vertex_shader);
    d3dDevice->GetFVF(&fvf);

    //d3dDevice.
    return NULL;


}


void drawRectangle(int x, int y, int h, int w, D3DCOLOR color, LPDIRECT3DDEVICE9 d3dDevice) {
    D3DRECT r = { x, y, x + w, y + h };
    d3dDevice->Clear(1, &r, D3DCLEAR_TARGET, color, 0, 0);
}


std::chrono::steady_clock::time_point begin;
std::chrono::steady_clock::time_point end;
void drawText(double frame_time);
void APIENTRY endSceneHook(LPDIRECT3DDEVICE9 p_pDevice) {

    if (!d3dDevice) {
        d3dDevice = p_pDevice;
        //initD3D(d3dDevice);
        initOverlay(hwnd);
        //initDirectWrite(hwnd);
    }
    
    trampEndScene(p_pDevice);

}

void APIENTRY presentHook(LPDIRECT3DDEVICE9 p_pDevice, THIS_ CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion) {
    //
    end = std::chrono::steady_clock::now();
    long long duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count();
    frame_time = (double)calcFps(duration);


    HDC dc = 0;

    begin = std::chrono::steady_clock::now();
    //drawText(frame_time);
    trampPresent(p_pDevice, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
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

    if (dummyDeviceCreated != D3D_OK)
    {
        /*switch (dummyDeviceCreated) {
        case D3DERR_DEVICELOST:  MessageBox(0, std::wstring(L"D3DERR_DEVICELOST").c_str(), L":(", 0); break;
        case D3DERR_INVALIDCALL: MessageBox(0, std::wstring(L"D3DERR_INVALIDCALL").c_str(), L":(", 0); break;
        case D3DERR_NOTAVAILABLE: MessageBox(0, std::wstring(L"D3DERR_NOTAVAILABLE").c_str(), L":(", 0); break; break;
        case D3DERR_OUTOFVIDEOMEMORY: MessageBox(0, std::wstring(L"D3DERR_OUTOFVIDEOMEMORY").c_str(), L":(", 0); break; break;
        }*/
        d3dpp.Windowed = !d3dpp.Windowed;

        dummyDeviceCreated = d3dSys->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3dpp.hDeviceWindow, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &dummyDev);

        if (dummyDeviceCreated != D3D_OK)
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

//mov r10, [64 adr]
const char* MOV_CMS = "\x49\xBA";

//push r10
const char* PSH_CMS = "\x41\x57";

//pop r10
const char* POP_CMS = "\x41\x5F";

//jmp r10
const char* JMP_CMS = "\x41\xFF\xE2";

const char* NOP = "\x90";
// 1 byte instruction + 4 bytes address
const uint64_t SIZE_OF_REL_JMP = 5;
const uint64_t CM_SIZE_PRESENT = 15;
// adapted from https://guidedhacking.com/threads/simple-x86-c-trampoline-hook.14188/ adapted for the Present() function
// hookedFn: The function that's about to the hooked
// hookFn: The function that will be executed before `hookedFn` by causing `hookFn` to take a detour
// the hooking technique had to be adapted to use an absolute instead of a relative jump due to the way that memory mapping works on 64 bit systems
void* WINAPI hookFnPresent(char* hookedFn, char* hookFn, int copyBytesSize, unsigned char* backupBytes, std::wstring descr) {

    if (copyBytesSize < 5)
    {
        return nullptr;
    }

    SIZE_T bytesRead;
    if (!ReadProcessMemory(GetCurrentProcess(), hookedFn, backupBytes, copyBytesSize, &bytesRead))
    {
        DWORD err = GetLastError();
        MessageBox(0, std::wstring(L"[hookFn] Failed to Backup Original Bytes for " + descr + std::to_wstring(err)).c_str(), L":(", 0);
        return nullptr;
    }


    char* trampoline = (char*)VirtualAlloc(0, copyBytesSize + CM_SIZE_PRESENT, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

    memcpy(trampoline, hookedFn, copyBytesSize);
    
    uint64_t hookedFnTrampolineOffset = reinterpret_cast<uint64_t>(hookedFn) + 5;// - trampoline - CM_SIZE;

    //need to 
    memcpy(trampoline + copyBytesSize, PSH_CMS, 2);
    memcpy(trampoline + copyBytesSize + 2, MOV_CMS, sizeof(MOV_CMS));
    memcpy(trampoline + copyBytesSize + 4, &hookedFnTrampolineOffset, sizeof(hookedFnTrampolineOffset));
    memcpy(trampoline + copyBytesSize + 12, JMP_CMS, 3);


    DWORD oldProtect;
    if (!VirtualProtect(hookedFn, copyBytesSize, PAGE_EXECUTE_READWRITE, &oldProtect))
    {
        MessageBox(0, std::wstring(L"[hookFn] Failed to set RXW for " + descr).c_str(), L":(", 0);
        return nullptr;
    }

    int hookedFnHookFnOffset = hookFn - hookedFn - SIZE_OF_REL_JMP;

    memcpy(hookedFn, REL_JMP, 1);
    memcpy(hookedFn + 1, &hookedFnHookFnOffset, sizeof(hookedFnHookFnOffset));
    memcpy(hookedFn + 5, POP_CMS, 2);

    if (!VirtualProtect(hookedFn, copyBytesSize, oldProtect, &oldProtect))
    {
        MessageBox(0, std::wstring(L"[hookFn] Failed to Restore Protection for " + descr).c_str(), L":(", 0);
    }

    return trampoline;
}

//mov r11, [64 adr]
const char* MOV_CMS1 = "\x49\xBB";
//jmp r11
const char* JMP_CMS1 = "\x41\xFF\xE3";

const uint64_t CM_SIZE_ENDSCENE = 13;
void* WINAPI hookFnEndscene(char* hookedFn, char* hookFn, int copyBytesSize, unsigned char* backupBytes, std::wstring descr) {

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

    char* trampoline = (char*)VirtualAlloc((LPVOID)0x7FF000000000, copyBytesSize + CM_SIZE_ENDSCENE, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

    // steal the first `copyBytesSize` bytes from the original function
    // these will be used to make the trampoline work
    // --> jump back to `hookedFn` without executing `hookFn` again
    memcpy(trampoline, hookedFn, copyBytesSize);
    // append the relative JMP instruction after the stolen instructions
    memcpy(trampoline + copyBytesSize, MOV_CMS1, sizeof(MOV_CMS1));

    // calculate the offset between the hooked function and the trampoline
    // --> distance between the trampoline and the original function `hookedFn`
    // this will land directly *after* the inserted JMP instruction, hence subtracting 5
    uint64_t hookedFnTrampolineOffset = reinterpret_cast<uint64_t>(hookedFn) + 6;// - trampoline - CM_SIZE;
    memcpy(trampoline + copyBytesSize + 2, &hookedFnTrampolineOffset, sizeof(hookedFnTrampolineOffset));
    memcpy(trampoline + copyBytesSize + 10, JMP_CMS1, sizeof(JMP_CMS1));
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

void restoreFunc(char* func,int restoreSize, unsigned char* restoreBytes, std::wstring descr) {
    DWORD oldProtect;
    if (!VirtualProtect(func, restoreSize, PAGE_EXECUTE_READWRITE, &oldProtect))
    {
        MessageBox(0, std::wstring(L"[hookFn] Failed to set RXW for " + descr).c_str(), L":(", 0);
        return;
    }
    memcpy(func, restoreBytes, restoreSize);
    if (!VirtualProtect(func, restoreSize, oldProtect, &oldProtect))
    {
        MessageBox(0, std::wstring(L"[hookFn] Failed to Restore Protection for " + descr).c_str(), L":(", 0);
    }
}