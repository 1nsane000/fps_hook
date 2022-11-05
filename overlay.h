#pragma once

void initD3D(LPDIRECT3DDEVICE9 d3dDevice);
void initWindow();
void windowTick(double);
void initWindowThread();
void renderOverlay(double frame_time);