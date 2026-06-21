#pragma once
#include <d3d9.h>
#include <d3dcompiler.h>

typedef const char* D3DXHANDLE;

typedef enum _D3DXREGISTER_SET {
    D3DXRS_BOOL,
    D3DXRS_INT4,
    D3DXRS_FLOAT4,
    D3DXRS_SAMPLER,
    D3DXRS_FORCE_DWORD = 0x7fffffff
} D3DXREGISTER_SET;

typedef enum _D3DXPARAMETER_CLASS {
    D3DXPC_SCALAR,
    D3DXPC_VECTOR,
    D3DXPC_MATRIX_ROWS,
    D3DXPC_MATRIX_COLUMNS,
    D3DXPC_OBJECT,
    D3DXPC_STRUCT,
    D3DXPC_FORCE_DWORD = 0x7fffffff,
} D3DXPARAMETER_CLASS;

typedef enum _D3DXPARAMETER_TYPE {
    D3DXPT_VOID, D3DXPT_BOOL, D3DXPT_INT, D3DXPT_FLOAT, D3DXPT_STRING,
    D3DXPT_TEXTURE, D3DXPT_TEXTURE1D, D3DXPT_TEXTURE2D, D3DXPT_TEXTURE3D, D3DXPT_TEXTURECUBE,
    D3DXPT_SAMPLER, D3DXPT_SAMPLER1D, D3DXPT_SAMPLER2D, D3DXPT_SAMPLER3D, D3DXPT_SAMPLERCUBE,
    D3DXPT_PIXELSHADER, D3DXPT_VERTEXSHADER, D3DXPT_PIXELFRAGMENT, D3DXPT_VERTEXFRAGMENT,
    D3DXPT_UNSUPPORTED, D3DXPT_FORCE_DWORD = 0x7fffffff,
} D3DXPARAMETER_TYPE;

struct D3DXCONSTANT_DESC {
    const char* Name;
    D3DXREGISTER_SET RegisterSet;
    UINT RegisterIndex;
    UINT RegisterCount;
    D3DXPARAMETER_CLASS Class;
    D3DXPARAMETER_TYPE Type;
    UINT Rows;
    UINT Columns;
    UINT Elements;
    UINT StructMembers;
    UINT Bytes;
    const void* DefaultValue;
};

struct D3DXCONSTANTTABLE_DESC {
    const char* Creator;
    DWORD Version;
    UINT Constants;
};

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
