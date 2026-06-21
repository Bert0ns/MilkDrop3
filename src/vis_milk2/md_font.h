#pragma once
#include <windows.h>
#include <d3d9.h>

class mdFont {
public:
    virtual int mdDrawTextA(void* pSprite, const char* pString, int Count, LPRECT pRect, DWORD Format, D3DCOLOR Color) = 0;
    virtual int mdDrawTextW(void* pSprite, const wchar_t* pString, int Count, LPRECT pRect, DWORD Format, D3DCOLOR Color) = 0;
    virtual ULONG Release() = 0;
    virtual ~mdFont() {}
};

typedef mdFont* LPD3DXFONT;

HRESULT mdCreateFontW(
    IDirect3DDevice9* pDevice,
    INT Height,
    UINT Width,
    UINT Weight,
    UINT MipLevels,
    BOOL Italic,
    DWORD CharSet,
    DWORD OutputPrecision,
    DWORD Quality,
    DWORD PitchAndFamily,
    LPCWSTR pFacename,
    LPD3DXFONT* ppFont);
