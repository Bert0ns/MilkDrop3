#include "md_font.h"
#include <vector>

class mdFontImpl : public mdFont {
    IDirect3DDevice9* m_pDevice;
    HFONT m_hFont;
    HDC m_hDC;
    
    int InternalDrawText_W(const wchar_t* pString, int Count, LPRECT pRect, DWORD Format, D3DCOLOR Color) {
        if (!pString || !pRect) return 0;
        
        int len = Count < 0 ? lstrlenW(pString) : Count;
        if (len == 0) return 0;
        
        if (Format & DT_CALCRECT) {
            return ::DrawTextW(m_hDC, pString, len, pRect, Format);
        }
        
        RECT calcRect = *pRect;
        ::DrawTextW(m_hDC, pString, len, &calcRect, Format | DT_CALCRECT);
        
        int width = calcRect.right - calcRect.left;
        int height = calcRect.bottom - calcRect.top;
        if (width <= 0 || height <= 0) return 0;
        
        // Ensure texture is at least 1x1
        if (width == 0) width = 1;
        if (height == 0) height = 1;
        
        BITMAPINFO bmi = {0};
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = width;
        bmi.bmiHeader.biHeight = -height;
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;
        
        void* pBits = nullptr;
        HBITMAP hBmp = CreateDIBSection(m_hDC, &bmi, DIB_RGB_COLORS, &pBits, NULL, 0);
        if (!hBmp) return 0;
        
        HBITMAP hOldBmp = (HBITMAP)SelectObject(m_hDC, hBmp);
        
        RECT dibRect = {0, 0, width, height};
        FillRect(m_hDC, &dibRect, (HBRUSH)GetStockObject(BLACK_BRUSH));
        
        SetTextColor(m_hDC, RGB(255, 255, 255));
        SetBkColor(m_hDC, RGB(0, 0, 0));
        SetBkMode(m_hDC, TRANSPARENT);
        
        DWORD drawFormat = Format & ~(DT_CENTER | DT_RIGHT | DT_VCENTER | DT_BOTTOM);
        ::DrawTextW(m_hDC, pString, len, &dibRect, drawFormat);
        
        IDirect3DTexture9* pTex = nullptr;
        if (SUCCEEDED(m_pDevice->CreateTexture(width, height, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &pTex, NULL))) {
            D3DLOCKED_RECT lr;
            if (SUCCEEDED(pTex->LockRect(0, &lr, NULL, 0))) {
                DWORD* pDest = (DWORD*)lr.pBits;
                DWORD* pSrc = (DWORD*)pBits;
                
                // Color comes as D3DCOLOR (AARRGGBB), we want to extract RGB
                DWORD rgb = Color & 0x00FFFFFF;
                DWORD alphaMult = (Color >> 24) & 0xFF;
                
                for (int y = 0; y < height; y++) {
                    for (int x = 0; x < width; x++) {
                        DWORD pixel = pSrc[y * width + x];
                        BYTE intensity = (BYTE)(pixel & 0xFF);
                        
                        // Scale alpha by the text color's alpha
                        BYTE finalAlpha = (intensity * alphaMult) / 255;
                        
                        pDest[y * (lr.Pitch / 4) + x] = (finalAlpha << 24) | rgb;
                    }
                }
                pTex->UnlockRect(0);
                
                IDirect3DStateBlock9* pStateBlock = nullptr;
                if (SUCCEEDED(m_pDevice->CreateStateBlock(D3DSBT_ALL, &pStateBlock))) {
                    pStateBlock->Capture();
                    
                    m_pDevice->SetTexture(0, pTex);
                    m_pDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1);
                    
                    m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
                    m_pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
                    m_pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
                    m_pDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
                    m_pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
                    
                    // Note: We ignore pixel/vertex shaders, draw straight D3D
                    m_pDevice->SetPixelShader(NULL);
                    m_pDevice->SetVertexShader(NULL);
                    m_pDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
                    m_pDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
                    m_pDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
                    m_pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
                    m_pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
                    m_pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
                    
                    float fx = (float)calcRect.left;
                    float fy = (float)calcRect.top;
                    
                    struct Vertex { float x, y, z, rhw; D3DCOLOR c; float u, v; };
                    Vertex verts[4] = {
                        { fx,         fy,          0.0f, 1.0f, 0xFFFFFFFF, 0.0f, 0.0f },
                        { fx + width, fy,          0.0f, 1.0f, 0xFFFFFFFF, 1.0f, 0.0f },
                        { fx,         fy + height, 0.0f, 1.0f, 0xFFFFFFFF, 0.0f, 1.0f },
                        { fx + width, fy + height, 0.0f, 1.0f, 0xFFFFFFFF, 1.0f, 1.0f }
                    };
                    
                    m_pDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, verts, sizeof(Vertex));
                    
                    pStateBlock->Apply();
                    pStateBlock->Release();
                }
                pTex->Release();
            }
        }
        
        SelectObject(m_hDC, hOldBmp);
        DeleteObject(hBmp);
        
        return height;
    }

public:
    mdFontImpl(IDirect3DDevice9* pDevice, HFONT hFont) : m_pDevice(pDevice), m_hFont(hFont) {
        m_pDevice->AddRef();
        m_hDC = CreateCompatibleDC(NULL);
        SelectObject(m_hDC, m_hFont);
    }
    
    ~mdFontImpl() {
        DeleteDC(m_hDC);
        DeleteObject(m_hFont);
        m_pDevice->Release();
    }
    
    int mdDrawTextA(void* pSprite, const char* pString, int Count, LPRECT pRect, DWORD Format, D3DCOLOR Color) override {
        if (!pString) return 0;
        int len = Count < 0 ? lstrlenA(pString) : Count;
        std::vector<wchar_t> wstr(len + 1);
        MultiByteToWideChar(CP_ACP, 0, pString, len, &wstr[0], len + 1);
        return InternalDrawText_W(&wstr[0], len, pRect, Format, Color);
    }
    
    int mdDrawTextW(void* pSprite, const wchar_t* pString, int Count, LPRECT pRect, DWORD Format, D3DCOLOR Color) override {
        return InternalDrawText_W(pString, Count, pRect, Format, Color);
    }
    
    ULONG Release() override {
        delete this;
        return 0;
    }
};

HRESULT mdCreateFontW(
    IDirect3DDevice9* pDevice,
    INT Height, UINT Width, UINT Weight, UINT MipLevels, BOOL Italic,
    DWORD CharSet, DWORD OutputPrecision, DWORD Quality, DWORD PitchAndFamily,
    LPCWSTR pFacename, LPD3DXFONT* ppFont) 
{
    if (!pDevice || !ppFont) return E_INVALIDARG;
    
    HFONT hFont = CreateFontW(Height, Width, 0, 0, Weight, Italic, FALSE, FALSE,
                              CharSet, OutputPrecision, CLIP_DEFAULT_PRECIS,
                              Quality, PitchAndFamily, pFacename);
    if (!hFont) return E_FAIL;
    
    *ppFont = new mdFontImpl(pDevice, hFont);
    return S_OK;
}
