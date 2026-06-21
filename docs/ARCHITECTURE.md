# MilkDrop 3 Codebase Discoveries

This document chronicles the discoveries made while migrating the **MilkDrop 3** project from an antiquated Visual Studio solution to a modern, robust CMake build system, and the intricacies uncovered while reverse-engineering the source code.

## 1. Project Overview & Architecture
MilkDrop 3 is a highly complex hardware-accelerated music visualizer built natively for Windows. It utilizes:
* **Win32 API** for window creation, message pumping, and operating system interfacing.
* **DirectX 9 (Direct3D 9)** for high-performance rendering, texture management, and shader compilation.
* **NS-EEL2 (Nullsoft Expression Evaluator Library)**: The core engine that gives MilkDrop its flexibility. NS-EEL2 takes human-readable math equations (used to generate preset visual effects) and JIT-compiles them directly into native machine code at runtime for maximum performance.

## 2. The Architecture Constraint: 32-bit (x86) Only
We discovered that the codebase **cannot** be compiled as a 64-bit (x64) executable under the Microsoft Visual C++ (MSVC) compiler.
* The `ns-eel2` engine heavily utilizes inline `__asm` blocks to dynamically generate x86 machine code.
* The modern MSVC 64-bit compiler strictly forbids the use of inline assembly.
* **Resolution**: The project must explicitly target the 32-bit architecture (`Win32` or `x86`). When using CMake, this is enforced by running `cmake -A Win32 ..`.

## 3. The DirectX SDK Dependency
Despite DirectX 9 being over a decade old, modern Windows 10/11 SDKs only include the base `d3d9.h` headers. 
* MilkDrop relies on the deprecated **D3DX** utility library (specifically `d3dx9.h` and `d3dx9math.h`) for texture loading, matrix math, and shader processing.
* **Resolution**: Building the project strictly requires the installation of the legacy **DirectX SDK (June 2010)**. Our CMake script now actively searches for the `DXSDK_DIR` environment variable to link against these legacy libraries.

## 4. Legacy C/C++ Compilation Quirks
The source code was written prior to modern C++ standards, relying on behaviors that are now heavily penalized or outright rejected by modern compilers. We encountered and bypassed several massive hurdles:

### The "For Loop Scope" Bug
* **Issue**: In pre-2005 Visual Studio, a variable declared inside a `for` loop (`for (int i = 0; ...)`) remained in scope even after the loop terminated. Modern C++ standard strictly confines the variable to the loop's block.
* **Resolution**: Compiling normally resulted in hundreds of `error C2065: 'i': undeclared identifier` errors. We added the `/Zc:forScope-` compile option in CMake to force MSVC to use the legacy, non-conformant scoping behavior.

### The MSVC Intrinsic Collision (C2169, C7552)
* **Issue**: The original authors wrote their own `__floor` function wrapper in `nseel-compiler.c`. However, modern MSVC has claimed `__floor` as a reserved compiler intrinsic (a built-in math operation).
* **Resolution**: We renamed the internal function from `__floor` to `nseel_floor` to prevent the compiler namespace collision.

### The C2099 "Initializer is not a constant" Error
* **Issue**: The `ns-eel2` C-compiler parses equations and maps them to standard math functions (`sin`, `cos`, `tan`, `pow`, etc.) using a static lookup table (`fnTable1`). In C mode, MSVC forbids initializing a static array with the memory address of dynamic/imported intrinsic math functions (e.g., `&sin`), throwing `error C2099`.
* **Resolution**: We wrote static wrapper functions for all required math intrinsics (e.g., `static double nseel_sin(double a) { return sin(a); }`) and passed the addresses of these wrappers (`&nseel_sin`) to the table instead. This perfectly satisfies the MSVC C-compiler's strict static memory requirements without introducing C++ implicit casting errors.

## 5. Repository Modernization
The repository was originally cluttered with non-functional and deprecated files. We cleaned it up to adhere to modern standards:
* **Removed Visual Studio Bloat**: Deleted all legacy `.sln`, `.vcxproj`, `.vcproj`, and `.dsp` files.
* **Removed Linux Bloat**: Deleted the non-functional `linux/` directory, which merely contained a script instructing users to run the app via Wine.
* **Restructured**: Renamed the raw `code/` directory to `src/` and relocated raw images to a `docs/` folder, updating the `README.md` to reflect these paths.
* **Implemented CMake**: Created a centralized, standard `CMakeLists.txt` at the root of the project to cleanly manage source files, dependencies, and compiler flags.

---
*The project is now stable, clean, and fully compilable on modern Windows machines.*
