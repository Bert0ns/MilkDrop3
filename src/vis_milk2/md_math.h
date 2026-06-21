#pragma once
#include <DirectXMath.h>

typedef DirectX::XMFLOAT2 mdVector2;
typedef DirectX::XMFLOAT3 mdVector3;
typedef DirectX::XMFLOAT4 mdVector4;
typedef DirectX::XMFLOAT4X4 mdMatrix;

inline void mdMatrixIdentity(mdMatrix* pOut) {
    DirectX::XMStoreFloat4x4(pOut, DirectX::XMMatrixIdentity());
}

inline void mdMatrixRotationX(mdMatrix* pOut, float angle) {
    DirectX::XMStoreFloat4x4(pOut, DirectX::XMMatrixRotationX(angle));
}

inline void mdMatrixRotationY(mdMatrix* pOut, float angle) {
    DirectX::XMStoreFloat4x4(pOut, DirectX::XMMatrixRotationY(angle));
}

inline void mdMatrixRotationZ(mdMatrix* pOut, float angle) {
    DirectX::XMStoreFloat4x4(pOut, DirectX::XMMatrixRotationZ(angle));
}

inline void mdMatrixTranslation(mdMatrix* pOut, float x, float y, float z) {
    DirectX::XMStoreFloat4x4(pOut, DirectX::XMMatrixTranslation(x, y, z));
}

inline void mdMatrixScaling(mdMatrix* pOut, float x, float y, float z) {
    DirectX::XMStoreFloat4x4(pOut, DirectX::XMMatrixScaling(x, y, z));
}

inline void mdMatrixMultiply(mdMatrix* pOut, const mdMatrix* pM1, const mdMatrix* pM2) {
    DirectX::XMMATRIX m1 = DirectX::XMLoadFloat4x4(pM1);
    DirectX::XMMATRIX m2 = DirectX::XMLoadFloat4x4(pM2);
    DirectX::XMStoreFloat4x4(pOut, DirectX::XMMatrixMultiply(m1, m2));
}

inline void mdMatrixOrthoLH(mdMatrix* pOut, float w, float h, float zn, float zf) {
    DirectX::XMStoreFloat4x4(pOut, DirectX::XMMatrixOrthographicLH(w, h, zn, zf));
}

inline void mdMatrixLookAtLH(mdMatrix* pOut, const mdVector3* pEye, const mdVector3* pAt, const mdVector3* pUp) {
    DirectX::XMVECTOR eye = DirectX::XMLoadFloat3(pEye);
    DirectX::XMVECTOR at = DirectX::XMLoadFloat3(pAt);
    DirectX::XMVECTOR up = DirectX::XMLoadFloat3(pUp);
    DirectX::XMStoreFloat4x4(pOut, DirectX::XMMatrixLookAtLH(eye, at, up));
}

inline void mdVec3Normalize(mdVector3* pOut, const mdVector3* pV) {
    DirectX::XMVECTOR v = DirectX::XMLoadFloat3(pV);
    DirectX::XMStoreFloat3(pOut, DirectX::XMVector3Normalize(v));
}

inline void mdVec3Cross(mdVector3* pOut, const mdVector3* pV1, const mdVector3* pV2) {
    DirectX::XMVECTOR v1 = DirectX::XMLoadFloat3(pV1);
    DirectX::XMVECTOR v2 = DirectX::XMLoadFloat3(pV2);
    DirectX::XMStoreFloat3(pOut, DirectX::XMVector3Cross(v1, v2));
}

inline float mdVec3Dot(const mdVector3* pV1, const mdVector3* pV2) {
    DirectX::XMVECTOR v1 = DirectX::XMLoadFloat3(pV1);
    DirectX::XMVECTOR v2 = DirectX::XMLoadFloat3(pV2);
    return DirectX::XMVectorGetX(DirectX::XMVector3Dot(v1, v2));
}

#define mdSetVector(pCT, device, handle, pVector) (pCT)->SetVector((device), (handle), (const D3DXVECTOR4*)(pVector))
#define mdSetMatrix(pCT, device, handle, pMatrix) (pCT)->SetMatrix((device), (handle), (const D3DXMATRIX*)(pMatrix))

