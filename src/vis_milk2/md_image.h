#pragma once
#include <d3d9.h>

struct mdImageInfo {
    UINT Width;
    UINT Height;
    UINT Depth;
    UINT MipLevels;
    D3DFORMAT Format;
    D3DRESOURCETYPE ResourceType;
};

#define MD_D3DX_DEFAULT ((UINT)-1)
#define MD_D3DX_DEFAULT_NONPOW2 ((UINT)-2)

HRESULT mdCreateTextureFromFileExW(
    IDirect3DDevice9* pDevice,
    const wchar_t* pSrcFile,
    UINT Width,
    UINT Height,
    UINT MipLevels,
    DWORD Usage,
    D3DFORMAT Format,
    D3DPOOL Pool,
    DWORD Filter,
    DWORD MipFilter,
    D3DCOLOR ColorKey,
    mdImageInfo* pSrcInfo,
    PALETTEENTRY* pPalette,
    IDirect3DTexture9** ppTexture);

HRESULT mdCreateTextureFromFileW(
    IDirect3DDevice9* pDevice,
    const wchar_t* pSrcFile,
    IDirect3DTexture9** ppTexture);
