#include "md_shader.h"
#include <vector>
#include <string>
#include <stdio.h>
#include <DirectXMath.h>

#pragma pack(push, 1)
struct CTABHeader {
    DWORD Size;
    DWORD Creator;
    DWORD Version;
    DWORD Constants;
    DWORD ConstantInfo;
    DWORD Flags;
    DWORD Target;
};

struct CTABInfo {
    DWORD Name;
    WORD  RegisterSet;
    WORD  RegisterIndex;
    WORD  RegisterCount;
    WORD  Reserved;
    DWORD TypeInfo;
    DWORD DefaultValue;
};

struct CTABTypeInfo {
    WORD Class;
    WORD Type;
    WORD Rows;
    WORD Columns;
    WORD Elements;
    WORD StructMembers;
    DWORD StructMemberInfo;
};
#pragma pack(pop)

class mdConstantTableImpl : public mdConstantTable {
    std::vector<D3DXCONSTANT_DESC> m_constants;
    D3DXCONSTANTTABLE_DESC m_desc;
    std::string m_creator;
    bool m_isPS;
    ULONG m_refCount;
public:
    mdConstantTableImpl(ID3DBlob* pShaderBlob) {
        m_refCount = 1;
        m_desc.Constants = 0;
        m_desc.Creator = nullptr;
        m_desc.Version = 0;
        m_isPS = false;
        
        if (!pShaderBlob) return;
        
        DWORD* pData = (DWORD*)pShaderBlob->GetBufferPointer();
        SIZE_T len = pShaderBlob->GetBufferSize() / 4;
        
        for (SIZE_T i = 0; i < len; i++) {
            if ((pData[i] & 0xFFFF0000) == 0xFFFE0000) {
                DWORD commentSize = (pData[i] >> 16);
                if (i + 1 < len && pData[i+1] == 0x42415443) { // "CTAB"
                    const char* ctabData = (const char*)&pData[i+2];
                    CTABHeader* header = (CTABHeader*)ctabData;
                    
                    if (header->Creator) m_creator = ctabData + header->Creator;
                    m_desc.Creator = m_creator.c_str();
                    m_desc.Version = header->Version;
                    m_desc.Constants = header->Constants;
                    m_isPS = (header->Version >> 16) == 0xFFFF;
                    
                    CTABInfo* infos = (CTABInfo*)(ctabData + header->ConstantInfo);
                    for (DWORD c = 0; c < header->Constants; c++) {
                        D3DXCONSTANT_DESC desc = {};
                        desc.Name = ctabData + infos[c].Name;
                        desc.RegisterSet = (D3DXREGISTER_SET)infos[c].RegisterSet;
                        desc.RegisterIndex = infos[c].RegisterIndex;
                        desc.RegisterCount = infos[c].RegisterCount;
                        
                        CTABTypeInfo* typeInfo = (CTABTypeInfo*)(ctabData + infos[c].TypeInfo);
                        desc.Class = (D3DXPARAMETER_CLASS)typeInfo->Class;
                        desc.Type = (D3DXPARAMETER_TYPE)typeInfo->Type;
                        desc.Rows = typeInfo->Rows;
                        desc.Columns = typeInfo->Columns;
                        desc.Elements = typeInfo->Elements;
                        desc.StructMembers = typeInfo->StructMembers;
                        desc.Bytes = 4 * desc.Elements * desc.Rows * desc.Columns;
                        desc.DefaultValue = infos[c].DefaultValue ? (ctabData + infos[c].DefaultValue) : nullptr;
                        
                        m_constants.push_back(desc);
                    }
                    break;
                }
                i += commentSize;
            }
        }
    }
    
    HRESULT GetDesc(D3DXCONSTANTTABLE_DESC* pDesc) override {
        if (!pDesc) return E_POINTER;
        *pDesc = m_desc;
        return S_OK;
    }
    
