# D3DX9 Removal Plan

**Objective**: Completely eradicate the legacy DirectX 2010 SDK (`D3DX9`) dependency from the MilkDrop 3 codebase to modernize the build process, remove the requirement for an ancient SDK installer, and prepare the codebase for future modernizations.

This refactor is massive and will touch many core components. To avoid breaking the entire application at once, the work is divided into four distinct phases. We will tackle and test each phase sequentially.

---

## Phase 1: Math Library Migration (DirectXMath)
MilkDrop currently uses D3DX for all matrix and vector operations (`D3DXMATRIX`, `D3DXVECTOR3`, `D3DXMatrixMultiply`, etc.).

**Implementation Plan:**
1. Include the modern Windows SDK header: `#include <DirectXMath.h>`.
2. Replace static data structures:
   - `D3DXVECTOR2`, `D3DXVECTOR3`, `D3DXVECTOR4` -> `DirectX::XMFLOAT2`, `XMFLOAT3`, `XMFLOAT4`.
   - `D3DXMATRIX` -> `DirectX::XMFLOAT4X4` (for storage).
3. Replace mathematical operations:
   - Whenever math needs to be performed, load variables into SIMD registers using `XMLoadFloat4` / `XMLoadFloat4x4` into `XMVECTOR` / `XMMATRIX`.
   - Replace function calls (e.g., `D3DXMatrixRotationX` -> `XMMatrixRotationX`, `D3DXMatrixMultiply` -> `XMMatrixMultiply`).
   - Store results back to memory using `XMStoreFloat4` / `XMStoreFloat4x4`.
4. Compile and verify visual math (rotations, scaling, translations) still behaves exactly identically.

---

## Phase 2: Texture Loading (stb_image)
MilkDrop uses `D3DXCreateTextureFromFileEx` to load user `.jpg`, `.png`, and `.bmp` files for background presets.

**Implementation Plan:**
1. Integrate the single-file public domain library `stb_image.h` into the codebase (e.g., `src/third_party/stb/stb_image.h`).
2. Write a custom helper function `CreateTextureFromFile(IDirect3DDevice9*, const wchar_t*, IDirect3DTexture9**)`:
   - Use `stbi_load` to decode the image file into a raw CPU pixel buffer (RGBA).
   - Use `IDirect3DDevice9::CreateTexture` to create an empty `D3DFMT_A8R8G8B8` texture.
   - Use `IDirect3DTexture9::LockRect` to map the texture memory, copy the decoded CPU buffer into it, and `UnlockRect`.
3. Replace all instances of `D3DXCreateTextureFromFileExW`.
4. Test by loading presets that utilize external textures.

---

## Phase 3: Shader Compilation (d3dcompiler.h)
The application compiles `.fx` HLSL files dynamically at runtime using `D3DXCompileShader` and passes variables to them using `LPD3DXCONSTANTTABLE`.

**Implementation Plan:**
1. Include the modern Windows SDK header: `#include <d3dcompiler.h>`.
2. Link against `d3dcompiler.lib`.
3. Replace `D3DXCompileShader` with `D3DCompile` or `D3DCompileFromFile`.
4. Replace `ID3DXConstantTable`:
   - *Option A*: Try to use `D3DGetConstantTable` (from older Windows SDKs if still available alongside `d3dcompiler`).
   - *Option B*: Manually parse the HLSL constant registers or manually assign explicit `register(c0)` bindings in the HLSL code, and update them using standard `IDirect3DDevice9::SetVertexShaderConstantF` and `SetPixelShaderConstantF`.
5. Test by letting MilkDrop dynamically compile and cycle through multiple presets, observing if shader variables (bass, treble, time) are correctly applied.

---

## Phase 4: Font Rendering (COMPLETE)
**Goal:** Remove `ID3DXFont` usage.
*   **Action:** Implement a custom font renderer using GDI to draw text to a DIB (Device Independent Bitmap), then upload it to an `IDirect3DTexture9` and render it using a textured quad.
*   **Result:** Replaced `LPD3DXFONT` inside `CTextManager` and `pluginshell.cpp` with `mdFont`, a drop-in replacement.
*   **Impact:** Eliminates the final major `D3DX` component, completely removing `d3dx9.lib` from the build requirements.

---

## Final Step
Once all 4 phases are complete, we will completely purge `<d3dx9.h>` from the codebase, remove the legacy SDK dependency checks from `CMakeLists.txt`, and celebrate.
