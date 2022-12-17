#include "pch.h"
#include <Windows.h>
#include <wingdi.h>

void drawGdi(int x, int y) {
	HDC hdc = CreateCompatibleDC(0);
	HBITMAP bitmap = CreateCompatibleBitmap(hdc, x, y);
	
}



void gdiPathText(HDC hdc) {
	BeginPath(hdc);

	// print some text
	TextOut(hdc, 0, 0, L"Some text", 9);

	// close the path bracket
	EndPath(hdc);

	
}