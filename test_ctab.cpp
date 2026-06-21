#include <windows.h>
#include <d3dcompiler.h>
#include <stdio.h>

#pragma comment(lib, "d3dcompiler.lib")

const char* g_Shader = 
"float4x4 matWorldViewProj; \n"
"float4 color; \n"
"sampler2D tex0 : register(s0); \n"
"float4 main(float2 uv : TEXCOORD0) : COLOR { \n"
"   return tex2D(tex0, uv) * color; \n"
"}";

int main() {
    ID3DBlob* pCode = NULL;
    ID3DBlob* pError = NULL;
    HRESULT hr = D3DCompile(g_Shader, strlen(g_Shader), "test.hlsl", NULL, NULL, "main", "ps_2_0", 0, 0, &pCode, &pError);
    if (FAILED(hr)) {
        if (pError) printf("Error: %s\n", (char*)pError->GetBufferPointer());
        return 1;
    }
    
    DWORD* pData = (DWORD*)pCode->GetBufferPointer();
    SIZE_T len = pCode->GetBufferSize() / 4;
    
    for (SIZE_T i=0; i<len; i++) {
        if ((pData[i] & 0xFFFF0000) == 0xFFFE0000) {
            // comment token
            DWORD commentSize = (pData[i] >> 16);
            if (i + 1 < len && pData[i+1] == 0x42415443) { // "CTAB"
                printf("Found CTAB at %zu (size %d dwords)\n", i, commentSize);
                DWORD* ctab = &pData[i+2]; // skip token and "CTAB"
                printf("Size: %u\n", ctab[0]);
                printf("Creator: %u\n", ctab[1]);
                printf("Version: %08X\n", ctab[2]);
                printf("Constants: %u\n", ctab[3]);
                printf("ConstantInfo: %u\n", ctab[4]);
                printf("Flags: %u\n", ctab[5]);
                printf("Target: %u\n", ctab[6]);
                break;
            }
        }
    }
    return 0;
}