    D3DXHANDLE GetConstant(D3DXHANDLE hConstant, UINT Index) override {
        if (Index < m_constants.size()) {
            return (D3DXHANDLE)(uintptr_t)(Index + 1);
        }
        return NULL;
    }
    
    HRESULT GetConstantDesc(D3DXHANDLE hConstant, D3DXCONSTANT_DESC* pDesc, UINT* pCount) override {
        UINT index = (UINT)(uintptr_t)hConstant - 1;
        if (index >= m_constants.size()) return E_INVALIDARG;
        if (pDesc) *pDesc = m_constants[index];
        if (pCount) *pCount = 1;
        return S_OK;
    }
    
    HRESULT SetVector(IDirect3DDevice9* pDevice, D3DXHANDLE hConstant, const void* pVector) override {
        UINT index = (UINT)(uintptr_t)hConstant - 1;
        if (index >= m_constants.size()) return E_INVALIDARG;
        
        UINT regIdx = m_constants[index].RegisterIndex;
        UINT regCount = 1; // vector is 1 register
        
        if (m_isPS) {
            return pDevice->SetPixelShaderConstantF(regIdx, (const float*)pVector, regCount);
        } else {
            return pDevice->SetVertexShaderConstantF(regIdx, (const float*)pVector, regCount);
        }
    }

    HRESULT SetMatrix(IDirect3DDevice9* pDevice, D3DXHANDLE hConstant, const void* pMatrix) override {
        UINT index = (UINT)(uintptr_t)hConstant - 1;
        if (index >= m_constants.size()) return E_INVALIDARG;
        
        UINT regIdx = m_constants[index].RegisterIndex;
        UINT regCount = m_constants[index].RegisterCount; // matrix is usually 4
        
        DirectX::XMMATRIX mat = DirectX::XMLoadFloat4x4((const DirectX::XMFLOAT4X4*)pMatrix);
        mat = DirectX::XMMatrixTranspose(mat);
        DirectX::XMFLOAT4X4 transposed;
        DirectX::XMStoreFloat4x4(&transposed, mat);
        
        if (m_isPS) {
            return pDevice->SetPixelShaderConstantF(regIdx, (const float*)&transposed, regCount);
        } else {
            return pDevice->SetVertexShaderConstantF(regIdx, (const float*)&transposed, regCount);
        }
    }

    ULONG AddRef() override {
        return ++m_refCount;
    }

    ULONG Release() override {
        if (--m_refCount == 0) {
            delete this;
            return 0;
        }
        return m_refCount;
    }
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
    mdConstantTable** ppConstantTable)
{
    ID3DBlob* pCode = nullptr;
    HRESULT hr = D3DCompile(pSrcData, SrcDataLen, NULL, (const D3D_SHADER_MACRO*)pDefines, (ID3DInclude*)pInclude, pFunctionName, pProfile, Flags, 0, &pCode, ppErrorMsgs);
    if (FAILED(hr)) return hr;
    
    if (ppErrorMsgs && *ppErrorMsgs) {
        (*ppErrorMsgs)->Release();
        *ppErrorMsgs = nullptr;
    }
    
    if (ppShader) {
        *ppShader = pCode;
        pCode->AddRef();
    }
    
    if (ppConstantTable) {
        *ppConstantTable = new mdConstantTableImpl(pCode);
    }
    
    pCode->Release();
    return S_OK;
}

HRESULT mdCompileShaderFromFileA(
    const char* pSrcFile,
    const void* pDefines,
    const void* pInclude,
    const char* pFunctionName,
    const char* pProfile,
    DWORD Flags,
    ID3DBlob** ppShader,
    ID3DBlob** ppErrorMsgs,
    mdConstantTable** ppConstantTable)
{
    FILE* f = fopen(pSrcFile, "rb");
    if (!f) return E_FAIL;
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    std::string data(size, 0);
    fread(&data[0], 1, size, f);
    fclose(f);
    
    return mdCompileShader(data.c_str(), size, pDefines, pInclude, pFunctionName, pProfile, Flags, ppShader, ppErrorMsgs, ppConstantTable);
}
