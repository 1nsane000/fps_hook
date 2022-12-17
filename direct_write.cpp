#include "pch.h"
#include <dwrite.h>
#include <ddraw.h>
#include <D2D1.h>
#include <d3d9.h>
#include <string>

//Direct Write interfaces
IDWriteFactory* pDWriteFactory_;
IDWriteTextFormat* pTextFormat_;

//Direct 2D interfaces
ID2D1Factory* pD2DFactory_;
ID2D1HwndRenderTarget* pRT_;
ID2D1SolidColorBrush* pBrush;

void initDirectWrite(HWND hwnd) {
    HRESULT hr = D2D1CreateFactory(
        D2D1_FACTORY_TYPE_SINGLE_THREADED,
        &pD2DFactory_
    );

    if (SUCCEEDED(hr))
    {
        hr = DWriteCreateFactory(
            DWRITE_FACTORY_TYPE_SHARED,
            __uuidof(IDWriteFactory),
            reinterpret_cast<IUnknown**>(&pDWriteFactory_)
        );
    }

    if (SUCCEEDED(hr))
    {
        hr = pDWriteFactory_->CreateTextFormat(
            L"Gabriola",                // Font family name.
            NULL,                       // Font collection (NULL sets it to use the system font collection).
            DWRITE_FONT_WEIGHT_REGULAR,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            72.0f,
            L"en-us",
            &pTextFormat_
        );
    }

    RECT rc;
    GetClientRect(hwnd, &rc);

    D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);

    if (!pRT_)
    {
        // Create a Direct2D render target.
        hr = pD2DFactory_->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(
                hwnd,
                size
            ),
            &pRT_
        );

        // Create a black brush.
        if (SUCCEEDED(hr))
        {
            hr = pRT_->CreateSolidColorBrush(
                D2D1::ColorF(D2D1::ColorF::Red),
                &pBrush
            );
        }
    }
}



//void releaseDw() {
//    SafeRelease(&pRT_);
//    SafeRelease(&pBlackBrush_);
//}

void drawDirectWrite() {
    //CreateDeviceResources();

    static const WCHAR sc_helloWorld[] = L"Hello, World!";

    // Retrieve the size of the render target.
    D2D1_SIZE_F renderTargetSize = pRT_->GetSize();

    pRT_->BeginDraw();

    pRT_->SetTransform(D2D1::Matrix3x2F::Identity());

    pRT_->Clear(D2D1::ColorF(D2D1::ColorF::White));

    pRT_->DrawText(
        sc_helloWorld,
        ARRAYSIZE(sc_helloWorld) - 1,
        pTextFormat_,
        D2D1::RectF(0, 0, 100, 100),
        pBrush
    );

    pRT_->EndDraw();

    //DiscardDeviceResources();


}