cmake_minimum_required(VERSION 3.10)
project(MilkDrop3)

# Requires C++17 or later depending on usage, but 11 is safe minimum
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set common definitions from the old vcxproj
add_definitions(
    -DWIN32
    -D_WINDOWS
    -DUSE_VIS_HDR_HWND
    -DSTRSAFE_NO_DEPRECATE
    -DNSEEL_REENTRANT_EXECUTION
    -D_CRT_SECURE_NO_WARNINGS
    -D_CRT_NON_CONFORMING_SWPRINTFS
)

# Set source files
set(SOURCES
    src/audio/audiobuf.cpp
    src/audio/guid.cpp
    src/audio/log.cpp
    src/audio/loopback-capture.cpp
    src/audio/prefs.cpp
    src/ns-eel2/nseel-caltab.c
    src/ns-eel2/nseel-cfunc.c
    src/ns-eel2/nseel-compiler.c
    src/ns-eel2/nseel-eval.c
    src/ns-eel2/nseel-lextab.c
    src/ns-eel2/nseel-ram.c
    src/ns-eel2/nseel-yylex.c
    src/vis_milk2/dxcontext.cpp
    src/vis_milk2/fft.cpp
    src/vis_milk2/menu.cpp
    src/vis_milk2/Milkdrop2PcmVisualizer.cpp
    src/vis_milk2/milkdropfs.cpp
    src/vis_milk2/md_image.cpp
    src/vis_milk2/md_shader.cpp
    src/vis_milk2/md_font.cpp
    src/vis_milk2/plugin.cpp
    src/vis_milk2/pluginshell.cpp
    src/vis_milk2/state.cpp
    src/vis_milk2/support.cpp
    src/vis_milk2/texmgr.cpp
    src/vis_milk2/textmgr.cpp
    src/vis_milk2/utility.cpp
    src/vis_milk2/wasabi.cpp
)

# Handle MSVC-specific requirements (legacy scoping, 32-bit architecture)
if(MSVC)
    add_compile_options(/Zc:forScope-)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        message(FATAL_ERROR "MilkDrop 3 uses 32-bit inline assembly (__asm) and cannot be compiled for 64-bit. Please delete your build folder and run: cmake -A Win32 ..")
    endif()
endif()

# We want an executable
add_executable(MilkDrop3 WIN32 ${SOURCES})

# Include directories
target_include_directories(MilkDrop3 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/audio
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ns-eel2
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vis_milk2
)

# Handle DirectX 9 and Windows libraries
if(WIN32)
    # Search for DirectX SDK from environment variable DXSDK_DIR
    if(DEFINED ENV{DXSDK_DIR})
        file(TO_CMAKE_PATH "$ENV{DXSDK_DIR}" DXSDK_DIR_CMAKE)
        target_include_directories(MilkDrop3 PRIVATE ${DXSDK_DIR_CMAKE}/Include)
        
        # Check architecture to link correct DirectX library path
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            target_link_directories(MilkDrop3 PRIVATE ${DXSDK_DIR_CMAKE}/Lib/x64)
        else()
            target_link_directories(MilkDrop3 PRIVATE ${DXSDK_DIR_CMAKE}/Lib/x86)
        endif()
    else()
        message(WARNING "DXSDK_DIR environment variable not found. Please install the DirectX SDK (June 2010) or ensure the environment variable is set.")
    endif()

    target_link_libraries(MilkDrop3 PRIVATE
        d3d9
        d3dcompiler
        Shlwapi
        user32
    )
endif()

# Copy the clean resources and presets to the build output directory
add_custom_command(TARGET MilkDrop3 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/resources"
        "$<TARGET_FILE_DIR:MilkDrop3>/plugins/Milkdrop3"
    COMMENT "Copying MilkDrop3 resources and presets to the build directory..."
)
