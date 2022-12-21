#pragma once
#define MY_MESSAGE (WM_APP + 1)



void initD3D(LPDIRECT3DDEVICE9 d3dDevice);
void renderOverlay(double frame_time, LPDIRECT3DDEVICE9 d3dDevice);
void initOverlay(HWND hwnd);