#include "pch.h"
#include <Windows.h>
#include <wingdi.h>
#include <d3d9.h>

void drawGdi(int x, int y) {
	HDC hdc = CreateCompatibleDC(0);


	const wchar_t* str = L"Test String!";
	RECT r = { x , y, x, y };
	DrawTextEx(
		hdc,
		(LPWSTR)str,
		-1,
		&r,
		DT_NOCLIP,
		0
	);
}



void gdiPathText(HDC hdc) {
	BeginPath(hdc);

	// print some text
	TextOut(hdc, 0, 0, L"Some text", 9);

	// close the path bracket
	EndPath(hdc);

	
}