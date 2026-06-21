#include "md_image.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <windows.h>
#include <string>

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
    IDirect3DTexture9** ppTexture)
{
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, pSrcFile, -1, NULL, 0, NULL, NULL);
    std::string utf8_filename(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, pSrcFile, -1, &utf8_filename[0], size_needed, NULL, NULL);

    int w, h, channels;
    unsigned char* pixels = stbi_load(utf8_filename.c_str(), &w, &h, &channels, 4);
    if (!pixels) {
        return E_FAIL;
    }

    if (pSrcInfo) {
        pSrcInfo->Width = w;
        pSrcInfo->Height = h;
        pSrcInfo->Depth = 1;
    }

    UINT targetW = (Width == MD_D3DX_DEFAULT || Width == MD_D3DX_DEFAULT_NONPOW2 || Width == 0) ? w : Width;
    UINT targetH = (Height == MD_D3DX_DEFAULT || Height == MD_D3DX_DEFAULT_NONPOW2 || Height == 0) ? h : Height;
    UINT levels = 1; 

    HRESULT hr = pDevice->CreateTexture(targetW, targetH, levels, Usage, D3DFMT_A8R8G8B8, Pool, ppTexture, NULL);
    if (FAILED(hr)) {
        stbi_image_free(pixels);
        return hr;
    }

    D3DLOCKED_RECT rect;
    hr = (*ppTexture)->LockRect(0, &rect, NULL, 0);
    if (SUCCEEDED(hr)) {
        unsigned char* dest = (unsigned char*)rect.pBits;
        
        for (UINT y = 0; y < targetH; ++y) {
            int srcY = (y * h) / targetH;
            unsigned char* srcRow = pixels + (srcY * w * 4);
            unsigned char* dstRow = dest + (y * rect.Pitch);
            
            for (UINT x = 0; x < targetW; ++x) {
                int srcX = (x * w) / targetW;
                int srcIdx = srcX * 4;
                int dstIdx = x * 4;
                
                unsigned char r = srcRow[srcIdx + 0];
                unsigned char g = srcRow[srcIdx + 1];
                unsigned char b = srcRow[srcIdx + 2];
                unsigned char a = srcRow[srcIdx + 3];
                
                if (ColorKey != 0) {
                    DWORD pixelColor = D3DCOLOR_ARGB(255, r, g, b);
                    if ((pixelColor & 0x00FFFFFF) == (ColorKey & 0x00FFFFFF)) {
                        a = 0;
                    }
                }
                
                dstRow[dstIdx + 0] = b;
                dstRow[dstIdx + 1] = g;
                dstRow[dstIdx + 2] = r;
                dstRow[dstIdx + 3] = a;
            }
        }
        (*ppTexture)->UnlockRect(0);
    }

    stbi_image_free(pixels);
    return hr;
}

HRESULT mdCreateTextureFromFileW(IDirect3DDevice9* pDevice, const wchar_t* pSrcFile, IDirect3DTexture9** ppTexture) {
    return mdCreateTextureFromFileExW(pDevice, pSrcFile, MD_D3DX_DEFAULT_NONPOW2, MD_D3DX_DEFAULT_NONPOW2, MD_D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, MD_D3DX_DEFAULT, MD_D3DX_DEFAULT, 0, NULL, NULL, ppTexture);
}
