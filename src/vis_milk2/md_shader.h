#pragma once
#include <d3d9.h>
#include <d3dx9.h>
#include <d3dcompiler.h>

class mdConstantTable {
public:
    virtual HRESULT GetDesc(D3DXCONSTANTTABLE_DESC* pDesc) = 0;
    virtual D3DXHANDLE GetConstant(D3DXHANDLE hConstant, UINT Index) = 0;
    virtual HRESULT GetConstantDesc(D3DXHANDLE hConstant, D3DXCONSTANT_DESC* pDesc, UINT* pCount) = 0;
    virtual HRESULT SetVector(IDirect3DDevice9* pDevice, D3DXHANDLE hConstant, const void* pVector) = 0;
    virtual HRESULT SetMatrix(IDirect3DDevice9* pDevice, D3DXHANDLE hConstant, const void* pMatrix) = 0;
    virtual ULONG AddRef() = 0;
    virtual ULONG Release() = 0;
    virtual ~mdConstantTable() {}
};

HRESULT mdCompileShader(
    const char* pSrcData,
    UINT SrcDataLen,
    const void* pDefines,
    const void* pInclude,
    const char* pFunctionName,
    const char* pProfile,
    DWORD Flags,
    ID3DBlob** ppShader,
    ID3DBlob** ppErrorMsgs,
    mdConstantTable** ppConstantTable);

HRESULT mdCompileShaderFromFileA(
    const char* pSrcFile,
    const void* pDefines,
    const void* pInclude,
    const char* pFunctionName,
    const char* pProfile,
    DWORD Flags,
    ID3DBlob** ppShader,
    ID3DBlob** ppErrorMsgs,
    mdConstantTable** ppConstantTable);
