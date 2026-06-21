#include <windows.h>
#include <d3dcompiler.h>

int main() {
    LPCVOID pSrcData = nullptr;
    SIZE_T SrcDataSize = 0;
    ID3DBlob* pCode = nullptr;
    ID3DBlob* pErrorMsgs = nullptr;
    
    HRESULT hr = D3DCompile(pSrcData, SrcDataSize, NULL, NULL, NULL, "main", "ps_2_0", 0, 0, &pCode, &pErrorMsgs);
    
    return 0;
}
